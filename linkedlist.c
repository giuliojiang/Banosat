#include <stdlib.h>
#include "linkedlist.h"

// [private] Create/destroy nodes ---------------------------------------------

static linkedlist_node_t* linkedlist_node_create() {
    linkedlist_node_t* ret = malloc(sizeof(linkedlist_node_t));
    ret->value = NULL;
    ret->prev = NULL;
    ret->next = NULL;
    return ret;
}

static linkedlist_node_t* linkedlist_node_create_value(void* value) {
    linkedlist_node_t* ret = linkedlist_node_create();
    ret->value = value;
    return ret;
}

static inline void linkedlist_node_destroy(linkedlist_node_t* node) {
    free(node);
}

// [private] Node insertion ---------------------------------------------------

static void linkedlist_insert_after(linkedlist_t* this, linkedlist_node_t* new_node, linkedlist_node_t* prev) {
    linkedlist_node_t* next = prev->next;
    prev->next = new_node;
    new_node->prev = prev;
    new_node->next = next;
    next->prev = new_node;
    this->size++;
}

// Create linkedlist ----------------------------------------------------------

linkedlist_t* linkedlist_create() {
    linkedlist_t* ret = malloc(sizeof(linkedlist_t));
    ret->size = 0;
    ret->head = linkedlist_node_create();
    ret->tail = linkedlist_node_create();
    ret->head->next = ret->tail;
    ret->tail->prev = ret->head;
    return ret;
}

// Linkedlist operations ------------------------------------------------------

linkedlist_node_t* linkedlist_add_last(linkedlist_t* this, void* value) {
    // Add after the prev of tail
    linkedlist_node_t* new_node = linkedlist_node_create_value(value);
    linkedlist_node_t* prev = this->tail->prev;
    linkedlist_insert_after(this, new_node, prev);
    return new_node;
}

void* linkedlist_remove_node(linkedlist_t* this, linkedlist_node_t* node) {
    // NOTE possible undefined behaviour, re-link after free
    void* value = node->value;
    linkedlist_node_t* prev = node->prev;
    linkedlist_node_t* next = node->next;
    prev->next = next;
    next->prev = prev;
    linkedlist_node_destroy(node);
    this->size--;
    return value;
}

void linkedlist_foreach(linkedlist_t* this, linkedlist_node_consumer consumer, void* aux) {
    for (linkedlist_node_t* current = this->head->next; current != this->tail; current = current->next) {
        consumer(current, aux);
    }
}
