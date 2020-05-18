#ifndef MONITOR_H
#define MONITOR_H


#include "common.h"
#include "workspace.h"
#include "list.h"

typedef struct{
    List list;
    List workspaces_list;
    WMWorkspace* focused_workspace;

    int number;
    int x;
    int y;
    unsigned int width;
    unsigned int height;

}WMMonitor;

WMMonitor* monitor_create(int number, int x, int y, unsigned int width, unsigned height);
void monitor_destroy(WMMonitor* monitor);

int monitor_add_workspace(WMMonitor* monitor, WMWorkspace* workspace);
int monitor_del_workspace(WMMonitor* monitor, WMWorkspace* workspace);

int monitor_focus_workspace(WMMonitor* monitor, WMWorkspace* workspace);

#endif
