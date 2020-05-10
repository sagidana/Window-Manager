#include "wm.h"

#include <X11/Xlib.h> 
#include <stdio.h>      
#include <stdlib.h>     // for exit()
#include <unistd.h>     

#include "common.h"


// global declerations
static Display* _display = NULL;
static Window _root_window;

void on_default(XEvent* e){}

void on_configure_request(XEvent* e){
    // currently just forward the request

    XConfigureRequestEvent* event = (XConfigureRequestEvent*)e;
    XWindowChanges changes;

    changes.x = event->x;
    changes.y = event->y;
    changes.width = event->width;
    changes.height = event->height;
    changes.border_width = event->border_width;
    changes.sibling = event->above;
    changes.stack_mode = event->detail;

    XConfigureWindow(_display, 
                    event->window, 
                    event->value_mask, 
                    &changes);
}

void on_map_request(XEvent* e){
    // currently just forward the request

    XMapRequestEvent* event = (XMapRequestEvent*) e;
    XMapWindow(_display, event->window);
}

static void (*event_handlers[LASTEvent]) (XEvent *) = {
	[ButtonPress]       = on_default,
	[ClientMessage]     = on_default,
	// [ConfigureRequest]  = on_configure_request,
	[ConfigureRequest]  = on_default,
	[ConfigureNotify]   = on_default,
	[DestroyNotify]     = on_default,
	[EnterNotify]       = on_default,
	[Expose]            = on_default,
	[FocusIn]           = on_default,
	[KeyPress]          = on_default,
	[MappingNotify]     = on_default,
	// [MapRequest]        = on_map_request,
	[MapRequest]        = on_default,
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

    XSelectInput(   _display, 
                    _root_window,
                    SubstructureRedirectMask | SubstructureNotifyMask);

    XSync(_display, FALSE);
}

int spawn(char** argv){
    if (fork() == 0){   // i am the child
        // clean xorg-server connection on child
        if (_display){
            close(((_XPrivDisplay)_display)->fd);
        }
        setsid();
        execvp(argv[0], argv);
        exit(0);
    }
    return 0;
}

int start(){
    _display = XOpenDisplay(NULL); // create connection
    ASSERT(_display, "failed to open display\n");

    _root_window = DefaultRootWindow(_display);

    detect_other_wm();

    XSetErrorHandler(x_on_error);

    XSelectInput(   _display, 
                    _root_window,
                    SubstructureRedirectMask | SubstructureNotifyMask |
                    ButtonPressMask | PointerMotionMask |
                    EnterWindowMask | LeaveWindowMask |
                    StructureNotifyMask | PropertyChangeMask);

    return 0;
}

int end(){
    XCloseDisplay(_display);
    _display = NULL;
    return 0;
}

void main_event_loop(){
    XEvent e;

    char* termcmd[] = {"st", NULL};
    spawn(termcmd);

    while(TRUE){
        // fetch next event
        XNextEvent(_display, &e);

        LOG("%d", e.type);

        // execute right handler.

        if (event_handlers[e.type]){
            event_handlers[e.type](&e);
        }
    }
}

int main(){
    if (start())
        return -1;

    LOG("wm has started!\n");

    main_event_loop();

    return end();
}
