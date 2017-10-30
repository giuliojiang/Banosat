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
    arrayList_t* participatingClauses; // arraylist of clause_t
    int currentAssignment;
    // More to come
} variable_t;

variable_t* variable_create();
void variable_add_value_into_map(arraymap_t* map, literal_t lit, clause_t* clause);
void variable_destroy(variable_t* variable);
void variable_set_value(variable_t* variable, bool new_value);
void variable_set_raw_value(variable_t* variable, int new_value);
int variable_sorter(const void* elem1, const void* elem2);
#endif //SAT_VARIABLE_H
