//
// Basic arraylist implementation
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "arraylist.h"

#define BASE_MAX_LEN 2
#define BOUND_CHECK(len, i) assert(i < len)

arrayList_t* createArrayList() {
    arrayList_t* ret = malloc(sizeof(arrayList_t));
    ret->array = calloc(BASE_MAX_LEN, sizeof(void*));
    ret->currIndex = 0;
    ret->maxLen = BASE_MAX_LEN;
    return ret;
}

void insert(arrayList_t* list, void* value) {
    if(list->currIndex == list->maxLen) {
        list->maxLen *= 2;
        list->array = realloc(list->array, list->maxLen * sizeof(void*));
    }
    list->array[list->currIndex++] = value;
}

void printAll(arrayList_t* list) {
    for(size_t i = 0; i < list->currIndex; i++) {
        void* ptr = get(list, i);
        printf("%p -> %d ", ptr, *((literal_t*)ptr));
    }
    printf("\n");
}

void set(arrayList_t* list, size_t index, void* value) {
    BOUND_CHECK(list->maxLen, index);
    list->array[index] = value;
}

void* get(arrayList_t* list, size_t index) {
    BOUND_CHECK(list->currIndex, index);
    return list->array[index];
}

void destroy(arrayList_t* list) {
    // Good candidate for use after free!
    for(size_t i = 0; i < list->currIndex; i++) {
        free(list->array[i]);
    }
    free(list->array);
}