#include <stdlib.h>

#include "linkedlist.h"
#include "macros.h"
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
    linkedlist_node_t* current = this->head->next;
    linkedlist_node_t* next = NULL;
    while (current != this->tail) {
        next = current->next;
        consumer(current, aux);
        current = next;
    }

    // This version causes undefined behaviour when using the consumer to
    // free the current node, as the next iteration is going to
    // use current->next to load the next node, but current has already
    // been freed
    /*
    for (linkedlist_node_t* current = this->head->next; current != this->tail; current = current->next) {
        consumer(current, aux);
    }
     */
}

// Destroy linkedlist ---------------------------------------------------------

static void linkedlist_destroy_node_destroyer(linkedlist_node_t* node, void* UNUSED(aux)) {
    linkedlist_node_destroy(node);
}

void linkedlist_destroy(linkedlist_t* this, linkedlist_node_consumer destroy_function, void* aux) {
    // If a destroy function is provided, destroy each element
    if (destroy_function) {
        linkedlist_foreach(this, destroy_function, aux);
    }
    // Destroy each of the nodes
    linkedlist_foreach(this, linkedlist_destroy_node_destroyer, NULL);
    // Destroy head and tail
    linkedlist_node_destroy(this->head);
    linkedlist_node_destroy(this->tail);
    // Free the linkedlist
    free(this);
}
