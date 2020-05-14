#ifndef ARRANGE_DEFAULT_H
#define ARRANGE_DEFAULT_H

#include "arrange.h"

int default_on_new_window(  WMWorkspace* workspace, 
                            WMWindow* window);

int default_on_del_window(  WMWorkspace* workspace, 
                            WMWindow* window);

int default_on_key_press(int keysym, WMWorkspace* workspace);

#endif
