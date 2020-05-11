
#include "window.h"


WMWindow* window_create(Display* display, Window x_window){
    WMWindow* new = NULL;
    int ret;

    new = (WMWindow*)malloc(sizeof(WMWindow));
    ASSERT(new != 0, "failed with malloc\n");

    new->x_window = x_window;

    ret = window_update(display, new);
    ASSERT(ret == 0, "failed to create a window.\n");

    return new;

fail:
    return NULL;
}

void destroy_window(WMWindow* window){
    free(window);
}

int window_focus(Display* display, WMWindow* window){
    int ret;

    ret = XRaiseWindow(display, window->x_window);
    ASSERT(ret, "failed to raise the window\n");

    ret = XSetInputFocus(   display, 
                            window->x_window,
                            RevertToPointerRoot,
                            CurrentTime);
    ASSERT(ret, "failed to set focus for window\n");

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
    changes.border_width = window->width;

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
    unsigned int depth;

    ret = XGetGeometry( display, 
                        window->x_window,
                        &returned_root,
                        &window->x, &window->y,
                        &window->width, &window->height,
                        &window->border_width,
                        &depth);

    ASSERT(ret, "failed to get geometry\n");

    return 0;
fail:
    return -1;
}
