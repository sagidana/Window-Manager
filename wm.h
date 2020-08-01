#ifndef WM_H
#define WM_H

#include <X11/Xlib.h> 
#include <X11/keysym.h> 
#include <X11/XF86keysym.h> 

#include "common.h"
#include "workspace.h"
#include "monitor.h"
#include "window.h"
#include "list.h"


typedef struct{
    Display* display;
    Window root_window;
    int to_exit;

    List monitors_list;
    WMMonitor* focused_monitor;

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
    {MODKEY,                XK_Return,      spawn,            {.ptr = termcmd,  .i = -1} },
    {MODKEY,                XK_D,           spawn,            {.ptr = dmenucmd, .i = -1} },

    // exit wm
    {MODKEY | ShiftMask,    XK_E,           to_exit,          {.ptr = NULL,     .i = -1 } },

    {MODKEY | ShiftMask,    XK_Q,           kill,             {.ptr = NULL,     .i = -1} },

    // -------------------------------------------------------------------------------------
    // Arrange bindings
    // -------------------------------------------------------------------------------------
    // toggle vertical mode
    {MODKEY,                XK_V,           arrange,          {.ptr = NULL,     .i = 0} },
    {MODKEY | ShiftMask,    XK_V,           arrange,          {.ptr = NULL,     .i = 1} },
    // toggle fullscreen mode
    {MODKEY,                XK_F,           arrange,          {.ptr = NULL,     .i = 2} },

    // to move
    {MODKEY | ShiftMask,    XK_H,           arrange,          {.ptr = NULL,     .i = 3} },
    {MODKEY | ShiftMask,    XK_J,           arrange,          {.ptr = NULL,     .i = 4} },
    {MODKEY | ShiftMask,    XK_K,           arrange,          {.ptr = NULL,     .i = 5} },
    {MODKEY | ShiftMask,    XK_L,           arrange,          {.ptr = NULL,     .i = 6} },

    // for focus
    {MODKEY,                XK_H,           arrange,          {.ptr = NULL,     .i = 7} },
    {MODKEY,                XK_J,           arrange,          {.ptr = NULL,     .i = 8} },
    {MODKEY,                XK_K,           arrange,          {.ptr = NULL,     .i = 9} },
    {MODKEY,                XK_L,           arrange,          {.ptr = NULL,     .i = 10} },

    // // for alignment (for default arrange only)
    // {MODKEY | Mod1Mask,     XK_H,           arrange,          {.ptr = NULL,     .i = 11} },
    // {MODKEY | Mod1Mask,     XK_J,           arrange,          {.ptr = NULL,     .i = 12} },
    // {MODKEY | Mod1Mask,     XK_K,           arrange,          {.ptr = NULL,     .i = 13} },
    // {MODKEY | Mod1Mask,     XK_L,           arrange,          {.ptr = NULL,     .i = 14} },

    // to resize
    {MODKEY | Mod1Mask,  XK_H,           arrange,          {.ptr = NULL,     .i = 15} },
    {MODKEY | Mod1Mask,  XK_J,           arrange,          {.ptr = NULL,     .i = 16} },
    {MODKEY | Mod1Mask,  XK_K,           arrange,          {.ptr = NULL,     .i = 17} },
    {MODKEY | Mod1Mask,  XK_L,           arrange,          {.ptr = NULL,     .i = 18} },
    // -------------------------------------------------------------------------------------

    {MODKEY,                XK_1,           switch_workspace, {.ptr = NULL,     .i = 1 } },
    {MODKEY,                XK_2,           switch_workspace, {.ptr = NULL,     .i = 2 } },
    {MODKEY,                XK_3,           switch_workspace, {.ptr = NULL,     .i = 3 } },
    {MODKEY,                XK_4,           switch_workspace, {.ptr = NULL,     .i = 4 } },
    {MODKEY,                XK_5,           switch_workspace, {.ptr = NULL,     .i = 5 } },
    {MODKEY,                XK_6,           switch_workspace, {.ptr = NULL,     .i = 6 } },
    {MODKEY,                XK_7,           switch_workspace, {.ptr = NULL,     .i = 7 } },
    {MODKEY,                XK_8,           switch_workspace, {.ptr = NULL,     .i = 8 } },
    {MODKEY,                XK_9,           switch_workspace, {.ptr = NULL,     .i = 9 } },
    {MODKEY,                XK_0,           switch_workspace, {.ptr = NULL,     .i = 10 } },
    {MODKEY,                XK_KP_End,      switch_workspace, {.ptr = NULL,     .i = 11 } },
    {MODKEY,                XK_KP_Down,     switch_workspace, {.ptr = NULL,     .i = 12 } },
    {MODKEY,                XK_KP_Next,     switch_workspace, {.ptr = NULL,     .i = 13 } },
    {MODKEY,                XK_KP_Left,     switch_workspace, {.ptr = NULL,     .i = 14 } },
    {MODKEY,                XK_KP_Begin,    switch_workspace, {.ptr = NULL,     .i = 15 } },
    {MODKEY,                XK_KP_Right,    switch_workspace, {.ptr = NULL,     .i = 16 } },
    {MODKEY,                XK_KP_Home,     switch_workspace, {.ptr = NULL,     .i = 17 } },
    {MODKEY,                XK_KP_Up,       switch_workspace, {.ptr = NULL,     .i = 18 } },
    {MODKEY,                XK_KP_Prior,    switch_workspace, {.ptr = NULL,     .i = 19 } },
    {MODKEY,                XK_KP_Insert,   switch_workspace, {.ptr = NULL,     .i = 20 } }
};

// ------------------------------------------------------------------

// ------------------------------------------------------------------
// Colors configurations
// ------------------------------------------------------------------

char focused_window_color[] = "#00FFFF";
char normal_window_color[]  = "#000000";

int main();

#endif

