#ifndef COMMON_H
#define COMMON_H

#define FALSE (0)
#define TRUE (!(FALSE))

#define ASSERT(expr, msg) if(!expr) {printf(msg); return -1;}

#define LOG_FILE_PATH "/home/s/wm.log"

void wm_log(char* msg);

#define LOG(...) do{                \
    char buff[256];                 \
    sprintf(buff, __VA_ARGS__);     \
    wm_log(buff);                   \
}while(0)

#endif
