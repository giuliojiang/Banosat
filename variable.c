
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
