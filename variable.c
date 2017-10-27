
#include <stdlib.h>
#include "variable.h"

variable_t* variable_create() {
    variable_t* ret = (variable_t*) malloc(sizeof(variable_t));
    ret->participatingClauses = NULL;
    ret->currentAssignment = 0;
    ret->unsatTrueLiteralCount = 0;
    ret->unsatNegatedLiteralCount = 0;
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

void variable_set_unsat_true_literal_count(variable_t* variable, int new_count) {
    variable->unsatTrueLiteralCount = new_count;
}

void variable_set_unsat_negated_literal_count(variable_t* variable, int new_count) {
    variable->unsatNegatedLiteralCount = new_count;
}

// The purity of a variable is represented by:
// 0 : The variable is not a pure literal
// 1 : The variable is a purely true literal variable
// -1: The variable is a purely negated literal variable
int variable_get_purity(variable_t* variable) {
    int unsatTrueLiteralCount = variable->unsatTrueLiteralCount;
    int unsatNegatedLiteralCount = variable->unsatNegatedLiteralCount;
    if (unsatTrueLiteralCount > 0 && unsatNegatedLiteralCount == 0) {
        // Variable occurs only in true literals
        return 1;
    } else if (unsatNegatedLiteralCount > 0 && unsatTrueLiteralCount == 0) {
        // Variable occurs only in negated literals
        return -1;
    } else {
        return 0;
    }
}

