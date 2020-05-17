#include "workspace.h"
#include "arrange.h"
#include "point.h"


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

int workspace_del_window(WMWorkspace* workspace, WMWindow* window){
    int ret;

    ret = arrange_on_del_window(workspace, window);
    ASSERT(ret == 0, "failed to arrange the workspace.\n");

    ret = list_del(&window->list);
    ASSERT(ret == 0, "failed to delete window from list.\n");

    // if we remove the window that is focused,
    // we need to replace it.
    if (workspace->focused_window == window){
        if (LIST_EMPTY(&workspace->windows_list)){
            workspace->focused_window = NULL;
        }else{
            // workspace->focused_window = (WMWindow*) workspace->windows_list.next;

            // in case of the first window in the workspace
            // the prev is the workspace itself, so dont focus it
            if (window->list.prev && window->list.prev != &workspace->windows_list){
                workspace->focused_window = (WMWindow*) window->list.prev;
            }else if(window->list.next){
                workspace->focused_window = (WMWindow*) window->list.next;
            }
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

// ---------------------------------------------------------------------
// Navigation
// ---------------------------------------------------------------------

Point center_point(WMWindow* window){
    Point center = {.x = -1, .y = -1};

    ASSERT(window, "window is NULL?\n");

    center.x = window->x + (window->width / 2);
    center.y = window->y + (window->height / 2);

fail:
    return center;
}

WMWindow* workspace_get_right_window(WMWorkspace* workspace){
    WMWindow* found_window = NULL;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "no focused window.\n");

    Point focused_center_point = center_point(focused_window);
    float min_found_distance = workspace->width;

    List* curr = &workspace->windows_list;
    while(curr->next){
        curr = curr->next;
        WMWindow* curr_window = (WMWindow*) curr;

        if (curr_window->x < focused_window->x + focused_window->width){
            continue;
        }
        if (curr_window->y + curr_window->height < focused_window->y){
            continue;
        }
        if (curr_window->y > focused_window->y + focused_window->height){
            continue;
        }

        Point curr_center_point = center_point(curr_window);

        float distance = point_distance( focused_center_point, 
                                                curr_center_point);

        if (min_found_distance > distance){
            min_found_distance = distance;
            found_window = curr_window;
        }
    }

fail:
    return found_window;
}

WMWindow* workspace_get_left_window(WMWorkspace* workspace){
    WMWindow* found_window = NULL;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "no focused window.\n");

    Point focused_center_point = center_point(focused_window);
    float min_found_distance = workspace->width;

    List* curr = &workspace->windows_list;
    while(curr->next){
        curr = curr->next;
        WMWindow* curr_window = (WMWindow*) curr;

        if (curr_window->x + curr_window->width > focused_window->x){
            continue;
        }
        Point curr_center_point = center_point(curr_window);

        float distance = point_distance( focused_center_point, 
                                                curr_center_point);

        if (min_found_distance > distance){
            min_found_distance = distance;
            found_window = curr_window;
        }
    }

fail:
    return found_window;
}

WMWindow* workspace_get_up_window(WMWorkspace* workspace){
    WMWindow* found_window = NULL;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "no focused window.\n");

    Point focused_center_point = center_point(focused_window);
    float min_found_distance = workspace->height;

    List* curr = &workspace->windows_list;
    while(curr->next){
        curr = curr->next;
        WMWindow* curr_window = (WMWindow*) curr;

        if (curr_window->y + curr_window->height > focused_window->y){
            continue;
        }
        Point curr_center_point = center_point(curr_window);

        float distance = point_distance( focused_center_point, 
                                                curr_center_point);

        if (min_found_distance > distance){
            min_found_distance = distance;
            found_window = curr_window;
        }
    }

fail:
    return found_window;
}

WMWindow* workspace_get_down_window(WMWorkspace* workspace){
    WMWindow* found_window = NULL;

    WMWindow* focused_window = workspace->focused_window;
    ASSERT(focused_window, "no focused window.\n");

    Point focused_center_point = center_point(focused_window);
    float min_found_distance = workspace->height;

    List* curr = &workspace->windows_list;
    while(curr->next){
        curr = curr->next;
        WMWindow* curr_window = (WMWindow*) curr;

        if (curr_window->y < focused_window->y + focused_window->height){
            continue;
        }
        Point curr_center_point = center_point(curr_window);

        float distance = point_distance( focused_center_point, 
                                                curr_center_point);

        if (min_found_distance > distance){
            min_found_distance = distance;
            found_window = curr_window;
        }
    }

fail:
    return found_window;
}

// ---------------------------------------------------------------------
