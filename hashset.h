#ifndef SAT_HASHSET_H
#define SAT_HASHSET_H

#include <stdbool.h>

#include "arraymap.h"

// Functions ------------------------------------------------------------------

typedef unsigned (*hashset_hash_func)(void* input);

typedef bool (*hashset_equal_func)(void* a, void* b);

typedef void (*hashset_free_func)(void* input, void* aux);

// Structs --------------------------------------------------------------------

typedef struct hashset {
    hashset_hash_func hash_func;
    hashset_equal_func equal_func;
    arraymap_t* buckets; // arraymap<unsigned, linkedlist_t*<void*>>
} hashset_t;

// Configuration --------------------------------------------------------------

static const unsigned HASHSET_NUM_BUCKETS = 256;

// Public interface -----------------------------------------------------------

hashset_t* hashset_create(hashset_hash_func hash_function, hashset_equal_func equal_function);

void hashset_add(hashset_t* this, void* elem);

bool hashset_contains(hashset_t* this, void* elem);

void hashset_destroy(hashset_t* this, hashset_free_func free_func, void* aux);

#endif
