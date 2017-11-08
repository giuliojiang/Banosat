// Clause definition

#ifndef SAT_CLAUSE_H
#define SAT_CLAUSE_H

#include "arraylist.h"
#include "macros.h"
#include "hashset.h"

typedef struct clause {
    arrayList_t* literals; // elements are literal_t*
    linkedlist_node_t* participating_unsat; // linkedlist_node<clause_t*>
    linkedlist_node_t* participating_false_clauses; // linkedlist_node<clause_t*>
    linkedlist_t* variables_removal_nodes; // linkedlist<linkedlist_node<clause_t*>>
    literal_t* an_unassigned_literal; // An unassigned literal. Valid if the unassigned_count is > 0
                                      // There are no guarantees about what is the unassigned literal
    linkedlist_node_t* participating_unit_clause; // linkedlist_node<clause_t*>
} clause_t;


static inline void clause_destroy(void* elem, void* UNUSED(aux)) {
    clause_t* clause = (clause_t*) elem;
    arraylist_destroy(clause->literals, &arraylist_destroy_free, NULL);
    linkedlist_destroy(clause->variables_removal_nodes, NULL, NULL);
    free(clause);
}

clause_t* clause_create();

void clause_print(clause_t* this);

bool clause_add_literal(clause_t* this, int literal_value, hashset_t* seen_literals, bool* contains_negation);

#endif //SAT_CLAUSE_H
