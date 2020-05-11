#ifndef LIST_H
#define LIST_H

#include "common.h"


typedef struct list_t{
    struct list_t* next;
    struct list_t* prev;
}List;

int list_del(List* element){
    List* prev = element->prev;
    List* next = element->next;

    if (prev){
        prev->next = next;
    }
    if (next){
        next->prev = prev;
    }

    return 0;
}

int list_add(List* list, List* element){
    List* next = list->next;

    list->next = element;
    element->prev = list;

    element->next = next;
    if (next){
        next->prev = element;
    }

    return 0;
}

int list_add_tail(List* list, List* element){
    int ret;

    while (list->next){
        list = list->next;
    }
    ret = list_add(list, element);

    ASSERT(ret == 0, "failed to add to list\n");
    return 0;

fail:
    return -1;
}

#endif
