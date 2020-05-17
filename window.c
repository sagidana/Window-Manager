#include "window.h"


WMWindow* window_create(Display* display, Window x_window){
    WMWindow* new = NULL;
    int ret;

    new = (WMWindow*)malloc(sizeof(WMWindow));
    ASSERT(new != 0, "failed with malloc\n");

    new->x_window = x_window;
    new->list.next = NULL;
    new->list.prev = NULL;
    new->visible = 1;

    new->border_width = BORDER_WIDTH;

    ret = window_update(display, new);
    ASSERT(ret == 0, "failed to create a window.\n");

    return new;

fail:
    return NULL;
}

void window_destroy(WMWindow* window){
    free(window);
}

int window_focus(   Display* display, 
                    WMWindow* window, 
                    unsigned long pixel){
    int ret;

    ASSERT(window, "window to focus is NULL.\n");

    ret = XRaiseWindow(display, window->x_window);
    ASSERT(ret, "failed to raise the window\n");

    // TODO: add change border color to focused color

    ret = XSetInputFocus(   display, 
                            window->x_window,
                            RevertToPointerRoot,
                            CurrentTime);
    ASSERT(ret, "failed to set focus for window\n");

    ret = XSetWindowBorder(display, window->x_window, pixel);
    ASSERT(ret, "failed to change border color.\n");

    return 0;

fail:
    return -1;
}

int window_unfocus( Display* display, 
                    Window root_window, 
                    WMWindow* window,
                    unsigned long pixel){
    int ret;

    ASSERT(window, "window to focus is NULL.\n");

    // TODO: add change border color to normal color

    ret = XSetInputFocus(   display, 
                            root_window,
                            RevertToPointerRoot,
                            CurrentTime);
    ASSERT(ret, "failed to unfocus the window.\n");

    ret = XSetWindowBorder(display, window->x_window, pixel);
    ASSERT(ret, "failed to change border color.\n");

    return 0;

fail:
    return -1;
}
int window_resize(  Display* display, 
                    WMWindow* window,
                    unsigned int width,
                    unsigned int height){
    int ret; 
    ret = XResizeWindow(display,
                        window->x_window,
                        width, 
                        height);
    ASSERT(ret, "failed to resize window.\n");
    return 0;

fail:
    return -1;
}

int window_move(    Display* display, 
                    WMWindow* window,
                    int x,
                    int y){
    int ret; 
    ret = XMoveWindow(  display,
                        window->x_window,
                        x, 
                        y);
    ASSERT(ret, "failed to move window.\n");
    return 0;

fail:
    return -1;
}

int window_reconfigure( Display* display, 
                        WMWindow* window, 
                        int x,
                        int y,
                        unsigned int width,
                        unsigned int height){
    int ret;
    XWindowChanges changes;
    unsigned int value_mask;

    changes.x = x;
    changes.y = y;
    changes.width = width;
    changes.height = height;
    changes.border_width = window->border_width;

    value_mask =    CWX | CWY |
                    CWWidth | CWHeight |
                    CWBorderWidth;

    ret = XConfigureWindow( display, 
                            window->x_window,
                            value_mask,
                            &changes);

    ASSERT(ret, "failed to configure window\n");

    return 0;
fail:
    return -1;
}

int window_update(Display* display, WMWindow* window){
    int ret;
    Window returned_root;
    unsigned int depth, border_width;

    ret = XGetGeometry( display, 
                        window->x_window,
                        &returned_root,
                        &window->x, &window->y,
                        &window->width, &window->height,
                        &border_width,
                        &depth);

    ASSERT(ret, "failed to get geometry\n");

    return 0;
fail:
    return -1;
}

int window_hide(Display* display, WMWindow* window){
    int ret;
    int new_x, new_y;
    new_x = window->width * (-1); 
    new_y = window->height * (-1); 

    ret = window_move(  display, 
                        window,
                        new_x,
                        new_y);
    ASSERT(ret == 0, "failed to hide window.\n");
    window->visible = 0;

    return 0;

fail:
    return -1;
}

int window_show(Display* display, WMWindow* window){
    int ret;

    ret = window_reconfigure(display, 
                             window,
                             window->x,
                             window->y,
                             window->width,
                             window->height);

    ASSERT(ret == 0, "failed to show window.\n");
    window->visible = 1;

    return 0;

fail:
    return -1;

}
