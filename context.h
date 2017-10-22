#ifndef SAT_CONTEXT_H
#define SAT_CONTEXT_H

#include "clause.h"
#include "arraymap.h"

typedef struct context {
    arrayList_t* formula; // ArrayList of clauses
    arrayList_t* conflicts; // ArrayList of clauses
    arraymap_t* variables;
} context_t;

context_t* context_create();

void context_set_formula(context_t* this, arrayList_t* formula);

void context_set_variables(context_t* this, arraymap_t* variables);

void context_add_conflict_clause(context_t* this, clause_t* clause);

void context_print_formula(context_t* this);

void context_destroy(context_t* this);

#endif