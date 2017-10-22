//
// Created by dmarino on 22/10/17.
//

#ifndef SAT_VARIABLE_H
#define SAT_VARIABLE_H

#include "literal.h"
#include "arraylist.h"
#include "arraymap.h"
#include "clause.h"

typedef struct variable {
    arrayList_t* participatingClauses;
    // More to come
} variable_t;

variable_t* variable_create();
void variable_add_value_into_map(arraymap_t* map, literal_t lit, clause_t* clause);
void variable_destroy(variable_t* variable);
#endif //SAT_VARIABLE_H
