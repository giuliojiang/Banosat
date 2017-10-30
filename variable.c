
#include <stdlib.h>
#include "variable.h"

variable_t* variable_create() {
    variable_t* ret = (variable_t*) malloc(sizeof(variable_t));
    ret->participatingClauses = NULL;
    ret->currentAssignment = 0;
    return ret;
}

void variable_insert_clause(variable_t* this, clause_t* clause) {
    if(!this->participatingClauses) {
        this->participatingClauses = arraylist_create();
    }
    arraylist_insert(this->participatingClauses, clause);
}

void variable_add_value_into_map(arraymap_t *map, literal_t lit, clause_t* clause) {
    size_t absLit = (size_t) abs(lit);
    variable_t* var = arraymap_get(map, absLit);
    if(!var) {
        // If the arraymap doesn't have an entry yet for this variable
        // Create it
        var = variable_create();
        arraymap_put(map, absLit, var);
    }
    variable_insert_clause(var, clause);
}

// variable_destroy -----------------------------------------------------------

void variable_destroy(variable_t* variable) {
    arraylist_destroy(variable->participatingClauses, NULL, NULL);
    free(variable);
}

void variable_set_value(variable_t* variable, bool new_value) {
    variable->currentAssignment = new_value ? 1 : -1;
}

void variable_set_raw_value(variable_t* variable, int new_value) {
    variable->currentAssignment = new_value;
}

int variable_sorter(const void* elem1, const void* elem2) {
    const variable_t* variable1 = elem1;
    const variable_t* variable2 = elem2;
    LOG_DEBUG("Variable 1: %p %p\n", variable1, variable1->participatingClauses);
    LOG_DEBUG("Variable 2: %p %p\n", variable2, variable2->participatingClauses);
    if(!variable1->participatingClauses) {
        return -1; // TODO: not sure what to do with this
    }
    if(!variable2->participatingClauses) {
        return 1; // TODO: Same as above
    }
    size_t size1 = arraylist_size(variable1->participatingClauses);
    size_t size2 = arraylist_size(variable2->participatingClauses);
    return (int)size1 - (int)size2;
}