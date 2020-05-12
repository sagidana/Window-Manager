#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "window.h"
#include "list.h"

#define NUM_OF_WORKSPACES (32)

typedef struct{
    List windows_list;
    WMWindow* focused_window;
}WMWorkspace;

int workspace_init(WMWorkspace* workspace);

int workspace_add_window(WMWorkspace* workspace, WMWindow* window);
int workspace_remove_window(WMWorkspace* workspace, WMWindow* window);

WMWindow* workspace_get_window(WMWorkspace* workspace, Window x_window);

int workspace_has_window(WMWorkspace* workspace, WMWindow* window);

#endif
