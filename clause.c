#include "clause.h"
#include "macros.h"

#include <stdio.h>

// clause_print ---------------------------------------------------------------

void clause_print(clause_t* this) {
    if (ENABLE_DEBUG) {
        arrayList_t* clause_literals = this->literals;
        LOG_DEBUG("Clause: ");
        for (size_t j = 0; j < arraylist_size(clause_literals); j++) {
            literal_t* a_literal = (literal_t*) arraylist_get(clause_literals, j);
            LOG_DEBUG("%d\t", *a_literal);
        }
        LOG_DEBUG("\n");
    }
}

// clause_create --------------------------------------------------------------

clause_t* clause_create() {
    clause_t* result = malloc(sizeof(clause_t));
    result->literals = arraylist_create();
    result->participating_unsat = NULL;
    result->participating_false_clauses = NULL;
    result->variables_removal_nodes = linkedlist_create();
    return result;
}

// clause_add_literal ---------------------------------------------------------

// Adds a clause to the literal
//
// <seen_literals>    hashset<signed> The literals already seen in the current clause
// return             true if added successfully
//                    false if the clause is automatically True
bool clause_add_literal(clause_t* this, int literal_value, hashset_t* seen_literals) {
    if (hashset_contains(seen_literals, (void*) (long) literal_value)) {
        // Do nothing, because the same literal already exists
        return true;
    } else if (hashset_contains(seen_literals, (void*) (long) (-literal_value))) {
        // The negation of this literal is in the same clause, the entire clause is True
        return false;
    } else {
        // Add to hashset
        hashset_add(seen_literals, (void*) (long) literal_value);
        // Add to clause
        literal_t* lit_ptr = malloc(sizeof(literal_t));
        *lit_ptr = literal_value;
        arrayList_t* literals = this->literals; // arraylist<literal_t*>
        arraylist_insert(literals, lit_ptr);
        return true;
    }
}
