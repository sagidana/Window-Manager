#include "arrange_default.h"
#include <X11/keysym.h> 

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

#define VERTICAL_MODE (0)
#define HORIZONTAL_MODE (1)

typedef struct{
    int mode;
}State;

State state = {
    .mode = VERTICAL_MODE
};

int default_on_new_window(  WMWorkspace* workspace,
                            WMWindow* window){

    // the first window gets all the screen.
    if (get_num_of_windows(workspace) == 0){ 
        window->x = 0;
        window->y = 0;
        window->width = workspace->width;
        window->height = workspace->height;

        return 0;
    }

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "focused_widnow is null?\n");

    if(state.mode == VERTICAL_MODE){
        focused_window->width /= 2;

        window->x = focused_window->x + focused_window->width + 1;
        window->y = focused_window->y;
        window->width = focused_window->width;
        window->height = focused_window->height;
    }else if (state.mode == HORIZONTAL_MODE){
        focused_window->height /= 2;

        window->x = focused_window->x;
        window->y = focused_window->y + focused_window->height + 1;
        window->width = focused_window->width;
        window->height = focused_window->height;
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
        focused_window->width += focused_window->x;
        focused_window->x = 0;
    }else{
        focused_window->width += focused_window->x - (window->x + window->width);
        focused_window->x -= focused_window->x - (window->x + window->width);
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
        focused_window->width += workspace->width - (focused_window->x + focused_window->width);
    }else{
        focused_window->width += window->x - (focused_window->x + focused_window->width);
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
        focused_window->height += focused_window->y;
        focused_window->y = 0;
    }else{
        focused_window->height += focused_window->y - (window->y + window->height);
        focused_window->y -= focused_window->y - (window->y + window->height);
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
        focused_window->height +=   workspace->height - (focused_window->y + focused_window->height);
    }else{
        focused_window->height += window->y - (focused_window->y + focused_window->height);
    }
fail:
    return -1;
}

// one thing to note here.. 
// the keysym can be difer from what the user
// actually pressed. it is just the keysym
// configured in the wm.h configuration of the
// keybindings.
int default_on_key_press(int keysym, WMWorkspace* workspace){
    switch (keysym){
        case (XK_V):
            state.mode = VERTICAL_MODE;
            break;
        case (XK_1):
            state.mode = HORIZONTAL_MODE;
            break;
        // align functions;
        case (XK_H):
            default_on_align_left(workspace);
            break;
        case (XK_J):
            default_on_align_down(workspace);
            break;
        case (XK_K):
            default_on_align_up(workspace);
            break;
        case (XK_L):
            default_on_align_right(workspace);
            break;
        default:
            break;
    }
    return 0;
}

// ---------------------------------------------------------

