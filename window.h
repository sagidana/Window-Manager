#ifndef WINDOW_H
#define WINDOW_H

#include "common.h"

typedef struct{
    List list;
    int x;
    int y;
    unsigned int width;
    unsigned int height;
    unsigned int border_width;

    Window x_window;
}WMWindow;

WMWindow* window_create(Display* display, Window x_window);
void window_destroy(WMWindow* window);

int window_focus(Display* display, WMWindow* window);
int window_reconfigure( Display* display, 
                        WMWindow* window, 
                        int x,
                        int y,
                        unsigned int width,
                        unsigned int height);

int window_resize(  Display* display, 
                    WMWindow* window,
                    unsigned int width,
                    unsigned int height);

int window_move(    Display* display, 
                    WMWindow* window,
                    int x,
                    int y);

int window_update(Display* display, WMWindow* window);

#endif
