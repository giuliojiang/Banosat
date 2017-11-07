//
// Created by dmarino on 22/10/17.
//

#ifndef SAT_VARIABLE_H
#define SAT_VARIABLE_H

#include "literal.h"
#include "arraylist.h"
#include "arraymap.h"
#include "clause.h"

#include <stdbool.h>


typedef struct variable {
    // Participating Clauses should be constructed at the beginning of the formula
    // and become read-only for the rest of the execution
    linkedlist_t* participatingClauses; // linkedlist<clause_t*>
    int currentAssignment;
    int unsatTrueLiteralCount; // number of occurrences in true literals in unsat clauses
    int unsatNegatedLiteralCount; // number of occurrences in negated literals in unsat clauses
    linkedlist_t* deduced_from; // linkedlist<size_t> a variable index.
                                // The parent link to the primary variable assignment
                                // that allowed this variable's deduction to be worked by BCP or PLP
} variable_t;

variable_t* variable_create();
void variable_destroy(variable_t* variable);
void variable_insert_clause(variable_t* this, clause_t* clause);
void variable_set_value(variable_t* variable, bool new_value);
void variable_set_raw_value(variable_t* variable, int new_value);
void variable_set_unsat_true_literal_count(variable_t* variable, int new_count);
void variable_set_unsat_negated_literal_count(variable_t* variable, int new_count);
int variable_get_purity(variable_t* variable);
#endif //SAT_VARIABLE_H
