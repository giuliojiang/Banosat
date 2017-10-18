// Header for arrayLists

#ifndef SAT_ARRAYLIST_H
#define SAT_ARRAYLIST_H


#include "literal.h"

typedef struct arrayList {
    size_t currIndex;
    size_t maxLen;
    literal_t* array;
} arrayList_t;

arrayList_t* createArrayList();

void insert(arrayList_t* list, literal_t value);

void printAll(arrayList_t* list);

void destroy(arrayList_t* list);
#endif //SAT_ARRAYLIST_H
