//
// Basic arraylist implementation
//

#include <stdlib.h>
#include <stdio.h>
#include "arraylist.h"

#define BASE_MAX_LEN 2

arrayList_t* createArrayList() {
    arrayList_t* ret = malloc(sizeof(arrayList_t));
    ret->array = calloc(BASE_MAX_LEN, sizeof(literal_t));
    ret->currIndex = 0;
    ret->maxLen = BASE_MAX_LEN;
    return ret;
}

void insert(arrayList_t* list, literal_t value) {
    if(list->currIndex == list->maxLen) {
        list->maxLen *= 2;
        list->array = realloc(list->array, list->maxLen * sizeof(literal_t));
    }
    list->array[list->currIndex++] = value;
}

void printAll(arrayList_t* list) {
    for(size_t i = 0; i < list->currIndex; i++) {
        printf("%d ", list->array[i]);
    }
    printf("\n");
}

void destroy(arrayList_t* list) {
    free(list->array);
}