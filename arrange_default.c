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
    if(state.mode == VERTICAL_MODE){
        focused_window->width /= 2;

        window->x = focused_window->x + focused_window->width;
        window->y = focused_window->y;
        window->width = focused_window->width;
        window->height = focused_window->height;
    }else if (state.mode == HORIZONTAL_MODE){
        focused_window->height /= 2;

        window->x = focused_window->x;
        window->y = focused_window->y + focused_window->height;
        window->width = focused_window->width;
        window->height = focused_window->height;
    }

    return 0;
}

int default_on_del_window(  WMWorkspace* workspace,
                            WMWindow* window){
    if (get_num_of_windows(workspace) == 0){ 
        return 0;
    }

    return default_on_new_window(workspace, window);
}

// one thing to note here.. 
// the keysym can be difer from what the user
// actually pressed. it is just the keysym
// configured in the wm.h configuration of the
// keybindings.
int default_on_key_press(int keysym){
    switch (keysym){
        case (XK_V):
            state.mode = VERTICAL_MODE;
            break;
        case (XK_H):
            state.mode = HORIZONTAL_MODE;
            break;
        default:
            break;
    }
    return 0;
}

// ---------------------------------------------------------

