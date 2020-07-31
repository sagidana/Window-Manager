#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <errno.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xinerama.h> // multi-monitor

#include "wm.h"
#include "arrange.h"


#define MONITOR (wm.focused_monitor)
#define WORKSPACE (MONITOR->focused_workspace)
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
    List* curr_1 = &wm.monitors_list;
    while (curr_1->next){
        curr_1 = curr_1->next;
        WMMonitor* monitor = (WMMonitor*)curr_1;

        List* curr_2 = &monitor->workspaces_list;
        while (curr_2->next){
            curr_2 = curr_2->next;
            WMWorkspace* workspace = (WMWorkspace*)curr_2;

            List* curr_3 = &workspace->windows_list;
            while (curr_3->next){
                curr_3 = curr_3->next;
                WMWindow* window = (WMWindow*)curr_3;

                if (window->x_window == x_window){
                    return window;
                }
            }
        }
    }
    return NULL;
}

WMWorkspace* get_workspace_by_window(WMWindow* window){
    List* curr_1 = &wm.monitors_list;
    while (curr_1->next){
        curr_1 = curr_1->next;
        WMMonitor* monitor = (WMMonitor*)curr_1;

        List* curr_2 = &monitor->workspaces_list;
        while (curr_2->next){
            curr_2 = curr_2->next;
            WMWorkspace* workspace = (WMWorkspace*)curr_2;

            List* curr_3 = &workspace->windows_list;
            while (curr_3->next){
                curr_3 = curr_3->next;
                WMWindow* curr_window = (WMWindow*)curr_3;

                if (curr_window == window){
                    return workspace;
                }
            }
        }
    }
    return NULL;
}

WMMonitor* get_monitor_by_workspace(WMWorkspace* workspace){
    List* curr_1 = &wm.monitors_list;
    while (curr_1->next){
        curr_1 = curr_1->next;
        WMMonitor* monitor = (WMMonitor*)curr_1;

        List* curr_2 = &monitor->workspaces_list;
        while (curr_2->next){
            curr_2 = curr_2->next;
            WMWorkspace* curr_workspace = (WMWorkspace*)curr_2;

            if (curr_workspace == workspace){
                return monitor;
            }
        }
    }

    return NULL;
}

WMWorkspace* get_workspace_by_number(int number){
    WMWorkspace* found_workspace = NULL;
    List* curr_1 = &wm.monitors_list;
    while (curr_1->next){
        curr_1 = curr_1->next;
        WMMonitor* monitor = (WMMonitor*)curr_1;

        List* curr_2 = &monitor->workspaces_list;
        while (curr_2->next){
            curr_2 = curr_2->next;
            WMWorkspace* workspace = (WMWorkspace*)curr_2;

            if (workspace->number == number){
                found_workspace = workspace;
            }
        }
    }
    return found_workspace;
}

int next_workspace_number(){
    int found_number = 1;
    int prev_found_number = -1;

    while (found_number != prev_found_number){
        prev_found_number = found_number;

        List* curr_1 = &wm.monitors_list;
        while (curr_1->next){
            curr_1 = curr_1->next;
            WMMonitor* monitor = (WMMonitor*)curr_1;

            List* curr_2 = &monitor->workspaces_list;
            while (curr_2->next){
                curr_2 = curr_2->next;
                WMWorkspace* workspace = (WMWorkspace*)curr_2;

                if (workspace->number == found_number){
                    found_number++;
                }
            }
        }
    }
    return found_number;
}

int send_event(WMWindow* window, Atom proto);

// -----------------------------------------------------
// events callbacks
// -----------------------------------------------------

void on_default(XEvent* e){
}

void on_configure_request(XEvent* e){
    // currently just forward the request

    XConfigureRequestEvent* event = &e->xconfigurerequest;
    XWindowChanges changes;

    WMWindow* found = get_wmwindow_by_x_window(event->window);
    // do not let reconfigure already managed window.
    if (found){
        return;
    }

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

    WMMonitor* monitor = get_monitor_by_workspace(workspace);
    ASSERT(monitor, "window manager could not find monitor.\n");

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
    }else{
        if (workspace_empty(workspace)){
            ret = monitor_del_workspace(monitor, workspace);
            ASSERT(ret == 0, "failed to delete workspace from monitor.\n");

            LOG("destroyed workspace number: %d.\n",workspace->number);
            workspace_destroy(workspace);
        }
    }

fail:
    return;
}

void on_configure_notify(XEvent* e){
    XConfigureEvent* event = &e->xconfigure;

    if (event->window != wm.root_window){
        return;
    }

    // TODO: dynamic adjust to changes in layout:
    // - change in number of monitors
    // - change in size of monitors

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
    LOG("x_on_error()\n");

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

    WMWorkspace* workspace = get_workspace_by_number(args->i);
    
    // nothing to do.
    if (workspace == WORKSPACE){
        return;
    }

    if (WINDOW){ // only if there is a window.
        ret = window_unfocus(   wm.display, 
                                wm.root_window, 
                                WINDOW, 
                                wm.normal_window_color.pixel);
        ASSERT(ret == 0, "failed to unfocus prev window.\n");
    }

    // if workspace is not exist create it.
    if (workspace == NULL){
        workspace = workspace_create(   args->i, 
                                        MONITOR->x,
                                        MONITOR->y,
                                        MONITOR->width,
                                        MONITOR->height);
        ASSERT(workspace, "failed to create a workspace.\n");

        ret = workspace_hide(wm.display, WORKSPACE);
        ASSERT(ret == 0, "failed to hide workspace\n");

        ret = monitor_add_workspace(MONITOR, workspace);
        ASSERT(ret == 0, "failed adding workspace to monitor.\n");

    // if workspace exist change to focused
    }else{
        WMMonitor* next_monitor = get_monitor_by_workspace(workspace);
        ASSERT(next_monitor, "failed to get monitor from workspace.\n");

        ret = workspace_hide(wm.display, next_monitor->focused_workspace);
        ASSERT(ret == 0, "failed to hide workspace\n");

        ret = monitor_focus_workspace(next_monitor, workspace);
        ASSERT(ret == 0, "failed to focus workspace\n");

        // might not changed
        wm.focused_monitor = next_monitor;
    }

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
    int ret;
    ASSERT(WINDOW, "not window to kill.\n");

    Atom protocol = XInternAtom(    wm.display,
                                    "WM_DELETE_WINDOW",
                                    FALSE);
    ret = send_event(WINDOW, protocol);
    ASSERT_TO(fail_to_kill, (ret == 0), "failed to send delete event to client.\n");

    return;

fail_to_kill:
    // force killing the window when cannot send delete event.

    // lock the x server for changes
    XGrabServer(wm.display);

    // XSetErrorHandler(x_on_error_empty);

    // copied from dwm because of defunct problem.
    // TODO: understand better.
    XSetCloseDownMode(wm.display, DestroyAll);

    XKillClient(wm.display, WINDOW->x_window);

    XSync(wm.display, FALSE);

    // XSetErrorHandler(x_on_error);

    XUngrabServer(wm.display);

fail:
    return;
}

void spawn(Args* args){
    char** argv = (char**)args->ptr;

    char dmenu_mon_string[] = "0";

    // in case of dmenu. change the monitor the dmenu appears in
    if ((char**)dmenucmd == argv){
        LOG("dmenu is called.\n");
        dmenu_mon_string[0] = (char)(0x30 + MONITOR->number);
        argv[2] = dmenu_mon_string;

        LOG("dmenu is %s.\n", argv[2]);
    }

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

int send_event(WMWindow* window, Atom protocol){
    int ret;
    int found = 0;
    int num_of_protocols;
    Atom* protocols;

    // search protocol in target client.
    ret = XGetWMProtocols(wm.display, window->x_window, &protocols, &num_of_protocols);
    if (ret){
        while (num_of_protocols--){
            if (protocols[num_of_protocols] == protocol){
                found = 1;
            }
        }
        XFree(protocols);
    }
    ASSERT(found, "protocol not found in target client.\n");

    XEvent event;
    event.type = ClientMessage;
    event.xclient.window = window->x_window;
    event.xclient.message_type = XInternAtom(wm.display, "WM_PROTOCOLS", FALSE);
    event.xclient.format = 32;
    event.xclient.data.l[0] = protocol;
    event.xclient.data.l[1] = CurrentTime;

    XSendEvent( wm.display, 
                window->x_window, 
                FALSE, 
                NoEventMask, 
                &event);

    return 0;

fail:
    return -1;
}

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

    ret = XAllocColor(wm.display, colormap, &wm.focused_window_color);
    ASSERT(ret, "failed to alloc color.\n");
    ret = XAllocColor(wm.display, colormap, &wm.normal_window_color);
    ASSERT(ret, "failed to alloc color.\n");

    return 0;

fail:
    return -1;
}

int destroy_colors(){
    int screen = DefaultScreen(wm.display);
    Colormap colormap = DefaultColormap(wm.display, screen);

    XFreeColors(wm.display, 
                colormap,
                &wm.focused_window_color.pixel,
                1,
                0);
    XFreeColors(wm.display, 
                colormap,
                &wm.normal_window_color.pixel,
                1,
                0);
    return 0;
}

int monitors_setup(){
    int num_of_monitors;
    int ret;
    int i;

    wm.monitors_list.prev = NULL;
    wm.monitors_list.next = NULL;

    ASSERT(XineramaIsActive(wm.display), "Xinerama is not active.\n");

    XineramaScreenInfo* info = XineramaQueryScreens(wm.display, &num_of_monitors);
    for (i = 0; i < num_of_monitors; i++){
        WMMonitor* monitor = monitor_create(i,
                                            (int)info[i].x_org,
                                            (int)info[i].y_org,
                                            (unsigned int)info[i].width,
                                            (unsigned int)info[i].height);
        ASSERT(monitor, "failed to add monitor to wm.\n");

        ret = list_add_tail(&wm.monitors_list, &monitor->list);
        ASSERT(ret == 0, "failed to add monitor to wm.\n");
    }
    XFree(info);

    wm.focused_monitor = (WMMonitor*) wm.monitors_list.next;

    return 0;

fail:
    return -1;
}

int initialize_wm(){
    int ret;
    wm.to_exit = 0;

    XSetErrorHandler(x_on_error);

    create_colors();

    ret = register_key_events();
    ASSERT(ret == 0, "failed to register to key events\n");

    XSelectInput(   wm.display, 
                    wm.root_window,
                    SubstructureRedirectMask    | 
                    SubstructureNotifyMask      |
                    PropertyChangeMask          |
                    StructureNotifyMask);

    // Initialize the monitors
    monitors_setup();

    List* curr = &wm.monitors_list;
    while(curr->next){
        curr = curr->next;
        WMMonitor* monitor = (WMMonitor*) curr;

        WMWorkspace* workspace = workspace_create(  next_workspace_number(),
                                                    monitor->x,
                                                    monitor->y,
                                                    monitor->width,
                                                    monitor->height);
        ASSERT(workspace, "failed to create a workspace.\n");

        ret = monitor_add_workspace(monitor, workspace);
        ASSERT(ret == 0, "failed to add workspace to monitor.\n");
    }
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
    int ret;

    destroy_colors();
    unregister_key_event();
    XCloseDisplay(wm.display);

    wm.display = NULL;
    
    // clean add monitors, workspaces and windows before exit.
    while (wm.monitors_list.next){
        WMMonitor* monitor = (WMMonitor*)wm.monitors_list.next;

        ret = list_del(&monitor->list);
        ASSERT(ret == 0, "failed to delete monitor on cleanup.\n");

        while (monitor->workspaces_list.next){
            WMWorkspace* workspace = (WMWorkspace*)monitor->workspaces_list.next;

            ret = list_del(&workspace->list);
            ASSERT(ret == 0, "failed to delete workspace on cleanup.\n");

            while (workspace->windows_list.next){
                WMWindow* window = (WMWindow*)workspace->windows_list.next;

                ret = list_del(&window->list);
                ASSERT(ret == 0, "failed to delete window on cleanup.\n");

                window_destroy(window);
            }
            workspace_destroy(workspace);
        }
        monitor_destroy(monitor);
    }
    return 0;

fail:
    return -1;
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
    int ret;
    ret = start();
    ASSERT(ret == 0, "failed to start wm.\n");

    LOG("wm has started!\n");

    main_event_loop();

    LOG("wm has finished!\n");
    return end();

fail:
    return ret;
}
