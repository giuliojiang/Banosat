#include "../linkedlist.h"

#include <stdlib.h>
#include <stdio.h>

static void linkedlist_print_node(linkedlist_node_t* node, void* aux) {
    printf("%ld\n", (long) node->value);
}

int main(int argc, char** argv) {
    // Create a linkedlist
    linkedlist_t* the_list = linkedlist_create();
    // Add a number 1
    linkedlist_node_t* node_1 = linkedlist_add_last(the_list, (void*) 1);
    // Print
    printf("List is \n");
    linkedlist_foreach(the_list, &linkedlist_print_node, NULL);
    // Add a number 2
    linkedlist_node_t* node_2 = linkedlist_add_last(the_list, (void*) 2);
    // Print
    printf("List is \n");
    linkedlist_foreach(the_list, &linkedlist_print_node, NULL);
    // Remove node 1
    void* removed_value = linkedlist_remove_node(the_list, node_1);
    printf("Removed: %ld \n", (long) removed_value);
    // Print
    printf("List is \n");
    linkedlist_foreach(the_list, &linkedlist_print_node, NULL);
    // Free everything
    linkedlist_destroy(the_list, NULL, NULL);
}