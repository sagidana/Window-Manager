#include "arrange.h"


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

// ---------------------------------------------------------
// Default implementation
// ---------------------------------------------------------

int default_on_new_window(  WMWorkspace* workspace,
                            WMWindow* window){
    // int ret;

    List* curr = &workspace->windows_list;

    int num_of_windows = get_num_of_windows(workspace);
    int width_of_each_window = workspace->width / num_of_windows;
    int x_of_current_window = 0;

    while(curr->next){
        curr = curr->next;
        // the 'list' element inside the WMWindow struct
        // is the first element, which means that its address
        // point to the struct itself. this is why we can convert
        // the List* to WMWindow*
        WMWindow* curr_window = (WMWindow*) curr;

        curr_window->x = x_of_current_window;
        curr_window->y = 0;
        curr_window->width = width_of_each_window;
        curr_window->height = workspace->height;

        x_of_current_window += width_of_each_window;
    }

    return 0;
// fail:
    // return -1;
}

int default_on_del_window(  WMWorkspace* workspace,
                            WMWindow* window){
    return default_on_new_window(workspace, window);
}

int default_on_key_press(int keysym){
    return 0;
}

// ---------------------------------------------------------

// ---------------------------------------------------------
// choose implementation here
// ---------------------------------------------------------
int arrange_on_new_window(  WMWorkspace* workspace, 
                            WMWindow* window){
    return default_on_new_window(workspace, window);
}

int arrange_on_del_window(  WMWorkspace* workspace, 
                            WMWindow* window){
    return default_on_del_window(workspace, window);
}

int arrange_on_key_press(int keysym){
    return default_on_key_press(keysym);
}
// ---------------------------------------------------------

