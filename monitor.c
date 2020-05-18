#include "monitor.h"


WMMonitor* monitor_create(int x, int y, unsigned int width, unsigned height){
    WMMonitor* monitor = NULL;

    monitor = (WMMonitor*) malloc(sizeof(WMMonitor));
    ASSERT(monitor, "cant malloc new monitor.\n");

    monitor->workspaces_list.prev = NULL;
    monitor->workspaces_list.next = NULL;
    monitor->x = x;
    monitor->y = y;
    monitor->width = width;
    monitor->height = height;
    monitor->focused_workspace = NULL;

fail:
    return monitor;
}


void monitor_destroy(WMMonitor* monitor){
    free(monitor);
}

int monitor_add_workspace(WMMonitor* monitor, WMWorkspace* workspace){
    int ret;
    ret = list_add_tail(&monitor->workspaces_list, &workspace->list);
    ASSERT(ret == 0, "failed to add to list\n");

    ret = monitor_focus_workspace(monitor, workspace);
    ASSERT(ret == 0, "failed to focus new workspace\n");

    return 0;

fail:
    return -1;
}

int monitor_del_workspace(WMMonitor* monitor, WMWorkspace* workspace){
    int ret;

    ret = list_del(&workspace->list);
    ASSERT(ret == 0, "failed to delete from list\n");

    if (monitor->focused_workspace == workspace){
        if (LIST_EMPTY(&monitor->workspaces_list)){
            monitor->focused_workspace = NULL;
        }else{
            if (workspace->list.prev && workspace->list.prev != &monitor->workspaces_list){
                monitor->focused_workspace = (WMWorkspace*) workspace->list.prev;
            }else if(workspace->list.next){
                monitor->focused_workspace = (WMWorkspace*) workspace->list.next;
            }
        }
    }
    return 0;

fail:
    return -1;
}

int monitor_focus_workspace(WMMonitor* monitor, WMWorkspace* workspace){
    if (monitor->focused_workspace == NULL){
        monitor->focused_workspace = workspace;
        return 0;
    }
    if (monitor->focused_workspace == workspace){
        return 0; // do nothing.
    }

    if (workspace_empty(monitor->focused_workspace)){
        WMWorkspace* tmp = monitor->focused_workspace;

        monitor->focused_workspace = workspace;

        monitor_del_workspace(monitor, tmp);
        workspace_destroy(tmp);
    }else{
        monitor->focused_workspace = workspace;
    }

    return 0;
}
