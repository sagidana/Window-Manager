#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "window.h"
#include "list.h"

typedef struct{
    List list;
    int number;
    List windows_list;
    WMWindow* focused_window;

    int x;
    int y;
    unsigned int width;
    unsigned int height;

    void* arrange_context;
}WMWorkspace;

WMWorkspace* workspace_create(  int number, 
                                int x,
                                int y,
                                unsigned int width,
                                unsigned int height);
void workspace_destroy(WMWorkspace* workspace);

int workspace_init( WMWorkspace* workspace, 
                    unsigned int width, 
                    unsigned int height);

int workspace_resize(   Display* display, 
                        WMWorkspace* workspace,
                        unsigned int width, 
                        unsigned int height);

int workspace_empty(WMWorkspace* workspace);

int workspace_hide(Display* display, WMWorkspace* workspace);
int workspace_show(Display* display, WMWorkspace* workspace);

int workspace_add_window(WMWorkspace* workspace, WMWindow* window);
int workspace_del_window(WMWorkspace* workspace, WMWindow* window);

WMWindow* workspace_get_window(WMWorkspace* workspace, Window x_window);

int workspace_has_window(WMWorkspace* workspace, WMWindow* window);

WMWindow* workspace_get_left_window(WMWorkspace* workspace);
WMWindow* workspace_get_right_window(WMWorkspace* workspace);
WMWindow* workspace_get_up_window(WMWorkspace* workspace);
WMWindow* workspace_get_down_window(WMWorkspace* workspace);

#endif
