#ifndef SAT_CONTEXT_H
#define SAT_CONTEXT_H

#include "clause.h"

typedef struct context {
    arrayList_t* formula; // ArrayList of clauses
    arrayList_t* conflicts; // ArrayList of clauses
} context_t;

context_t* context_create();

void context_set_formula(context_t* this, arrayList_t* formula);

void context_add_conflict_clause(context_t* this, clause_t* clause);

void context_print_formula(context_t* this);

void context_destroy(context_t* this);

#endif