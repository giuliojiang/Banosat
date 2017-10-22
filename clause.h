// Clause definition

#ifndef SAT_CLAUSE_H
#define SAT_CLAUSE_H

#include "arraylist.h"

typedef struct clause {
    arrayList_t* literals;

} clause_t;


static inline void clause_destroy(void* elem, void* aux) {
    clause_t* clause = (clause_t*) elem;
    arraylist_destroy(clause->literals, &arraylist_destroy_free, NULL);
}
#endif //SAT_CLAUSE_H
