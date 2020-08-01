#include "arrange_tree.h"

#include <X11/keysym.h> 
#include <string.h> // for memset


#define ARRANGE_TREE_VERTICAL_MODE      (1 << 0)
#define ARRANGE_TREE_FULLSCREEN_MODE    (1 << 1)
#define ARRANGE_TREE_NESTED_MODE        (1 << 2)

#define ARRANGE_TREE_IS_MODE(x)       (arrange_tree_state.mode & x)

#define ARRANGE_TREE_SET_MODE(x)      (arrange_tree_state.mode |= x)
#define ARRANGE_TREE_SET_NO_MODE(x)   (arrange_tree_state.mode &= (~x))

#define ARRANGE_TREE_IS_WORKSPACE_MODE(x)        ((workspace->arrange_context) && \
                                                 (((ArrangeTreeWorkspaceState*) workspace->arrange_context)->mode & x))

#define ARRANGE_TREE_SET_WORKSPACE_MODE(x)       (((ArrangeTreeWorkspaceState*) workspace->arrange_context)->mode |= x)
#define ARRANGE_TREE_SET_WORKSPACE_NO_MODE(x)    (((ArrangeTreeWorkspaceState*) workspace->arrange_context)->mode &= (~x))

typedef struct{
    Tree node;
    WMWindow* window;
    int x;
    int y;
    unsigned int width;
    unsigned int height;
    float precentage; 
    int mode;
}TreeWindowNode;

typedef struct{
    unsigned int mode;

}ArrangeTreeState;

typedef struct{
    unsigned int mode;
    int prev_x;
    int prev_y;
    unsigned int prev_width;
    unsigned int prev_height;

    TreeWindowNode* root; // the tree model of the current workspace.

}ArrangeTreeWorkspaceState;

ArrangeTreeState arrange_tree_state = {
    .mode = ARRANGE_TREE_VERTICAL_MODE 
};

// ---------------------------------------------------------------------------------
// Helper functions
// ---------------------------------------------------------------------------------

TreeWindowNode* create_tree_node(WMWindow* window){
    TreeWindowNode* new = malloc(sizeof(TreeWindowNode));
    ASSERT(new, "failed to malloc tree node");
    memset(new, 0, sizeof(TreeWindowNode));

    tree_init_tree(&new->node);
    new->window = window;

    new->precentage = 1.0;

    new->mode = ARRANGE_TREE_IS_MODE(ARRANGE_TREE_VERTICAL_MODE) ? 
                ARRANGE_TREE_VERTICAL_MODE :
                !ARRANGE_TREE_VERTICAL_MODE;
    return new;

fail:
    return NULL;
}

ArrangeTreeWorkspaceState* create_workspace_context(){
    ArrangeTreeWorkspaceState* state = malloc(sizeof(ArrangeTreeWorkspaceState));
    ASSERT(state, "failed to allocat arrange_context. \n");
    memset(state, 0, sizeof(ArrangeTreeWorkspaceState));

    // creating a "frame" root node.
    state->root = create_tree_node(NULL);
    ASSERT(state->root, "failed to create root node.\n");



    return state;
fail:
    return NULL;
}

int tree_node_set_precentage(TreeWindowNode* node, 
                                float precentage){
    node->precentage = precentage;
    return 0;
}

int tree_node_set_dimentions(TreeWindowNode* node, 
                                int x,
                                int y,
                                unsigned int width,
                                unsigned int height){
    node->x = x;
    node->y = y;
    node->width = width;
    node->height = height;

    return 0;
}

int tree_node_update(TreeWindowNode* node){
    // This node is a leaf
    if (node->node.children == NULL){
        // nothing to update (when workspace is empty)
        if (node->window == NULL){ 
            return 0;
        }

        node->window->x = node->x + GAP;
        node->window->y = node->y + GAP;
        node->window->width = node->width - (GAP * 2);
        node->window->height = node->height - (GAP * 2);

        LOG("x: %d, y: %d, width: %d, height: %d\n", 
                node->window->x,
                node->window->y,
                node->window->width,
                node->window->height);

        return 0;
    }

    ASSERT((node->window == NULL), "should be a frame node.\n");

    int last_x_position = node->x;
    int last_y_position = node->y;

    Tree* head = node->node.children;
    Tree* next = head;
    do{
        TreeWindowNode* current = (TreeWindowNode*) next;

        // decide how to split
        if (node->mode & ARRANGE_TREE_VERTICAL_MODE){
            // what about the resize scenario?
            int height = node->height * current->precentage;

            tree_node_set_dimentions(   current,
                                        node->x,
                                        last_y_position,
                                        node->width,
                                        height);
            tree_node_update(current);

            last_y_position += height;
        }else{
            // what about the resize scenario?
            int width = node->width * current->precentage;

            tree_node_set_dimentions(   current,
                                        last_x_position,
                                        node->y,
                                        width,
                                        node->height);
            tree_node_update(current);

            last_x_position += width;
        }

        next = next->next;
    }while(next != head);

    return 0;

fail:
    return -1;
}

TreeWindowNode* find_tree_node_by_window(TreeWindowNode* root, WMWindow* window){
    // found!
    if (root->window == window){
        return root;
    }

    if (root->node.children == NULL){
        return NULL;
    }

    Tree* head = root->node.children;
    Tree* next = head;

    TreeWindowNode* found = NULL;
    do{
        found = find_tree_node_by_window(   (TreeWindowNode*)next, 
                                            window);
        if (found){
            return found;
        }

        next = next->next;
    }while(next != head);

    return NULL;
}

int split_tree(TreeWindowNode* parent, 
               TreeWindowNode* new_node){
    // assume no children exists.

    WMWindow* focused_window = parent->window;

    parent->window = NULL; // making the node a frame only node.

    TreeWindowNode* focused_node = create_tree_node(focused_window);
    ASSERT(focused_window, "was unable to create a tree node.\n");

    ASSERT_TO(  fail_on_tree_add, 
                (tree_add(&parent->node, &new_node->node) == 0), 
                "failed to add children to parent.\n");
    ASSERT_TO(  fail_on_tree_add, 
                (tree_add(&parent->node, &focused_node->node) == 0), 
                "failed to add children to parent.\n");

    tree_node_set_precentage(focused_node, 0.5);
    tree_node_set_precentage(new_node, 0.5);

    tree_node_update(parent);
    return 0;

fail_on_tree_add:
    free(focused_node);
fail:
    return -1;
}

int add_children(TreeWindowNode* parent, 
                 TreeWindowNode* new_node){
    ASSERT( (tree_add(&parent->node, &new_node->node) == 0), 
            "failed to add new node to the parent.\n");

    // TODO: need to take into account resizes.
    int num_of_children = tree_num_of_direct_children(&parent->node);

    float precentage = 1.0 / ((float)num_of_children);

    Tree* head = parent->node.children;
    Tree* next = head;
    do{
        TreeWindowNode* current = (TreeWindowNode*)next;

        tree_node_set_precentage(current, precentage);

        next = next->next;
    }while(next != head);

    return tree_node_update(parent);

fail:
    return -1;
}

int del_children(TreeWindowNode* node){
    TreeWindowNode* parent = (TreeWindowNode*)node->node.parent;

    ASSERT( (tree_del(&node->node) == 0), 
            "failed to delete node from tree.\n");

    // TODO: need to take into account resizes.
    int num_of_children = tree_num_of_direct_children(&parent->node);

    // nothing to do.
    if (num_of_children == 0){
        return 0;
    }

    float precentage = 1.0 / ((float)num_of_children);

    Tree* head = parent->node.children;
    Tree* next = head;
    do{
        TreeWindowNode* current = (TreeWindowNode*)next;

        LOG("setting precentage: %f\n", precentage);
        tree_node_set_precentage(current, precentage);

        next = next->next;
    }while(next != head);

    return tree_node_update(parent);

fail:
    return -1;
}

// ---------------------------------------------------------------------------------
// Main functions
// ---------------------------------------------------------------------------------

int tree_on_new_window(  WMWorkspace* workspace, 
                         WMWindow* window){
    // if context isn't exist create one.
    if (workspace->arrange_context == NULL){
        workspace->arrange_context = create_workspace_context();
        ASSERT(workspace->arrange_context, "failed to allocate arrange_context.\n");

        tree_node_set_dimentions(   ((ArrangeTreeWorkspaceState*)workspace->arrange_context)->root,
                                    0,
                                    0,
                                    workspace->width,
                                    workspace->height);
    }

    // creating the node corresponding te the new window.
    TreeWindowNode* new_node = create_tree_node(window);

    WMWindow* focused_window = workspace->focused_window;

    // This is the first window
    if (focused_window == NULL){ 
        // adding the first node directly to the root.
        ASSERT_TO(  fail_on_find_tree_node, 
                    (add_children(((ArrangeTreeWorkspaceState*)workspace->arrange_context)->root, 
                                  new_node) == 0), 
                    "failed to add children.\n");
        return 0;
    }

    // NOTE: the find_tree_node_by_window function should always return a leaf tree node.
    TreeWindowNode* parent = find_tree_node_by_window(  ((ArrangeTreeWorkspaceState*)workspace->arrange_context)->root, 
                                                        workspace->focused_window);
    ASSERT_TO(fail_on_find_tree_node, parent, "Wasn't able to find tree node for the focused window.\n");

    // now the question is:
    // - do we want to split the current window and add the new to the split?
    // - or do we want to add the new window to the parent tree?
    // the answer will be given to us by the global state's mode in the form of 
    // ARRANGE_TREE_NESTED_MODE
    
    if (ARRANGE_TREE_IS_MODE(ARRANGE_TREE_NESTED_MODE)){ // splitting the current window.
        ASSERT_TO(  fail_on_find_tree_node, 
                    (split_tree(parent, new_node) == 0),
                    "failed to split tree.\n");

        // we want to only nest one time after the user chooses vertical/horisontal splits.
        ARRANGE_TREE_SET_NO_MODE(ARRANGE_TREE_NESTED_MODE);
    }else{  // adding the window to the parent layout.
        parent = (TreeWindowNode*)parent->node.parent;
        ASSERT_TO(  fail_on_find_tree_node, 
                    (add_children(parent, new_node) == 0), 
                    "failed to add children.\n");
    }

    return 0;

fail_on_find_tree_node:
    free(new_node);
fail:
    return -1;
}

int tree_on_del_window(  WMWorkspace* workspace, 
                         WMWindow* window){
    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "focused_window is not when deleting??\n");

    TreeWindowNode* node = find_tree_node_by_window(( (ArrangeTreeWorkspaceState*)workspace->arrange_context)->root, 
                                                        workspace->focused_window);
    ASSERT(node, "Wasn't able to find tree node for the focused window.\n");

    // we need to trim the tree fully, without a tree that has no children
    TreeWindowNode* parent = (TreeWindowNode*)node->node.parent;
    do{
        ASSERT((del_children(node) == 0), "wasn't able to delete node from tree.\n");
        if (parent->node.children != NULL){
            break;
        }
        node = parent;
        parent = (TreeWindowNode*)node->node.parent;
    }while (node != ((ArrangeTreeWorkspaceState*)workspace->arrange_context)->root);
    return 0;

fail:
    return -1;
}

// ---------------------------------------------------------------------------------
// Key press callbacks
// ---------------------------------------------------------------------------------

int tree_on_horizontal(WMWorkspace* workspace){
    ARRANGE_TREE_SET_NO_MODE(ARRANGE_TREE_VERTICAL_MODE);

    TreeWindowNode* parent = find_tree_node_by_window(( (ArrangeTreeWorkspaceState*)workspace->arrange_context)->root, 
                                                        workspace->focused_window);
    ASSERT(parent, "Wasn't able to find tree node for the focused window.\n");

    ARRANGE_TREE_SET_MODE(ARRANGE_TREE_NESTED_MODE);

    parent->mode &= ~ARRANGE_TREE_VERTICAL_MODE;

    // if (ARRANGE_TREE_IS_MODE(ARRANGE_TREE_NESTED_MODE)){
        // parent->mode &= ~ARRANGE_TREE_VERTICAL_MODE;
    // }else{
        // parent = (TreeWindowNode*) parent->node.parent;
        // parent->mode &= ~ARRANGE_TREE_VERTICAL_MODE;
    // }

    return 0;

fail:
    return -1;
}

int tree_on_vertical(WMWorkspace* workspace){
    ARRANGE_TREE_SET_MODE(ARRANGE_TREE_VERTICAL_MODE);

    TreeWindowNode* parent = find_tree_node_by_window(( (ArrangeTreeWorkspaceState*)workspace->arrange_context)->root, 
                                                        workspace->focused_window);
    ASSERT(parent, "Wasn't able to find tree node for the focused window.\n");

    ARRANGE_TREE_SET_MODE(ARRANGE_TREE_NESTED_MODE);

    parent->mode |= ARRANGE_TREE_VERTICAL_MODE;
    // if (ARRANGE_TREE_IS_MODE(ARRANGE_TREE_NESTED_MODE)){
        // parent->mode |= ARRANGE_TREE_VERTICAL_MODE;
    // }else{
        // parent = (TreeWindowNode*) parent->node.parent;
        // parent->mode |= ARRANGE_TREE_VERTICAL_MODE;
    // }

    return 0;

fail:
    return -1;
}

int tree_on_fullscreen_toggle(WMWorkspace* workspace){
    return 0;
}

int tree_on_move_left(WMWorkspace* workspace){
    return 0;
}
int tree_on_move_down(WMWorkspace* workspace){
    return 0;
}
int tree_on_move_up(WMWorkspace* workspace){
    return 0;
}
int tree_on_move_right(WMWorkspace* workspace){
    return 0;
}

int tree_on_focus_left(WMWorkspace* workspace){
    ASSERT(workspace->focused_window, "no focused window.\n");

    WMWindow* window = workspace_get_left_window(workspace);
    ASSERT(window, "no window found to focus.\n");

    workspace->focused_window = window; 

    return 0;
fail:
    return -1;
}
int tree_on_focus_down(WMWorkspace* workspace){
    ASSERT(workspace->focused_window, "no focused window.\n");

    WMWindow* window = workspace_get_down_window(workspace);
    ASSERT(window, "no window found to focus.\n");

    workspace->focused_window = window; 

    return 0;
fail:
    return -1;
}
int tree_on_focus_up(WMWorkspace* workspace){
    ASSERT(workspace->focused_window, "no focused window.\n");

    WMWindow* window = workspace_get_up_window(workspace);
    ASSERT(window, "no window found to focus.\n");

    workspace->focused_window = window; 

    return 0;
fail:
    return -1;
}
int tree_on_focus_right(WMWorkspace* workspace){
    ASSERT(workspace->focused_window, "no focused window.\n");

    WMWindow* window = workspace_get_right_window(workspace);
    ASSERT(window, "no window found to focus.\n");

    workspace->focused_window = window; 

    return 0;
fail:
    return -1;
}

int tree_on_resize_left(WMWorkspace* workspace){
    ASSERT(workspace->focused_window, "there is no window to resize\n");

    TreeWindowNode* node = find_tree_node_by_window(    ((ArrangeTreeWorkspaceState*)workspace->arrange_context)->root, 
                                                        workspace->focused_window);
    ASSERT(node, "Wasn't able to find tree node for the focused window.\n");

    // when trying to resize left and right and the current tree is not horisontal
    // than we trying to expand beyond the tree boundries. 
    // keep going up to find the closest parent that has horisontal mode
    TreeWindowNode* parent = (TreeWindowNode*)node->node.parent;
    do{
        if (!(parent->mode & ARRANGE_TREE_VERTICAL_MODE)){
            break;
        }

        node = parent;
        parent = (TreeWindowNode*)parent->node.parent;
    }while(parent != NULL);
    ASSERT(parent, "Wasn't able to find closest parent with the required mode.\n");

    // node is an only child. (point to itself)
    if (node->node.next == (Tree*)node){
        return 0; // nothing to do? BUG?
    }

    // rising the precentage to reflect the resize
    int num_of_children = tree_num_of_direct_children(&parent->node);

    float target_precentage = 0.05;
    float surround_precentage = target_precentage / ((float) (num_of_children - 1)) * (-1);

    Tree* head = parent->node.children;
    Tree* next = head;
    do{
        TreeWindowNode* current = (TreeWindowNode*)next;

        if (current == node){
            LOG("updating precentage: %f\n", current->precentage + target_precentage);
            tree_node_set_precentage(current, current->precentage + target_precentage);
        }else{
            LOG("updating precentage: %f\n", current->precentage + surround_precentage);
            tree_node_set_precentage(current, current->precentage + surround_precentage);
        }

        next = next->next;
    }while(next != head);

    return tree_node_update(parent);
fail:
    return -1;
}
int tree_on_resize_down(WMWorkspace* workspace){
    ASSERT(workspace->focused_window, "there is no window to resize\n");

    TreeWindowNode* node = find_tree_node_by_window(    ((ArrangeTreeWorkspaceState*)workspace->arrange_context)->root, 
                                                        workspace->focused_window);
    ASSERT(node, "Wasn't able to find tree node for the focused window.\n");

    // when trying to resize left and right and the current tree is not horisontal
    // than we trying to expand beyond the tree boundries. 
    // keep going up to find the closest parent that has horisontal mode
    TreeWindowNode* parent = (TreeWindowNode*)node->node.parent;
    do{
        if ((parent->mode & ARRANGE_TREE_VERTICAL_MODE)){
            break;
        }

        node = parent;
        parent = (TreeWindowNode*)parent->node.parent;
    }while(parent != NULL);
    ASSERT(parent, "Wasn't able to find closest parent with the required mode.\n");

    // node is an only child. (point to itself)
    if (node->node.next == (Tree*)node){
        return 0; // nothing to do? BUG?
    }

    // rising the precentage to reflect the resize
    int num_of_children = tree_num_of_direct_children(&parent->node);

    float target_precentage = 0.05;
    float surround_precentage = target_precentage / ((float) (num_of_children - 1)) * (-1);

    Tree* head = parent->node.children;
    Tree* next = head;
    do{
        TreeWindowNode* current = (TreeWindowNode*)next;

        if (current == node){
            LOG("updating precentage: %f\n", current->precentage + target_precentage);
            tree_node_set_precentage(current, current->precentage + target_precentage);
        }else{
            LOG("updating precentage: %f\n", current->precentage + surround_precentage);
            tree_node_set_precentage(current, current->precentage + surround_precentage);
        }

        next = next->next;
    }while(next != head);

    return tree_node_update(parent);
fail:
    return -1;
}
int tree_on_resize_up(WMWorkspace* workspace){
    ASSERT(workspace->focused_window, "there is no window to resize\n");

    TreeWindowNode* node = find_tree_node_by_window(    ((ArrangeTreeWorkspaceState*)workspace->arrange_context)->root, 
                                                        workspace->focused_window);
    ASSERT(node, "Wasn't able to find tree node for the focused window.\n");

    // when trying to resize left and right and the current tree is not horisontal
    // than we trying to expand beyond the tree boundries. 
    // keep going up to find the closest parent that has horisontal mode
    TreeWindowNode* parent = (TreeWindowNode*)node->node.parent;
    do{
        if ((parent->mode & ARRANGE_TREE_VERTICAL_MODE)){
            break;
        }

        node = parent;
        parent = (TreeWindowNode*)parent->node.parent;
    }while(parent != NULL);
    ASSERT(parent, "Wasn't able to find closest parent with the required mode.\n");

    // node is an only child. (point to itself)
    if (node->node.next == (Tree*)node){
        return 0; // nothing to do? BUG?
    }

    // rising the precentage to reflect the resize
    int num_of_children = tree_num_of_direct_children(&parent->node);

    float target_precentage = -0.05;
    float surround_precentage = target_precentage / ((float) (num_of_children - 1)) * (-1);

    Tree* head = parent->node.children;
    Tree* next = head;
    do{
        TreeWindowNode* current = (TreeWindowNode*)next;

        if (current == node){
            LOG("updating precentage: %f\n", current->precentage + target_precentage);
            tree_node_set_precentage(current, current->precentage + target_precentage);
        }else{
            LOG("updating precentage: %f\n", current->precentage + surround_precentage);
            tree_node_set_precentage(current, current->precentage + surround_precentage);
        }

        next = next->next;
    }while(next != head);

    return tree_node_update(parent);
fail:
    return -1;
}
int tree_on_resize_right(WMWorkspace* workspace){
    ASSERT(workspace->focused_window, "there is no window to resize\n");

    TreeWindowNode* node = find_tree_node_by_window(    ((ArrangeTreeWorkspaceState*)workspace->arrange_context)->root, 
                                                        workspace->focused_window);
    ASSERT(node, "Wasn't able to find tree node for the focused window.\n");

    // when trying to resize left and right and the current tree is not horisontal
    // than we trying to expand beyond the tree boundries. 
    // keep going up to find the closest parent that has horisontal mode
    TreeWindowNode* parent = (TreeWindowNode*)node->node.parent;
    do{
        if (!(parent->mode & ARRANGE_TREE_VERTICAL_MODE)){
            break;
        }

        node = parent;
        parent = (TreeWindowNode*)parent->node.parent;
    }while(parent != NULL);
    ASSERT(parent, "Wasn't able to find closest parent with the required mode.\n");

    // node is an only child. (point to itself)
    if (node->node.next == (Tree*)node){
        return 0; // nothing to do? BUG?
    }

    // rising the precentage to reflect the resize
    int num_of_children = tree_num_of_direct_children(&parent->node);

    float target_precentage = -0.05;
    float surround_precentage = target_precentage / ((float) (num_of_children - 1)) * (-1);

    Tree* head = parent->node.children;
    Tree* next = head;
    do{
        TreeWindowNode* current = (TreeWindowNode*)next;

        if (current == node){
            LOG("updating precentage: %f\n", current->precentage + target_precentage);
            tree_node_set_precentage(current, current->precentage + target_precentage);
        }else{
            LOG("updating precentage: %f\n", current->precentage + surround_precentage);
            tree_node_set_precentage(current, current->precentage + surround_precentage);
        }

        next = next->next;
    }while(next != head);

    return tree_node_update(parent);
fail:
    return -1;
}

int (*tree_event_handlers[30]) (WMWorkspace *) = {
    [0] = tree_on_vertical,
    [1] = tree_on_horizontal,

    [2] = tree_on_fullscreen_toggle,

    [3] = tree_on_move_left,
    [4] = tree_on_move_down,
    [5] = tree_on_move_up,
    [6] = tree_on_move_right,

    [7] = tree_on_focus_left,
    [8] = tree_on_focus_down,
    [9] = tree_on_focus_up,
    [10] = tree_on_focus_right,

    [15] = tree_on_resize_left,
    [16] = tree_on_resize_down,
    [17] = tree_on_resize_up,
    [18] = tree_on_resize_right
};

// one thing to note here.. 
// the keysym can be difer from what the user
// actually pressed. it is just the keysym
// configured in the wm.h configuration of the
// keybindings.
int tree_on_key_press(int keysym, WMWorkspace* workspace){
    if (tree_event_handlers[keysym]){
        return tree_event_handlers[keysym](workspace);
    }
    return 0;
}
