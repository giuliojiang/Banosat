#ifndef SAT_ARRAYMAP_H
#define SAT_ARRAYMAP_H

// Data structure -------------------------------------------------------------

#include <stdlib.h>
#include "arraylist.h"

typedef struct arraymap {
    arrayList_t* arraylist;
} arraymap_t;

typedef struct arraymap_pair {
    size_t k;
    void* v;
} arraymap_pair_t;

typedef void (*arraymap_pair_consumer)(size_t key, void* value, void* aux);

// Public methods -------------------------------------------------------------

arraymap_t* arraymap_create();

void arraymap_put(arraymap_t* this, size_t key, void* value);

void* arraymap_get(arraymap_t* this, size_t key);

void arraymap_foreach_pair(arraymap_t* this, arraymap_pair_consumer consumer, void* aux);

void arraymap_destroy(arraymap_t* this, arraymap_pair_consumer destroyer, void* aux);

arraymap_pair_t arraymap_find_first_entry(arraymap_t* this);

arraymap_pair_t arraymap_find_next_entry(arraymap_t* this, size_t curr_index);

void arraymap_sort(arraymap_t* this, sorter sorter);
// For loop skeleton ----------------------------------------------------------

/*
for (size_t i = 0; i < arraylist_size(ARRAYMAP->arraylist); i++) {
    void* value = arraylist_get(ARRAYMAP->arraylist, i);
    if (value) {
        do(value);
    }
}
*/

#endif //SAT_ARRAYMAP_H
