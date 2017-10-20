#ifndef SAT_ARRAYMAP_H
#define SAT_ARRAYMAP_H

// Data structure -------------------------------------------------------------

#include <stdlib.h>
#include "arraylist.h"

typedef struct arraymap {
    arrayList_t* arraylist;
} arraymap_t;

typedef void (*arraymap_pair_consumer)(size_t key, void* value, void* aux);

// Public methods -------------------------------------------------------------

arraymap_t* arraymap_create();

void arraymap_put(arraymap_t* this, size_t key, void* value);

void* arraymap_get(arraymap_t* this, size_t key);

void arraymap_foreach_pair(arraymap_t* this, arraymap_pair_consumer consumer, void* aux);

void arraymap_destroy(arraymap_t* this, arraymap_pair_consumer destroyer, void* aux);

#endif //SAT_ARRAYMAP_H
