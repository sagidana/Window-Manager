#ifndef ARRANGE_TREE_H
#define ARRANGE_TREE_H

#include "arrange.h"
#include "tree.h"

int tree_on_new_window(  WMWorkspace* workspace, 
                            WMWindow* window);

int tree_on_del_window(  WMWorkspace* workspace, 
                            WMWindow* window);

int tree_on_key_press(int keysym, WMWorkspace* workspace);

#endif
