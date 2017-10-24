// Header for arrayLists

#ifndef SAT_ARRAYLIST_H
#define SAT_ARRAYLIST_H

#include <stddef.h>
#include <stdlib.h>

#include "function.h"
#include "literal.h"
#include "macros.h"

typedef struct arrayList {
    size_t currIndex;
    size_t maxLen;
    void** array;
} arrayList_t;

arrayList_t* arraylist_create();

void arraylist_insert(arrayList_t* list, void* value);

void arraylist_print_all(arrayList_t* list);

void arraylist_set(arrayList_t* list, size_t index, void* value);

void* arraylist_get(arrayList_t* list, size_t index);

void arraylist_destroy(arrayList_t* list, voidp_consumer destroyer, void* aux);

size_t arraylist_size(arrayList_t* list);

void arraylist_foreach(arrayList_t* list, voidp_consumer function, void* aux);

static inline void arraylist_destroy_free(void* item, void* UNUSED(aux)) {
    free(item);
}

#endif //SAT_ARRAYLIST_H
