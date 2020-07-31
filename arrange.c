#include "arrange.h"

#include "arrange_default.h"
// #include "arrange_tree.h"



// ---------------------------------------------------------
// choose implementation here
// ---------------------------------------------------------
int arrange_on_new_window(  WMWorkspace* workspace, 
                            WMWindow* window){
    return default_on_new_window(workspace, window);
    // return tree_on_new_window(workspace, window);
}

int arrange_on_del_window(  WMWorkspace* workspace, 
                            WMWindow* window){
    return default_on_del_window(workspace, window);
    // return tree_on_del_window(workspace, window);
}

int arrange_on_key_press(   int keysym, 
                            WMWorkspace* workspace){
    return default_on_key_press(keysym, workspace);
    // return tree_on_key_press(keysym, workspace);
}
// ---------------------------------------------------------

