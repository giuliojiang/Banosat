//
// Basic arraylist implementation
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "arraylist.h"

#define BASE_MAX_LEN 16

arrayList_t* arraylist_create() {
    arrayList_t* ret = malloc(sizeof(arrayList_t));
    assert(ret);
    ret->array = calloc(BASE_MAX_LEN, sizeof(void*));
    assert(ret->array);
    ret->currIndex = 0;
    ret->maxLen = BASE_MAX_LEN;
    return ret;
}

void arraylist_insert(arrayList_t* list, void* value) {
    if(list->currIndex == list->maxLen) {
        list->maxLen *= 2;
        list->array = realloc(list->array, list->maxLen * sizeof(void*));
    }

    list->array[list->currIndex++] = value;
}

void arraylist_print_all(arrayList_t* list) {
    if (ENABLE_DEBUG) {
        for(size_t i = 0; i < list->currIndex; i++) {
            void* ptr = arraylist_get(list, i);
            LOG_DEBUG("%p -> %d ", ptr, *((literal_t*)ptr));
        }
        LOG_DEBUG("\n");
    }
}

void arraylist_set(arrayList_t* list, size_t index, void* value) {
    BOUND_CHECK(list->maxLen, index);
    list->array[index] = value;
}

void* arraylist_get(arrayList_t* list, size_t index) {
    BOUND_CHECK(list->currIndex, index);
    return list->array[index];
}


void arraylist_destroy(arrayList_t* list, voidp_consumer destroyer, void* aux) {
    // Destroy each element using the provided destroyer
    if (destroyer) {
        arraylist_foreach(list, destroyer, aux);
    }
    // Free array and list itself
    free(list->array);
    free(list);
}

size_t arraylist_size(arrayList_t* list) {
    return list->currIndex;
}

void arraylist_foreach(arrayList_t* list, voidp_consumer function, void* aux) {
    for(size_t i = 0; i < list->currIndex; i++) {
        function(list->array[i], aux);
    }
}
