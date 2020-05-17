#include "wm.h"

#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <errno.h>

#include "arrange.h"


#define WORKSPACE (&wm.workspaces[wm.current_workspace_index])
#define WINDOW (WORKSPACE->focused_window)

#define IS_ACTIVE_WORKSPACE(ws) (ws == WORKSPACE)

// -----------------------------------------------------
// helpers functions
// -----------------------------------------------------
Key* get_key_by_event(Display* display, XKeyEvent* event){
#define ONLY_MODIFIERS(x) (x & (ShiftMask   | \
                                ControlMask | \
                                Mod1Mask    | \
                                Mod2Mask    | \
                                Mod3Mask    | \
                                Mod4Mask    | \
                                Mod5Mask))
    int i;
    for (i = 0; i < LENGTH(wm_keys); i++){
        if (event->keycode != XKeysymToKeycode(display, wm_keys[i].keysym)){
            continue;
        }
        if (ONLY_MODIFIERS(event->state) != ONLY_MODIFIERS(wm_keys[i].mod)){
            continue;
        }

        return &wm_keys[i];
    }
    return NULL;
}

WMWindow* get_wmwindow_by_x_window(Window x_window){
    WMWindow* window = NULL;
    int i;

    for (i = 0; i < NUM_OF_WORKSPACES; i++){
        window = workspace_get_window(&wm.workspaces[i], x_window);
        if (window != NULL){ 
            break;
        }
    }

    return window;
}

WMWorkspace* get_workspace_by_window(WMWindow* window){
    int i;
    for (i = 0; i < NUM_OF_WORKSPACES; i++){
        if(workspace_has_window(&wm.workspaces[i], window)){
            return &wm.workspaces[i];
        }
    }
    return NULL;
}

// -----------------------------------------------------
// events callbacks
// -----------------------------------------------------
void on_default(XEvent* e){}

void on_configure_request(XEvent* e){
    // currently just forward the request

    XConfigureRequestEvent* event = &e->xconfigurerequest;
    XWindowChanges changes;

    changes.x = event->x;
    changes.y = event->y;
    changes.width = event->width;
    changes.height = event->height;
    changes.border_width = event->border_width;
    changes.sibling = event->above;
    changes.stack_mode = event->detail;

    XConfigureWindow(wm.display, 
                    event->window, 
                    event->value_mask, 
                    &changes);
    XSync(wm.display, FALSE);
}

void on_map_request(XEvent* e){
    int ret;

    // currently just forward the request
    XMapRequestEvent* event = &e->xmaprequest;

    XMapWindow(wm.display, event->window);

    // this is where we add the new window to our
    // window manager!!
    WMWindow* window = window_create(wm.display, event->window);
    ASSERT(window, "failed to create window on_map_request.\n");

    if (WINDOW){
        ret = window_unfocus(   wm.display, 
                wm.root_window, 
                WINDOW, 
                wm.normal_window_color.pixel);
        ASSERT(ret == 0, "failed to unfocus previous window");
    }

    ret = workspace_add_window(WORKSPACE, window);
    ASSERT(ret == 0, "failed to add window to workspace.\n");

    ret = workspace_show(wm.display, WORKSPACE);
    ASSERT(ret == 0, "failed to show workspace.\n");

    ret = window_focus( wm.display, 
                        WINDOW, 
                        wm.focused_window_color.pixel);
    ASSERT(ret == 0, "failed to focus new window.\n");

fail:
    return;
}

void on_key_press(XEvent* e){
    XKeyEvent* event = &e->xkey;

    LOG("Key pressed: %d\n", event->keycode);

    Key* current_key = get_key_by_event(wm.display, event);
    if (current_key == NULL){
        return;
    }

    if (current_key->func){
        current_key->func(&current_key->args);
    }
}

void on_unmap_notify(XEvent* e){
    int ret;
    XUnmapEvent* event = &e->xunmap;

    WMWindow* window = get_wmwindow_by_x_window(event->window);
    ASSERT(window, "window manager could not find window.\n");

    WMWorkspace* workspace = get_workspace_by_window(window);
    ASSERT(workspace, "window manager could not find workspace.\n");

    ret = workspace_del_window(workspace, window);
    ASSERT(ret == 0, "failed to remove window from workspace.\n");

    window_destroy(window);

    // focus the new window in case we remove from
    // active workspace
    if (IS_ACTIVE_WORKSPACE(workspace)){
        ret = workspace_show(wm.display, WORKSPACE);

        if (WINDOW){
            ret = window_focus( wm.display, 
                                WINDOW, 
                                wm.focused_window_color.pixel);
        }
    }

fail:
    return;
}
void on_configure_notify(XEvent* e){
    int ret;
    int i;
    XConfigureEvent* event = &e->xconfigure;

    if (event->window != wm.root_window){
        return;
    }

    // resize screen.
    for (i = 0; i < NUM_OF_WORKSPACES; i++){
        ret = workspace_resize( wm.display, 
                                &wm.workspaces[i],
                                (unsigned int)event->width,
                                (unsigned int)event->height);

        ASSERT(ret == 0, "failed resizing workspace.\n");
    }

fail:
    return;
}

// -----------------------------------------------------
// event_handlers declerations
// -----------------------------------------------------
static void (*event_handlers[LASTEvent]) (XEvent *) = {
	[ButtonPress]       = on_default,
	[ClientMessage]     = on_default,
	[ConfigureRequest]  = on_configure_request,
	[ConfigureNotify]   = on_configure_notify,
    [CreateNotify]      = on_default,
	[DestroyNotify]     = on_default,
	[EnterNotify]       = on_default,
	[Expose]            = on_default,
	[FocusIn]           = on_default,
	[KeyPress]          = on_key_press,
	[MappingNotify]     = on_default,
	[MapRequest]        = on_map_request,
	[MotionNotify]      = on_default,
	[PropertyNotify]    = on_default,
	[UnmapNotify]       = on_unmap_notify
};

int x_on_error(Display* display, XErrorEvent* e){
    // TODO:

    return 0;
}

int x_on_wm_detected(Display* display, XErrorEvent* e){
    LOG("another window manager was detected :(\n");

    exit(1);
}

void detect_other_wm(){
    // check if another 
    XSetErrorHandler(x_on_wm_detected);

    XSelectInput(   wm.display, 
                    wm.root_window,
                    SubstructureRedirectMask | SubstructureNotifyMask);

    XSync(wm.display, FALSE);
}


// -----------------------------------------------------
// functions for the key bindings events
// -----------------------------------------------------

void arrange(Args* args){
    arrange_on_key_press(args->i, WORKSPACE);

    workspace_show(wm.display, WORKSPACE);
}

void switch_workspace(Args* args){
    int ret;

    if ((args->i < 0) || args->i >= NUM_OF_WORKSPACES){
        ASSERT(FALSE, "failed to switch workspace i = %d\n", args->i);
    }

    // we already at the correct workspace
    if (wm.current_workspace_index == args->i){
        return;
    }

    if (WINDOW){ // only if there is a window.
        ret = window_unfocus(   wm.display, 
                                wm.root_window, 
                                WINDOW, 
                                wm.normal_window_color.pixel);
        ASSERT(ret == 0, "failed to unfocus prev window.\n");
    }

    ret = workspace_hide(wm.display, WORKSPACE);
    ASSERT(ret == 0, "failed to hide workspace\n");

    wm.current_workspace_index = args->i;

    ret = workspace_show(wm.display, WORKSPACE);
    ASSERT(ret == 0, "failed to show workspace\n");

    if (WINDOW){ // only if there is a window.
        ret = window_focus( wm.display, 
                            WINDOW, 
                            wm.focused_window_color.pixel);
        ASSERT(ret == 0, "failed to focus window.\n");
    }

fail:
    return;
}

void kill(Args* args){
    XDestroyWindow(wm.display, WINDOW->x_window);
    XSync(wm.display, FALSE);
}

void spawn(Args* args){
    char** argv = (char**)args->ptr;

    int ret = fork();
    if (ret == 0){   // i am the child
        // clean xorg-server connection on child
        if (wm.display){
            close(((_XPrivDisplay)wm.display)->fd);
        }
        setsid();
        execvp(argv[0], argv);
        exit(0);
    }
}
void to_exit(Args* args){
    // trigger wm to exit.
    wm.to_exit = 1; 
}

void focus_left(Args* args){
    int ret;

    ASSERT(WINDOW, "no window to focus\n");

    WMWindow* window = workspace_get_left_window(WORKSPACE);
    ASSERT(window, "no window found.\n");

    ret = window_unfocus(   wm.display, 
                            wm.root_window, 
                            WINDOW, 
                            wm.normal_window_color.pixel);
    ASSERT(ret == 0, "unable to unfocus window.\n");

    // switch focused window
    WORKSPACE->focused_window = window; 

    ret = window_focus( wm.display, 
                        WINDOW, 
                        wm.focused_window_color.pixel);
    ASSERT(ret == 0, "unable to focus window.\n");

fail:
    return;
}

void focus_right(Args* args){
    int ret;

    ASSERT(WINDOW, "no window to focus\n");

    WMWindow* window = workspace_get_right_window(WORKSPACE);
    ASSERT(window, "no window found.\n");

    ret = window_unfocus(   wm.display, 
                            wm.root_window, 
                            WINDOW, 
                            wm.normal_window_color.pixel);
    ASSERT(ret == 0, "unable to unfocus window.\n");

    // switch focused window
    WORKSPACE->focused_window = window; 

    ret = window_focus( wm.display, 
                        WINDOW, 
                        wm.focused_window_color.pixel);
    ASSERT(ret == 0, "unable to focus window.\n");

fail:
    return;
}

void focus_up(Args* args){
    int ret;

    ASSERT(WINDOW, "no window to focus\n");

    WMWindow* window = workspace_get_up_window(WORKSPACE);
    ASSERT(window, "no window found.\n");

    ret = window_unfocus(   wm.display, 
                            wm.root_window, 
                            WINDOW, 
                            wm.normal_window_color.pixel);
    ASSERT(ret == 0, "unable to unfocus window.\n");

    // switch focused window
    WORKSPACE->focused_window = window; 

    ret = window_focus( wm.display, 
                        WINDOW, 
                        wm.focused_window_color.pixel);
    ASSERT(ret == 0, "unable to focus window.\n");

fail:
    return;
}

void focus_down(Args* args){
    int ret;

    ASSERT(WINDOW, "no window to focus\n");

    WMWindow* window = workspace_get_down_window(WORKSPACE);
    ASSERT(window, "no window found.\n");

    ret = window_unfocus(   wm.display, 
                            wm.root_window, 
                            WINDOW, 
                            wm.normal_window_color.pixel);
    ASSERT(ret == 0, "unable to unfocus window.\n");

    // switch focused window
    WORKSPACE->focused_window = window; 

    ret = window_focus( wm.display, 
                        WINDOW, 
                        wm.focused_window_color.pixel);
    ASSERT(ret == 0, "unable to focus window.\n");

fail:
    return;
}


void move_left(Args* args){
    ASSERT(WINDOW, "no window to move\n");

    WMWindow* window = workspace_get_left_window(WORKSPACE);
    ASSERT(window, "no window found.\n");

    int x, y;
    unsigned int width, height;

    x = window->x;
    y = window->y;
    width = window->width;
    height = window->height;

    window->x = WINDOW->x;
    window->y = WINDOW->y;
    window->width = WINDOW->width;
    window->height = WINDOW->height;

    WINDOW->x = x;
    WINDOW->y = y;
    WINDOW->width = width;
    WINDOW->height = height;

    workspace_show(wm.display, WORKSPACE);

fail:
    return;
}

void move_right(Args* args){
    ASSERT(WINDOW, "no window to move\n");

    WMWindow* window = workspace_get_right_window(WORKSPACE);
    ASSERT(window, "no window found.\n");

    int x, y;
    unsigned int width, height;

    x = window->x;
    y = window->y;
    width = window->width;
    height = window->height;

    window->x = WINDOW->x;
    window->y = WINDOW->y;
    window->width = WINDOW->width;
    window->height = WINDOW->height;

    WINDOW->x = x;
    WINDOW->y = y;
    WINDOW->width = width;
    WINDOW->height = height;

    workspace_show(wm.display, WORKSPACE);

fail:
    return;
}

void move_up(Args* args){
    ASSERT(WINDOW, "no window to move\n");

    WMWindow* window = workspace_get_up_window(WORKSPACE);
    ASSERT(window, "no window found.\n");

    int x, y;
    unsigned int width, height;

    x = window->x;
    y = window->y;
    width = window->width;
    height = window->height;

    window->x = WINDOW->x;
    window->y = WINDOW->y;
    window->width = WINDOW->width;
    window->height = WINDOW->height;

    WINDOW->x = x;
    WINDOW->y = y;
    WINDOW->width = width;
    WINDOW->height = height;

    workspace_show(wm.display, WORKSPACE);
fail:
    return;
}

void move_down(Args* args){

    WMWindow* window = workspace_get_down_window(WORKSPACE);
    ASSERT(window, "no window found.\n");

    int x, y;
    unsigned int width, height;

    x = window->x;
    y = window->y;
    width = window->width;
    height = window->height;

    window->x = WINDOW->x;
    window->y = WINDOW->y;
    window->width = WINDOW->width;
    window->height = WINDOW->height;

    WINDOW->x = x;
    WINDOW->y = y;
    WINDOW->width = width;
    WINDOW->height = height;

    workspace_show(wm.display, WORKSPACE);
    ASSERT(WINDOW, "no window to move\n");
fail:
    return;
}

// -----------------------------------------------------

int unregister_key_event(){
    int ret;

    ret = XUngrabKey(wm.display, AnyKey, AnyModifier, wm.root_window);
    ASSERT(ret, "failed to ungrab all keys.\n");
    return 0;

fail:
    return -1;
}

int register_key_events(){
    int i;
    int ret;

    ret = unregister_key_event();
    ASSERT(ret == 0, "failed to unregiester all keys events.\n");

    for (i = 0; i < LENGTH(wm_keys); i++){
        // accept events only from the key presses 
        // that of interest to us by the keys array
        // which should be defined by the user.

        ret = XGrabKey( wm.display,
                        XKeysymToKeycode(wm.display, wm_keys[i].keysym),
                        wm_keys[i].mod, 
                        wm.root_window,
                        TRUE,
                        GrabModeAsync,
                        GrabModeAsync);

        ASSERT(ret, "failed in XGrabKey()\n");
    }

    return 0;
fail:
    return -1;
}

int create_colors(){
    int ret;
    int screen = DefaultScreen(wm.display);

    Colormap colormap = DefaultColormap(wm.display, screen);

    ret = XParseColor(wm.display, colormap, focused_window_color, &wm.focused_window_color);
    ASSERT(ret, "failed to parse color.\n");
    ret = XParseColor(wm.display, colormap, normal_window_color, &wm.normal_window_color);
    ASSERT(ret, "failed to parse color.\n");
    // XParseColor(wm.display, colormap, "rgb:cc/cc/cc", &wm.focused_window_color);
    // XParseColor(wm.display, colormap, "rgb:22/22/22", &wm.normal_window_color);

    ret = XAllocColor(wm.display, colormap, &wm.focused_window_color);
    ASSERT(ret, "failed to alloc color.\n");
    ret = XAllocColor(wm.display, colormap, &wm.normal_window_color);
    ASSERT(ret, "failed to alloc color.\n");

    return 0;

fail:
    return -1;
}

int initialize_wm(){
    int ret;
    int i;

    wm.to_exit = 0;

    XSetErrorHandler(x_on_error);

    ret = register_key_events();
    ASSERT(ret == 0, "failed to register to key events\n");

    XSelectInput(   wm.display, 
                    wm.root_window,
                    SubstructureRedirectMask    | 
                    SubstructureNotifyMask      |
                    PropertyChangeMask          |
                    StructureNotifyMask);

    // unused variables
    Window returned_root;
    int x, y;
    unsigned int border_width;
    unsigned int depth;

    unsigned int screen_width = 0;
    unsigned int screen_height = 0;

    ret = XGetGeometry( wm.display, 
                        wm.root_window,
                        &returned_root,
                        &x, &y,
                        &screen_width, &screen_height,
                        &border_width,
                        &depth);
    ASSERT(ret, "failed to get screen geometry\n");

    // the first workspace will be 1 not 0.
    wm.current_workspace_index = 1; 
    for (i = 0; i < NUM_OF_WORKSPACES; i++){

        ret = workspace_init(   &wm.workspaces[i],
                                screen_width,
                                screen_height);

        ASSERT(ret == 0, "failed to init workspaces.\n");
    }
    create_colors();

    return 0;

fail:
    return -1;
}

int start(){
    int ret;
    wm.display = XOpenDisplay(NULL); // create connection
    ASSERT(wm.display, "failed to open display\n");

    wm.root_window = DefaultRootWindow(wm.display);

    detect_other_wm();
    
    ret = initialize_wm();
    ASSERT(ret == 0, "failed to initialize wm.\n");

    XSync(wm.display, FALSE);
    return 0;

fail:
    return -1;
}

int end(){
    unregister_key_event();

    XCloseDisplay(wm.display);

    wm.display = NULL;

    return 0;
}

void main_event_loop(){
    XEvent e;

    // char* termcmd[] = {"st", NULL};
    // Args args = {.ptr = (void*)termcmd};
    // spawn(&args);

    while(TRUE){
        // fetch next event
        XNextEvent(wm.display, &e);

        LOG("Event type: %d\n", e.type);

        // execute right handler.

        if (event_handlers[e.type]){
            event_handlers[e.type](&e);
        }

        XSync(wm.display, FALSE);

        if (wm.to_exit) break;
    }
}

int main(){
    if (start())
        return -1;

    LOG("wm has started!\n");

    main_event_loop();

    LOG("wm has finished!\n");
    return end();
}
