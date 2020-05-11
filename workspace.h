#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "window.h"
#include "list.h"

#define NUM_OF_WORKSPACES (32)

typedef struct{
    List windows;
    WMWindow* focused_window;
}WMWorkspace;

int workspace_init(WMWorkspace* workspace);

#endif
