#ifndef COMMON_H
#define COMMON_H

#include <X11/Xlib.h> 
#include <stdlib.h>
#include <stdio.h>

#define FALSE (0)
#define TRUE (!(FALSE))

#define ASSERT(expr, ...) if(!expr) {printf(__VA_ARGS__); goto fail;}

#define LOG_FILE_PATH "/home/s/wm.log"

void wm_log(char* msg);

#define LOG(...) do{                \
    char buff[256];                 \
    sprintf(buff, __VA_ARGS__);     \
    wm_log(buff);                   \
}while(0)

#define LENGTH(x) (sizeof(x) / sizeof(x[0]))

#endif
