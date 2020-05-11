#ifndef KEYS_H
#define KEYS_H

#include <X11/keysym.h> 

typedef struct{
    void* ptr;
}Args;

typedef struct{
    unsigned int mod;
    KeySym keysym;
    void (*func) (Args args);
    Args args;
} Key;

// the mod key is winkey
#define MODKEY Mod4Mask

static const char* termcmd[] = {"st", NULL};

static Key keys[] = {
    {MODKEY,    XK_Return,      NULL,      {.ptr = termcmd } }
};

#endif
