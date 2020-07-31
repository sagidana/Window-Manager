#include "arrange_tree.h"

#include <X11/keysym.h> 


#define ARRANGE_TREE_VERTICAL_MODE   (1 << 0)
#define ARRANGE_TREE_FULLSCREEN_MODE (1 << 1)

#define ARRANGE_TREE_IS_MODE(x)       (state.mode & x)

#define ARRANGE_TREE_SET_MODE(x)      (state.mode |= x)
#define ARRANGE_TREE_SET_NO_MODE(x)   (state.mode &= (~x))

#define ARRANGE_TREE_IS_WORKSPACE_MODE(x)        ((workspace->arrange_context) && \
                                                 (((WorkspaceState*) workspace->arrange_context)->mode & x))

#define ARRANGE_TREE_SET_WORKSPACE_MODE(x)       (((WorkspaceState*) workspace->arrange_context)->mode |= x)
#define ARRANGE_TREE_SET_WORKSPACE_NO_MODE(x)    (((WorkspaceState*) workspace->arrange_context)->mode &= (~x))

typedef struct{
    Tree node;
    WMWindow* window;
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

    Tree* root; // the tree model of the current workspace.

}ArrangeTreeWorkspaceState;

ArrangeTreeState arrange_tree_state = {
    .mode = ARRANGE_TREE_VERTICAL_MODE
};

// ---------------------------------------------------------------------------------
// Tree functions
// ---------------------------------------------------------------------------------

TreeWindowNode* create_tree_node(WMWindow* window){
    TreeWindowNode* new = malloc(sizeof(TreeWindowNode));
    ASSERT(new, "failed to malloc tree node");

    tree_init_tree(&new->node);
    new->window = window;
    return new;

fail:
    return NULL;
}

// ---------------------------------------------------------------------------------
// Main functions
// ---------------------------------------------------------------------------------

int tree_on_new_window(  WMWorkspace* workspace, 
                         WMWindow* window){

    if (workspace->arrange_context == NULL){
        workspace->arrange_context = malloc(sizeof(ArrangeTreeWorkspaceState));
        ASSERT(workspace->arrange_context, "failed to allocate arrange_context.\n");
        memset(workspace->arrange_context, 0, sizeof(ArrangeTreeWorkspaceState));
    }

    WMWindow* focused_window = workspace->focused_window;
    // This is the first window
    if (focused_window == NULL){ 

    }else{
    }

    return -1;
}

int tree_on_del_window(  WMWorkspace* workspace, 
                         WMWindow* window){
    return 0;
}

// ---------------------------------------------------------------------------------
// Key press callbacks
// ---------------------------------------------------------------------------------

int tree_on_horizontal(WMWorkspace* workspace){
    return 0;
}
int tree_on_vertical(WMWorkspace* workspace){
    return 0;
}
int tree_on_fullscreen_toggle(WMWorkspace* workspace){
    return 0;
}
int tree_on_align_left(WMWorkspace* workspace){
    return 0;
}
int tree_on_align_down(WMWorkspace* workspace){
    return 0;
}
int tree_on_align_up(WMWorkspace* workspace){
    return 0;
}
int tree_on_align_right(WMWorkspace* workspace){
    return 0;
}
int tree_on_resize_left(WMWorkspace* workspace){
    return 0;
}
int tree_on_resize_down(WMWorkspace* workspace){
    return 0;
}
int tree_on_resize_up(WMWorkspace* workspace){
    return 0;
}
int tree_on_resize_right(WMWorkspace* workspace){
    return 0;
}

int (*tree_event_handlers[XK_nobreakspace]) (WMWorkspace *) = {
    [XK_V] = tree_on_horizontal,
    [XK_1] = tree_on_vertical,

    [XK_F] = tree_on_fullscreen_toggle,

    [XK_H] = tree_on_align_left,
    [XK_J] = tree_on_align_down,
    [XK_K] = tree_on_align_up,
    [XK_L] = tree_on_align_right,

    [XK_Y] = tree_on_resize_left,
    [XK_U] = tree_on_resize_down,
    [XK_I] = tree_on_resize_up,
    [XK_O] = tree_on_resize_right
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
