#ifndef ARRANGE_H
#define ARRANGE_H

#include "common.h"
#include "list.h"
#include "workspace.h"
#include "window.h"

#define GAP (3)

int arrange_on_new_window(  WMWorkspace* workspace, 
                            WMWindow* window);

int arrange_on_del_window(  WMWorkspace* workspace, 
                            WMWindow* window);

int arrange_on_key_press(   int keysym, 
                            WMWorkspace* workspace);

#endif
