// compile:
// gcc tree_tests.c -o tree_tests ../tree.o ../commmon.o

#include "../tree.h"
#include "../common.h"

typedef struct {
    Tree tree;
    int id;
}Node;

int tree_foreach(Tree* node){
    LOG("node id: %d\n", ((Node*) node)->id);

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


int main(){

    Node root = { 0 };
    root.id = 0;

    Node node_1 = { 0 };
    node_1.id = 1;

    Node node_2 = { 0 };
    node_2.id = 2;

    Node node_3 = { 0 };
    node_3.id = 3;

    ASSERT(tree_add(&root.tree, &node_1.tree) == 0, "failed to add node_1 to root\n");
    ASSERT(tree_add(&node_1.tree, &node_2.tree) == 0, "failed to add node_2 to node_1\n");
    ASSERT(tree_add(&node_1.tree, &node_3.tree) == 0, "failed to add node_3 to node_2\n");
    tree_foreach(&root.tree);
    ASSERT(tree_del(&node_3.tree) == 0, "failed to add node_3 to node_2\n");
    tree_foreach(&root.tree);
    ASSERT(tree_del(&node_2.tree) == 0, "failed to add node_3 to node_2\n");
    tree_foreach(&root.tree);

    return 0;

fail:
    return -1;
}

