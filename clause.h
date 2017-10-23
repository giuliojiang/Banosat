// Clause definition

#ifndef SAT_CLAUSE_H
#define SAT_CLAUSE_H

#include "arraylist.h"
#include "macros.h"

typedef struct clause {
    arrayList_t* literals;
    linked_list_node_t* participating_unsat;
    linked_list_node_t* participating_conflicting;
} clause_t;


static inline void clause_destroy(void* elem, void* UNUSED(aux)) {
    clause_t* clause = (clause_t*) elem;
    arraylist_destroy(clause->literals, &arraylist_destroy_free, NULL);
    free(elem);
}
#endif //SAT_CLAUSE_H
