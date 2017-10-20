//
// Basic arraylist implementation
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "arraylist.h"

#define BASE_MAX_LEN 16
#define BOUND_CHECK(len, i) assert(i < len)

arrayList_t* arraylist_create() {
    arrayList_t* ret = malloc(sizeof(arrayList_t));
    assert(ret);
    ret->array = calloc(BASE_MAX_LEN, sizeof(void*));
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
    for(size_t i = 0; i < list->currIndex; i++) {
        void* ptr = arraylist_get(list, i);
        printf("%p -> %d ", ptr, *((literal_t*)ptr));
    }
    printf("\n");
}

void arraylist_set(arrayList_t* list, size_t index, void* value) {
    BOUND_CHECK(list->maxLen, index);
    list->array[index] = value;
}

void* arraylist_get(arrayList_t* list, size_t index) {
    BOUND_CHECK(list->currIndex, index);
    return list->array[index];
}

void arraylist_destroy(arrayList_t* list, voidF_t destroyer) {
    // Destroy each element using the provided destroyer
    if (destroyer) {
        arraylist_foreach(list, destroyer);
    }
    // NOTE: Good candidate for use after free!
    // Call free on each element
    arraylist_foreach(list, &free);
    // Free array and list itself
    free(list->array);
    free(list);
}

size_t arraylist_size(arrayList_t* list) {
    return list->currIndex;
}

void arraylist_foreach(arrayList_t* list, voidF_t function) {
    for(size_t i = 0; i < list->currIndex; i++) {
        function(list->array[i]);
    }
}