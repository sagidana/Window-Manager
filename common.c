#include <stdio.h>
#include <string.h>

#include "common.h"


void wm_log(char* msg){
    FILE* f = fopen(LOG_FILE_PATH, "a+");
    if (!f) return;

    fwrite(msg, 1, strlen(msg), f);

    fclose(f);
}

