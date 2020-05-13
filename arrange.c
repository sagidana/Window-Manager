#include "arrange.h"



// ---------------------------------------------------------
// Default implementation
// ---------------------------------------------------------

int default_on_new_window(  WMWorkspace* workspace,
                            WMWindow* window){
    // int ret;

    List* curr = &workspace->windows_list;

    while(curr->next){
        curr = curr->next;
        // the 'list' element inside the WMWindow struct
        // is the first element, which means that its address
        // point to the struct itself. this is why we can convert
        // the List* to WMWindow*
        WMWindow* curr_window = (WMWindow*) curr;

        curr_window->x = 0;
        curr_window->y = 0;
        curr_window->width = workspace->width;
        curr_window->height = workspace->height;
    }

    return 0;
// fail:
    // return -1;
}

int default_on_del_window(  WMWorkspace* workspace,
                            WMWindow* window){
    return 0;
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

