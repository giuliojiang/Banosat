#ifndef SAT_CONTEXT_H
#define SAT_CONTEXT_H

#include "clause.h"
#include "arraymap.h"
#include "variable.h"

typedef struct context {
    arrayList_t* formula; // ArrayList of clause_t
    arrayList_t* conflicts; // ArrayList of clause_t
    arraymap_t* variables; // Map from unsigned -> variable_t
    linkedlist_t* unsat; // Unsatisfied clauses (clause_t)
    linkedlist_t* false_clauses; // List of false clauses in a formula
    linkedlist_t* assignment_history; // linkedlist<assignment_level_t*> Assignment history
} context_t;

context_t* context_create();

void context_set_formula(context_t* this, arrayList_t* formula);

void context_set_variables(context_t* this, arraymap_t* variables);

void context_add_conflict_clause(context_t* this, clause_t* clause);

void context_print_formula(context_t* this);

void context_destroy(context_t* this);

void context_assign_variable_value(context_t* this, size_t variable_index, bool new_value);

void context_unassign_variable(context_t* this, size_t variable_index);

int context_run_bcp(context_t* this);

void context_print_current_state(context_t* this);

int context_evaluate_formula(context_t* this);

void context_apply_new_decision_level(context_t* this, size_t variable_index, bool new_value);

assignment_level_t* context_get_last_assignment_level(context_t* this);

#endif
