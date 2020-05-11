#ifndef LIST_H
#define LIST_H

#include "common.h"


typedef struct list_t{
    struct list_t* next;
    struct list_t* prev;
}List;

int list_del(List* element);

int list_add(List* list, List* element);

int list_add_tail(List* list, List* element);

#endif
