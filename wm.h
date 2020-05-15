#ifndef WM_H
#define WM_H

#include <X11/Xlib.h> 
#include <X11/keysym.h> 

#include "common.h"
#include "workspace.h"
#include "window.h"


typedef struct{
    Display* display;
    Window root_window;
    int to_exit;

    WMWorkspace workspaces[NUM_OF_WORKSPACES];
    int current_workspace_index;

    XColor focused_window_color;
    XColor normal_window_color;
}Manager;

static Manager wm;

typedef struct{
    void* ptr;
    int i;
}Args;

typedef struct{
    unsigned int mod;
    KeySym keysym;
    void (*func) (Args* args);
    Args args;
} Key;

// ------------------------------------------------------------------
// functions exported for key bindings
// ------------------------------------------------------------------
void spawn(Args* args);
void kill(Args* args);
void to_exit(Args* args);
void switch_workspace(Args* args);
void arrange(Args* args);

void focus_left(Args* args);
void focus_right(Args* args);
void focus_up(Args* args);
void focus_down(Args* args);

void move_left(Args* args);
void move_right(Args* args);
void move_up(Args* args);
void move_down(Args* args);
// ------------------------------------------------------------------

// ------------------------------------------------------------------
// Key bindings configuration
// ------------------------------------------------------------------

// the mod key is winkey
#define MODKEY Mod4Mask

static const char* termcmd[] = {    "termite", 
                                    NULL};

static const char* dmenucmd[] = {   "dmenu_run", 
                                    "-m", "0", 
                                    "-fn", "monospace:size=10", 
                                    "-nb", "#222222", 
                                    "-nf", "#bbbbbb",
                                    "-sb", "#005577",
                                    "-sf", "#eeeeee",
                                    NULL};

static Key wm_keys[] = {
    {MODKEY,                XK_Return,  spawn,            {.ptr = termcmd,  .i = -1} },
    {MODKEY,                XK_D,       spawn,            {.ptr = dmenucmd, .i = -1} },

    // exit wm
    {MODKEY | ShiftMask,    XK_E,       to_exit,          {.ptr = NULL,     .i = -1 } },

    {MODKEY | ShiftMask,    XK_Q,       kill,             {.ptr = NULL,     .i = -1} },

    // toggle vertical mode
    {MODKEY,                XK_V,       arrange,          {.ptr = NULL,     .i = XK_V} },
    {MODKEY | ShiftMask,    XK_V,       arrange,          {.ptr = NULL,     .i = XK_1} },
    // toggle fullscreen mode
    {MODKEY,                XK_F,       arrange,          {.ptr = NULL,     .i = XK_F} },

    // to move
    {MODKEY | ShiftMask,    XK_H,       move_left,        {.ptr = NULL,     .i = -1} },
    {MODKEY | ShiftMask,    XK_J,       move_down,        {.ptr = NULL,     .i = -1} },
    {MODKEY | ShiftMask,    XK_K,       move_up,          {.ptr = NULL,     .i = -1} },
    {MODKEY | ShiftMask,    XK_L,       move_right,       {.ptr = NULL,     .i = -1} },

    // for focus
    {MODKEY,                XK_H,       focus_left,       {.ptr = NULL,     .i = -1} },
    {MODKEY,                XK_J,       focus_down,       {.ptr = NULL,     .i = -1} },
    {MODKEY,                XK_K,       focus_up,         {.ptr = NULL,     .i = -1} },
    {MODKEY,                XK_L,       focus_right,      {.ptr = NULL,     .i = -1} },

    // for alignment
    {MODKEY | Mod1Mask,     XK_H,       arrange,          {.ptr = NULL,     .i = XK_H} },
    {MODKEY | Mod1Mask,     XK_J,       arrange,          {.ptr = NULL,     .i = XK_J} },
    {MODKEY | Mod1Mask,     XK_K,       arrange,          {.ptr = NULL,     .i = XK_K} },
    {MODKEY | Mod1Mask,     XK_L,       arrange,          {.ptr = NULL,     .i = XK_L} },

    // to resize
    {MODKEY | ControlMask,  XK_H,       arrange,          {.ptr = NULL,     .i = XK_Y} },
    {MODKEY | ControlMask,  XK_J,       arrange,          {.ptr = NULL,     .i = XK_U} },
    {MODKEY | ControlMask,  XK_K,       arrange,          {.ptr = NULL,     .i = XK_I} },
    {MODKEY | ControlMask,  XK_L,       arrange,          {.ptr = NULL,     .i = XK_O} },

    {MODKEY,                XK_0,       switch_workspace, {.ptr = NULL,     .i = 0 } },
    {MODKEY,                XK_1,       switch_workspace, {.ptr = NULL,     .i = 1 } },
    {MODKEY,                XK_2,       switch_workspace, {.ptr = NULL,     .i = 2 } },
    {MODKEY,                XK_3,       switch_workspace, {.ptr = NULL,     .i = 3 } },
    {MODKEY,                XK_4,       switch_workspace, {.ptr = NULL,     .i = 4 } },
    {MODKEY,                XK_5,       switch_workspace, {.ptr = NULL,     .i = 5 } },
    {MODKEY,                XK_6,       switch_workspace, {.ptr = NULL,     .i = 6 } },
    {MODKEY,                XK_7,       switch_workspace, {.ptr = NULL,     .i = 7 } },
    {MODKEY,                XK_8,       switch_workspace, {.ptr = NULL,     .i = 8 } },
    {MODKEY,                XK_9,       switch_workspace, {.ptr = NULL,     .i = 9 } }
};

// ------------------------------------------------------------------

// ------------------------------------------------------------------
// Colors configurations
// ------------------------------------------------------------------

char focused_window_color[] = "#00FFFF";
char normal_window_color[]  = "#000000";

int main();

#endif

