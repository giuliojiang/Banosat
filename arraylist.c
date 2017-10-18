//
// Basic arraylist implementation
//

#include <stdlib.h>
#include <stdio.h>
#include "arraylist.h"

#define BASE_MAX_LEN 2

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
        list->array = realloc(list->array, list->maxLen * sizeof(literal_t));
    }
    list->array[list->currIndex++] = value;
}

void printAll(arrayList_t* list) {
    for(size_t i = 0; i < list->currIndex; i++) {
        printf("%p -> %d ", (list->array[i]), *((literal_t*)list->array[i]));
    }
    printf("\n");
}

void destroy(arrayList_t* list) {
    // Good candidate for use after free!
    for(size_t i = 0; i < list->currIndex; i++) {
        free(list->array[i]);
    }
    free(list->array);
}