#include "arrange.h"


int arrange_workspace(WMWorkspace* workspace){
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
