//
// Created by dmarino on 22/10/17.
//

#ifndef SAT_VARIABLE_H
#define SAT_VARIABLE_H

#include "literal.h"
#include "arraylist.h"
#include "arraymap.h"
#include "clause.h"

// For laziness of finding the right place to put this
typedef int bool;
#define true 1
#define false 0

typedef struct variable {
    arrayList_t* participatingClauses; // arraylist of clause_t
    int currentAssignment;
    // More to come
} variable_t;

variable_t* variable_create();
void variable_add_value_into_map(arraymap_t* map, literal_t lit, clause_t* clause);
void variable_destroy(variable_t* variable);
void variable_set_value(variable_t* variable, bool new_value);
#endif //SAT_VARIABLE_H
