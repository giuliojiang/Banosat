// Clause definition

#ifndef SAT_CLAUSE_H
#define SAT_CLAUSE_H

#include "arraylist.h"
#include "macros.h"

typedef struct clause {
    arrayList_t* literals; // elements are literal_t*
    linkedlist_node_t* participating_unsat; // linkedlist_node<clause_t*>
    linkedlist_node_t* participating_false_clauses; // linkedlist_node<clause_t*>
} clause_t;


static inline void clause_destroy(void* elem, void* UNUSED(aux)) {
    clause_t* clause = (clause_t*) elem;
    arraylist_destroy(clause->literals, &arraylist_destroy_free, NULL);
    free(clause);
}

void clause_print(clause_t* this);

#endif //SAT_CLAUSE_H
