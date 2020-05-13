#include "workspace.h"
#include "arrange.h"


int workspace_init( WMWorkspace* workspace, 
                    unsigned int width, 
                    unsigned int height){

    workspace->windows_list.next = NULL;
    workspace->windows_list.prev = NULL;
    workspace->focused_window = NULL;
    workspace->height = height;
    workspace->width = width;

    return 0;
}

int workspace_hide(Display* display, WMWorkspace* workspace){
    int ret;

    List* curr = &workspace->windows_list;

    while(curr->next){
        curr = curr->next;
        // the 'list' element inside the WMWindow struct
        // is the first element, which means that its address
        // point to the struct itself. this is why we can convert
        // the List* to WMWindow*
        WMWindow* curr_window = (WMWindow*) curr;

        ret = window_hide(display, curr_window);
        ASSERT(ret == 0, "failed to hide window\n");
    }

    return 0;
fail:
    return -1;
}

int workspace_show(Display* display, WMWorkspace* workspace){
    int ret;

    List* curr = &workspace->windows_list;

    while(curr->next){
        curr = curr->next;
        // the 'list' element inside the WMWindow struct
        // is the first element, which means that its address
        // point to the struct itself. this is why we can convert
        // the List* to WMWindow*
        WMWindow* curr_window = (WMWindow*) curr;

        ret = window_show(display, curr_window);
        ASSERT(ret == 0, "failed to show window\n");
    }

    return 0;
fail:
    return -1;
}

int workspace_add_window(WMWorkspace* workspace, WMWindow* window){
    int ret;

    ret = list_add_tail(&workspace->windows_list, &window->list);
    ASSERT(ret == 0, "failed to add window to list.\n");

    // whenever we add a new window to a 
    // workspace, change the focus to the new window.
    workspace->focused_window = window;

    ret = arrange_workspace(workspace);
    ASSERT(ret == 0, "faile to arrange the workspace.\n");

    return 0;

fail:
    return -1;
}

int workspace_remove_window(WMWorkspace* workspace, WMWindow* window){
    int ret;

    ret = list_del(&window->list);
    ASSERT(ret == 0, "failed to delete window from list.\n");

    // if we remove the window that is focused,
    // we need to replace it.
    if (workspace->focused_window == window){
        if (LIST_EMPTY(&workspace->windows_list)){
            workspace->focused_window = NULL;
        }else{
            workspace->focused_window = (WMWindow*) workspace->windows_list.next;
        }
    }

    ret = arrange_workspace(workspace);
    ASSERT(ret == 0, "faile to arrange the workspace.\n");

    return 0;

fail:
    return -1;
}

WMWindow* workspace_get_window(WMWorkspace* workspace, Window x_window){
    WMWindow* window = NULL;
    List* curr = &workspace->windows_list;

    while(curr->next){
        curr = curr->next;
        // the 'list' element inside the WMWindow struct
        // is the first element, which means that its address
        // point to the struct itself. this is why we can convert
        // the List* to WMWindow*
        WMWindow* curr_window = (WMWindow*) curr;

        if (curr_window->x_window == x_window){
            window = curr_window;
            break;
        }
    }

    return window;
}

int workspace_has_window(WMWorkspace* workspace, WMWindow* window){
    List* curr = &workspace->windows_list;
    while(curr->next){
        curr = curr->next;
        // the 'list' element inside the WMWindow struct
        // is the first element, which means that its address
        // point to the struct itself. this is why we can convert
        // the List* to WMWindow*
        WMWindow* curr_window = (WMWindow*) curr;

        if (curr_window == window){
            return TRUE;
        }
    }
    return FALSE;
}
