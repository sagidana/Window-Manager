#ifndef TREE_H
#define TREE_H

#include "common.h"


typedef struct tree_t{
    struct tree_t* parent;
    struct tree_t* children;
    struct tree_t* next;
    struct tree_t* prev;
}Tree;

void tree_init_tree(Tree* element);
int tree_del(Tree* element);
int tree_add(Tree* parent, Tree* element);

#endif
