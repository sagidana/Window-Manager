#include "tree.h"

void tree_init_tree(Tree* element){
    element->parent = NULL;
    element->children = NULL;
    element->next = element;
    element->prev = element;
}

int tree_del(Tree* element){
    ASSERT(element->parent, "no parent to delete it from");
    
    // the only children of the parent
    if (element->next == NULL || element->next == element){
        // remove its reference from the parent 
        // is the only thing that needs to be done
        element->parent->children = NULL; 

        // reset its fields
        element->parent = NULL;
        element->next = element;
        element->prev = element;

        return 0;
    }

    // remove from children list
    Tree* prev = element->prev;
    Tree* next = element->next;

    if (prev){
        prev->next = next;
    }
    if (next){
        next->prev = prev;
    }
    
    // make sure the parent's reference is not the node we just removing
    element->parent->children = prev; // randomly chose the prev

    // reset its fields
    element->parent = NULL;
    element->next = element;
    element->prev = element;
    return 0;

fail:
    return -1;
}

int tree_add(Tree* parent, Tree* element){
    ASSERT(parent, "parent can't be null.");

    // in case there are other children, link them to the current one.
    if (parent->children){
        Tree* next = parent->children;
        Tree* prev = next->prev;

        element->next = next;
        element->prev = prev;

        next->prev = element;
        if (prev){
            prev->next = element;
        }
    }else{
        element->next = element;
        element->prev = element;
    }

    // doesn't matter if there are other children, 
    // they will be referenced using the next and prev of the current element
    parent->children = element;
    element->parent = parent;
    return 0;

fail:
    return -1;
}

int tree_num_of_direct_children(Tree* element){
    int num_of_children = 0;
    if (element->children == NULL){
        return 0;
    }

    Tree* head = element->children;
    Tree* next = head;

    do{
        num_of_children++;

        next = next->next;
    }while(next != head);

    return num_of_children;
}

// example to iterate thought the entire tree.
int tree_foreach(Tree* node){
    // -----------------------------------
    // logic for each node goes here <==
    // -----------------------------------

    if (node->children == NULL){
        return 0;
    }

    Tree* head = node->children;
    Tree* next = head;

    do{
        tree_foreach(next);

        next = next->next;
    }while(next != head);

    return 0;
}
