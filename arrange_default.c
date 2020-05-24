#include "arrange_default.h"

#include <X11/keysym.h> 
#include <string.h>


// ---------------------------------------------------------
// helper functions
// ---------------------------------------------------------

int get_num_of_windows(WMWorkspace* workspace){
    int num_of_windows = 0;

    List* curr = &workspace->windows_list;
    while(curr->next){
        curr = curr->next;

        num_of_windows++;
    }

    return num_of_windows;
}

// TODO: if more than one choose smallest.
// TODO: if no one is fitting search two that fits together.
WMWindow* find_mergable(WMWorkspace* workspace, WMWindow* window){
    WMWindow* found = NULL;

    int x = window->x;
    int y = window->y;
    unsigned int width = window->width;
    unsigned int height = window->height;

    List* curr = &workspace->windows_list;
    while(curr->next){
        curr = curr->next;
        WMWindow* curr_window = (WMWindow*) curr;

        // ignore itself
        if (curr_window == window){
            continue;
        }

        // check down direction
        if ((curr_window->x == x) &&
            (curr_window->y == y + height) &&
            (curr_window->x + curr_window->width == x + width)){

            found = curr_window;
            break;
        }
        // check up direction
        if ((curr_window->x == x) &&
            (curr_window->y + curr_window->height == y) &&
            (curr_window->x + curr_window->width == x + width)){

            found = curr_window;
            break;
        }
        // check right direction
        if ((curr_window->y == y) &&
            (curr_window->x == x + width) &&
            (curr_window->y + curr_window->height == y + height)){

            found = curr_window;
            break;
        }
        // check left direction
        if ((curr_window->y == y) &&
            (curr_window->x + curr_window->width == x) &&
            (curr_window->y + curr_window->height == y + height)){

            found = curr_window;
            break;
        }
    }
    return found;
}

// ---------------------------------------------------------

#define VERTICAL_MODE   (1 << 0)
#define FULLSCREEN_MODE (1 << 1)

#define IS_MODE(x)       (state.mode & x)

#define SET_MODE(x)      (state.mode |= x)
#define SET_NO_MODE(x)   (state.mode &= (~x))

#define IS_WORKSPACE_MODE(x)        (((WorkspaceState*) workspace->arrange_context)->mode & x)
#define SET_WORKSPACE_MODE(x)       (((WorkspaceState*) workspace->arrange_context)->mode |= x)
#define SET_WORKSPACE_NO_MODE(x)    (((WorkspaceState*) workspace->arrange_context)->mode &= (~x))

typedef struct{
    unsigned int mode;

}State;

typedef struct{
    unsigned int mode;
    int prev_x;
    int prev_y;
    unsigned int prev_width;
    unsigned int prev_height;

}WorkspaceState;

State state = {
    .mode = VERTICAL_MODE
};

int default_on_new_window(  WMWorkspace* workspace,
                            WMWindow* window){

    // the first window gets all the screen.
    if (get_num_of_windows(workspace) == 0){ 
        window->x = 0 + GAP;
        window->y = 0 + GAP;
        window->width = workspace->width - (GAP * 2);
        window->height = workspace->height - (GAP * 2);

        return 0;
    }

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "focused_widnow is null?\n");

    if(IS_MODE(VERTICAL_MODE)){
        unsigned int new_width = (focused_window->width / 2) - GAP;

        focused_window->width = new_width;

        window->y = focused_window->y;
        window->height = focused_window->height;
        window->x = focused_window->x + focused_window->width + (2 * GAP);
        window->width = new_width;
    }else{
        unsigned int new_height = (focused_window->height / 2) - GAP;

        focused_window->height = new_height;

        window->x = focused_window->x;
        window->width = focused_window->width;
        window->y = focused_window->y + focused_window->height + (2 * GAP);
        window->height = new_height;
    }
    return 0;

fail:
    return -1;
}

int default_on_del_window(  WMWorkspace* workspace,
                            WMWindow* window){
    // last window - nothing to do.
    if (get_num_of_windows(workspace) == 1){
        return 0;
    }

    // // we need to merge the deleted window with existing one.
    // WMWindow* mergable = find_mergable(workspace, window);
    // ASSERT(mergable, "failed to find mergable.\n");

    // // up or down
    // if (mergable->x == window->x){
        // // up
        // if (mergable->y + mergable->height == window->y){
            // mergable->height += window->height;
            // return 0;
        // }
        // // down
        // if (mergable->y == window->y + window->height){
            // mergable->height += window->height;
            // mergable->y = window->y;
            // return 0;
        // }
    // }
    // // left or right
    // if (mergable->y == window->y){
        // // left
        // if (mergable->x + mergable->width == window->x){
            // mergable->width += window->width;
            // return 0;
        // }
        // // right
        // if (mergable->x == window->x + window->width){
            // mergable->width += window->width;
            // mergable->x = window->x;
            // return 0;
        // }
    // }

// fail:
    return 0;
}

// Align the focused window to the right direction.
int default_on_align_left(WMWorkspace* workspace){
    WMWindow* window;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "trying to align without a focused window.\n");

    window = workspace_get_left_window(workspace);
    // end of screen
    if (window == NULL){
        focused_window->width += focused_window->x - (GAP);
        focused_window->x = 0 + GAP;
    }else{
        focused_window->width += focused_window->x - 
                                (window->x + window->width) -
                                (GAP * 2);
        focused_window->x -= focused_window->x - 
                                (window->x + window->width) -
                                (GAP * 2);
    }

    return 0;
fail:
    return -1;
}

int default_on_align_right(WMWorkspace* workspace){
    WMWindow* window;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "trying to align without a focused window.\n");

    window = workspace_get_right_window(workspace);
    if (window == NULL){
        focused_window->width += workspace->width - 
                                (focused_window->x + focused_window->width) - 
                                GAP;
    }else{
        focused_window->width += window->x - 
                                (focused_window->x + focused_window->width) - 
                                (GAP * 2);
    }

    return 0;
fail:
    return -1;
}

int default_on_align_up(WMWorkspace* workspace){
    WMWindow* window;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "trying to align without a focused window.\n");

    window = workspace_get_up_window(workspace);
    if (window == NULL){
        focused_window->height += focused_window->y - GAP;
        focused_window->y = 0 + GAP;
    }else{
        focused_window->height += focused_window->y - 
                                    (window->y + window->height) - 
                                    (GAP * 2);
        focused_window->y -= focused_window->y - 
                                    (window->y + window->height) -
                                    (GAP * 2);
    }

    return 0;
fail:
    return -1;
}

int default_on_align_down(WMWorkspace* workspace){
    WMWindow* window;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "trying to align without a focused window.\n");

    window = workspace_get_down_window(workspace);
    if (window == NULL){
        focused_window->height += workspace->height - 
                                    (focused_window->y + focused_window->height) -
                                    GAP;
    }else{
        focused_window->height += window->y - 
                                    (focused_window->y + focused_window->height) -
                                    (GAP * 2);
    }
fail:
    return -1;
}

// Resize windows!
int default_on_resize_left(WMWorkspace* workspace);
int default_on_resize_right(WMWorkspace* workspace);
int default_on_resize_up(WMWorkspace* workspace);
int default_on_resize_down(WMWorkspace* workspace);

int default_on_resize_left(WMWorkspace* workspace){
    WMWindow* window;
    int ret = 0;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "trying to resize without a focused window.\n");

    window = workspace_get_left_window(workspace);
    if (window == NULL){
        window = workspace_get_right_window(workspace);
        workspace->focused_window = window;

        ret = default_on_resize_left(workspace);

        workspace->focused_window = focused_window;
    }else{

        // is perfect?
        ASSERT((window->y == focused_window->y), "resize: y != y\n");
        ASSERT((window->height == focused_window->height), "resize: height != height\n");

        // is close?
        unsigned int distance_between_windows = focused_window->x -
            (window->x + window->width);
        ASSERT((distance_between_windows < (GAP * 3)), 
                "distance_between_windows < (GAP * 3)\n");

        focused_window->x -= RESIZE_SIZE;
        focused_window->width += RESIZE_SIZE;
        window->width -= RESIZE_SIZE;
    }
    return ret;

fail:
    return -1;
}

int default_on_resize_right(WMWorkspace* workspace){
    WMWindow* window;
    int ret = 0;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "trying to resize without a focused window.\n");

    window = workspace_get_right_window(workspace);
    if (window == NULL){
        window = workspace_get_left_window(workspace);
        workspace->focused_window = window;

        ret = default_on_resize_right(workspace);

        workspace->focused_window = focused_window;
    }else{
        // is perfect?
        ASSERT((window->y == focused_window->y), "resize: y != y\n");
        ASSERT((window->height == focused_window->height), "resize: height != height\n");

        // // is close?
        unsigned int distance_between_windows = window->x -
            (focused_window->x + focused_window->width);
        ASSERT((distance_between_windows < (GAP * 3)), 
                "distance_between_windows < (GAP * 3)\n");

        window->x += RESIZE_SIZE;
        window->width -= RESIZE_SIZE;
        focused_window->width += RESIZE_SIZE;
    }

    return ret;
fail:
    return -1;
}

int default_on_resize_up(WMWorkspace* workspace){
    WMWindow* window;
    int ret = 0;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "trying to resize without a focused window.\n");

    window = workspace_get_up_window(workspace);
    if (window == NULL){
        window = workspace_get_down_window(workspace);
        workspace->focused_window = window;

        ret = default_on_resize_up(workspace);

        workspace->focused_window = focused_window;
    }else{
        // is perfect?
        ASSERT((window->x == focused_window->x), "resize: x != x\n");
        ASSERT((window->width == focused_window->width), "resize: width != width\n");

        // // is close?
        unsigned int distance_between_windows = focused_window->y -
            (window->y + window->height);
        ASSERT((distance_between_windows < (GAP * 3)), 
                "distance_between_windows < (GAP * 3)\n");

        focused_window->y -= RESIZE_SIZE;
        focused_window->height += RESIZE_SIZE;
        window->height -= RESIZE_SIZE;
    }

    return ret;
fail:
    return -1;
}

int default_on_resize_down(WMWorkspace* workspace){
    WMWindow* window;
    int ret = 0;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "trying to resize without a focused window.\n");

    window = workspace_get_down_window(workspace);
    if (window == NULL){
        window = workspace_get_up_window(workspace);
        workspace->focused_window = window;

        ret = default_on_resize_down(workspace);

        workspace->focused_window = focused_window;
    }else{
        // is perfect?
        ASSERT((window->x == focused_window->x), "resize: x != x\n");
        ASSERT((window->width == focused_window->width), "resize: width != width\n");

        // // is close?
        unsigned int distance_between_windows = window->y -
            (focused_window->y + focused_window->height);
        ASSERT((distance_between_windows < (GAP * 3)), 
                "distance_between_windows < (GAP * 3)\n");

        focused_window->height += RESIZE_SIZE;
        window->y += RESIZE_SIZE;
        window->height -= RESIZE_SIZE;
    }

    return ret;
fail:
    return -1;
}

int default_on_vertical_toggle(WMWorkspace* workspace){
    if (IS_MODE(VERTICAL_MODE)){
        SET_NO_MODE(VERTICAL_MODE);
    }else{
        SET_MODE(VERTICAL_MODE);
    }
    return 0;
}

int default_on_vertical(WMWorkspace* workspace){
    SET_MODE(VERTICAL_MODE);
    return 0;
}
int default_on_horizontal(WMWorkspace* workspace){
    SET_NO_MODE(VERTICAL_MODE);
    return 0;
}

int default_on_fullscreen_toggle(WMWorkspace* workspace){
    // create context when first needed.
    if (workspace->arrange_context == NULL){
        workspace->arrange_context = malloc(sizeof(WorkspaceState));
        ASSERT(workspace->arrange_context, "failed to allocate arrange_context.\n");
        memset(workspace->arrange_context, 0, sizeof(WorkspaceState));
    }

    if (IS_WORKSPACE_MODE(FULLSCREEN_MODE)){
        if (workspace->focused_window){
            workspace->focused_window->x = ((WorkspaceState*) workspace->arrange_context)->prev_x;
            workspace->focused_window->y = ((WorkspaceState*) workspace->arrange_context)->prev_y;
            workspace->focused_window->width = ((WorkspaceState*) workspace->arrange_context)->prev_width;
            workspace->focused_window->height = ((WorkspaceState*) workspace->arrange_context)->prev_height;

            // restore to default values.
            ((WorkspaceState*) workspace->arrange_context)->prev_x = -1;
            ((WorkspaceState*) workspace->arrange_context)->prev_y = -1;
            ((WorkspaceState*) workspace->arrange_context)->prev_width = 0;
            ((WorkspaceState*) workspace->arrange_context)->prev_height = 0;
        }
        SET_WORKSPACE_NO_MODE(FULLSCREEN_MODE);
    }else{
        // no window to fullscreen -> do nothing.
        if (workspace->focused_window == NULL){
            return 0;
        }
        // already fullscreen -> do nothing.
        if ((workspace->focused_window->x == 0 + GAP) &&
            (workspace->focused_window->y == 0 + GAP) &&
            (workspace->focused_window->width == workspace->width - (GAP * 2)) &&
            (workspace->focused_window->height == workspace->height - (GAP * 2))){
            return 0;
        }

        // saving prev state
        ((WorkspaceState*) workspace->arrange_context)->prev_x = workspace->focused_window->x;
        ((WorkspaceState*) workspace->arrange_context)->prev_y = workspace->focused_window->y;
        ((WorkspaceState*) workspace->arrange_context)->prev_width = workspace->focused_window->width;
        ((WorkspaceState*) workspace->arrange_context)->prev_height = workspace->focused_window->height;

        workspace->focused_window->x = 0 + GAP;
        workspace->focused_window->y = 0 + GAP;
        workspace->focused_window->width = workspace->width - (GAP * 2);
        workspace->focused_window->height = workspace->height - (GAP * 2);

        SET_WORKSPACE_MODE(FULLSCREEN_MODE);
    }
    return 0;

fail:
    return -1;
}

int (*event_handlers[XK_nobreakspace]) (WMWorkspace *) = {
    [XK_V] = default_on_horizontal,
    [XK_1] = default_on_vertical,

    [XK_F] = default_on_fullscreen_toggle,

    [XK_H] = default_on_align_left,
    [XK_J] = default_on_align_down,
    [XK_K] = default_on_align_up,
    [XK_L] = default_on_align_right,

    [XK_Y] = default_on_resize_left,
    [XK_U] = default_on_resize_down,
    [XK_I] = default_on_resize_up,
    [XK_O] = default_on_resize_right
};

// one thing to note here.. 
// the keysym can be difer from what the user
// actually pressed. it is just the keysym
// configured in the wm.h configuration of the
// keybindings.
int default_on_key_press(int keysym, WMWorkspace* workspace){
    if (event_handlers[keysym]){
        return event_handlers[keysym](workspace);
    }
    return 0;
}

// ---------------------------------------------------------

