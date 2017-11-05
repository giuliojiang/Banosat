#ifndef SAT_CONTEXT_H
#define SAT_CONTEXT_H

#include "clause.h"
#include "arraymap.h"
#include "variable.h"
#include "assignment_level.h"

typedef struct context {
    arrayList_t* formula; // arraylist<clause_t*>
    size_t* sorted_indices;
    arraymap_t* variables; // arraymap<size_t, variable_t*>
    linkedlist_t* unsat; // linkedlist<clause_t*>: unsatisfied clauses
    linkedlist_t* false_clauses; // linkedlist<clause_t*>: false clauses in the formula
    linkedlist_t* assignment_history; // linkedlist<assignment_level_t*>: assignment history
    size_t numVariables; // Number of variables
} context_t;

context_t* context_create();

void context_add_clause(context_t* this, clause_t* new_clause);

void context_finalize_variables(context_t* this, size_t numVariables);

void context_add_conflict_clause(context_t* this, clause_t* clause);

void context_print_formula(context_t* this);

void context_destroy(context_t* this);

void context_assign_variable_value(context_t* this, size_t variable_index, bool new_value);

void context_unassign_variable(context_t* this, size_t variable_index);

int context_run_bcp(context_t* this);

int context_run_plp(context_t* this);

void context_print_current_state(context_t* this);

int context_evaluate_formula(context_t* this);

void context_print_result_variables(const context_t* ctx);

void context_apply_new_decision_level(context_t* this, size_t variable_index, bool new_value);

assignment_level_t* context_get_last_assignment_level(context_t* this);

assignment_level_t* context_remove_last_assignment_level(context_t* this);

size_t context_get_first_variable_index(context_t* this);

// Returns the index of the next unassigned variable in the mapping
// Returns 0 if reaches the end of the map
size_t context_get_next_unassigned_variable(context_t* this);

unsigned context_get_clauses_count(context_t* this);

// Returns the first item from false_clauses
// Aborts execution if false_clauses is empty
clause_t* context_get_first_false_clause(context_t* this);

// Aborts execution if the current variable doesn't have a parent, or if the
// parent is unassigned
// <acc> is the data structure in which the results will be put, linkedlist<int>
void context_get_primary_assignment_of(context_t* this, unsigned query_variable_index, linkedlist_t* acc);

#endif
