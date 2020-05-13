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

    ret = arrange_on_new_window(workspace, window);
    ASSERT(ret == 0, "faile to arrange the workspace.\n");

    ret = list_add_tail(&workspace->windows_list, &window->list);
    ASSERT(ret == 0, "failed to add window to list.\n");

    // whenever we add a new window to a 
    // workspace, change the focus to the new window.
    workspace->focused_window = window;

    return 0;

fail:
    return -1;
}

int workspace_remove_window(WMWorkspace* workspace, WMWindow* window){
    int ret;

    ret = arrange_on_del_window(workspace, window);
    ASSERT(ret == 0, "faile to arrange the workspace.\n");

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


WMWindow* workspace_get_right_window(WMWorkspace* workspace){
    WMWindow* found_window = NULL;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "no focused window.\n");

    List* curr = &workspace->windows_list;
    while(curr->next){
        curr = curr->next;
        WMWindow* curr_window = (WMWindow*) curr;

        if ((curr_window->x > focused_window->x) &&  // is to the right?
            (curr_window->y >= focused_window->y) &&
            (curr_window->y <= focused_window->y + focused_window->height)){
            // we found window in the right direction, 
            // but is it the closest?
            if (!found_window){
                found_window = curr_window;
                continue;
            }

            // we just found closer window
            if (found_window->x > curr_window->x){
                found_window = curr_window;
            }
        }
    }

fail:
    return found_window;
}

WMWindow* workspace_get_left_window(WMWorkspace* workspace){
    WMWindow* found_window = NULL;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "no focused window.\n");

    List* curr = &workspace->windows_list;
    while(curr->next){
        curr = curr->next;
        WMWindow* curr_window = (WMWindow*) curr;

        if ((curr_window->x < focused_window->x) &&  // is to the left?
            (curr_window->y >= focused_window->y) &&
            (curr_window->y <= focused_window->y + focused_window->height)){
            // we found window in the right direction, 
            // but is it the closest?
            if (!found_window){
                found_window = curr_window;
                continue;
            }

            // we just found closer window
            if (found_window->x < curr_window->x){
                found_window = curr_window;
            }
        }
    }

fail:
    return found_window;
}

WMWindow* workspace_get_up_window(WMWorkspace* workspace){
    WMWindow* found_window = NULL;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "no focused window.\n");

    List* curr = &workspace->windows_list;
    while(curr->next){
        curr = curr->next;
        WMWindow* curr_window = (WMWindow*) curr;

        if ((curr_window->y > focused_window->y) &&  // is to up?
            (curr_window->x >= focused_window->x) &&
            (curr_window->x <= focused_window->x + focused_window->width)){
            // we found window in the right direction, 
            // but is it the closest?
            if (!found_window){
                found_window = curr_window;
                continue;
            }

            // we just found closer window
            if (found_window->y > curr_window->y){
                found_window = curr_window;
            }
        }
    }

fail:
    return found_window;
}

WMWindow* workspace_get_down_window(WMWorkspace* workspace){
    WMWindow* found_window = NULL;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "no focused window.\n");

    List* curr = &workspace->windows_list;
    while(curr->next){
        curr = curr->next;
        WMWindow* curr_window = (WMWindow*) curr;

        if ((curr_window->y < focused_window->y) &&  // is down?
            (curr_window->x >= focused_window->x) &&
            (curr_window->x <= focused_window->x + focused_window->width)){
            // we found window in the right direction, 
            // but is it the closest?
            if (!found_window){
                found_window = curr_window;
                continue;
            }

            // we just found closer window
            if (found_window->y < curr_window->y){
                found_window = curr_window;
            }
        }
    }

fail:
    return found_window;
}

