// Header for arrayLists

#ifndef SAT_ARRAYLIST_H
#define SAT_ARRAYLIST_H


#include "literal.h"

typedef struct arrayList {
    size_t currIndex;
    size_t maxLen;
    void** array;
} arrayList_t;

arrayList_t* createArrayList();

void insert(arrayList_t* list, void* value);

void printAll(arrayList_t* list);

void set(arrayList_t* list, size_t index, void* value);

void* get(arrayList_t* list, size_t index);

void destroy(arrayList_t* list);
#endif //SAT_ARRAYLIST_H
