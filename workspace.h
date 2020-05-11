#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "window.h"

#define NUM_OF_WORKSPACES (32)

typedef struct{
    WMWindow* windows;
}WMWorkspace;

#endif
