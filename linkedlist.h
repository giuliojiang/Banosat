#ifndef SAT_LINKEDLIST_H
#define SAT_LINKEDLIST_H

#include <stddef.h>

// Data structures ------------------------------------------------------------

struct linkedlist_node;

typedef struct linkedlist_node {
    void* value;
    struct linkedlist_node* prev;
    struct linkedlist_node* next;
} linkedlist_node_t;

typedef struct linkedlist {
    size_t size;
    linkedlist_node_t* head;
    linkedlist_node_t* tail;
} linkedlist_t;

// Functions ------------------------------------------------------------------

typedef void (*linkedlist_node_consumer)(linkedlist_node_t*, void* aux);

// Public methods -------------------------------------------------------------

linkedlist_t* linkedlist_create();

linkedlist_node_t* linkedlist_add_last(linkedlist_t* this, void* value);

void* linkedlist_remove_node(linkedlist_t* this, linkedlist_node_t* node);

void linkedlist_foreach(linkedlist_t* this, linkedlist_node_consumer consumer, void* aux);

void linkedlist_destroy(linkedlist_t* this, linkedlist_node_consumer destroy_function, void* aux);

#endif