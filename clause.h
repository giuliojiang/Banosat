// Clause definition

#ifndef SAT_CLAUSE_H
#define SAT_CLAUSE_H

#include "arraylist.h"

typedef struct clause {
    arrayList_t* literals;

} clause_t;
#endif //SAT_CLAUSE_H
