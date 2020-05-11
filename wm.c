#include "wm.h"

#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <errno.h>

#include "common.h"


// -----------------------------------------------------
// helpers functions
// -----------------------------------------------------
Key* get_key_by_code(Display* display, int key_code){
    int i;
    for (i = 0; i < LENGTH(wm_keys); i++){
        if (key_code == XKeysymToKeycode(display, wm_keys[i].keysym)){
            return &wm_keys[i];
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
    // currently just forward the request

    XMapRequestEvent* event = &e->xmaprequest;

    XMapWindow(wm.display, event->window);

    XSetInputFocus( wm.display, 
                    event->window, 
                    RevertToPointerRoot, 
                    CurrentTime);

    XSync(wm.display, FALSE);
}

void on_key_press(XEvent* e){
    XKeyEvent* event = &e->xkey;
    Key* current_key = get_key_by_code(wm.display, event->keycode);
    if (current_key == NULL){
        return;
    }

    LOG("Key pressed: %d\n", event->keycode);

    if (current_key->func){
        current_key->func(&current_key->args);
    }
}

static void (*event_handlers[LASTEvent]) (XEvent *) = {
	[ButtonPress]       = on_default,
	[ClientMessage]     = on_default,
	[ConfigureRequest]  = on_configure_request,
	[ConfigureNotify]   = on_default,
	[DestroyNotify]     = on_default,
	[EnterNotify]       = on_default,
	[Expose]            = on_default,
	[FocusIn]           = on_default,
	[KeyPress]          = on_key_press,
	[MappingNotify]     = on_default,
	[MapRequest]        = on_map_request,
	[MotionNotify]      = on_default,
	[PropertyNotify]    = on_default,
	[UnmapNotify]       = on_default
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

int register_key_events(){
    int i;
    int ret;
    XUngrabKey(wm.display, AnyKey, AnyModifier, wm.root_window);

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
}


int start(){
    int ret;
    wm.display = XOpenDisplay(NULL); // create connection
    ASSERT(wm.display, "failed to open display\n");

    wm.root_window = DefaultRootWindow(wm.display);
    wm.to_exit = 0;

    detect_other_wm();

    XSetErrorHandler(x_on_error);

    ret = register_key_events();
    ASSERT(ret == 0, "failed to register to key events\n");

    XSync(wm.display, FALSE);
    return 0;
}

int end(){
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
