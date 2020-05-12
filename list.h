#ifndef LIST_H
#define LIST_H

#include "common.h"


typedef struct list_t{
    struct list_t* next;
    struct list_t* prev;
}List;

// assume the list is the head.
#define LIST_EMPTY(list) ((list)->next == NULL)

int list_del(List* element);

int list_add(List* list, List* element);

int list_add_tail(List* list, List* element);

#endif
