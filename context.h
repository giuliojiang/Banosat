#ifndef SAT_CONTEXT_H
#define SAT_CONTEXT_H

#include "clause.h"
#include "arraymap.h"

typedef struct context {
    arrayList_t* formula; // ArrayList of clause_t
    arrayList_t* conflicts; // ArrayList of clause_t
    arraymap_t* variables; // Map from unsigned -> variable_t
    linkedlist_t* conflicting; // False clauses (clause_t)
    linkedlist_t* unsat; // Unsatisfied clauses (clause_t)
} context_t;

context_t* context_create();

void context_set_formula(context_t* this, arrayList_t* formula);

void context_set_variables(context_t* this, arraymap_t* variables);

void context_add_conflict_clause(context_t* this, clause_t* clause);

void context_print_formula(context_t* this);

void context_destroy(context_t* this);

void context_assign_variable_value(context_t* this, size_t variable_index, bool new_value);

int context_eval_clause(context_t* this, clause_t* clause);

void context_remove_clause_from_unsat(context_t* this, clause_t* clause);

void context_add_clause_to_conflicting(context_t* this, clause_t* clause);
#endif
