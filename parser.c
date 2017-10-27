#include "parser.h"

#include "string.h"

#include "arraymap.h"
#include "hashset.h"
#include "variable.h"

// input will be cast to a signed
static unsigned literal_hash_func(void* input) {
    signed literal = (signed) (long) input;
    unsigned result = abs(literal);
    return result;
}

// pa and pb are considered to be of type signed
static bool literal_equal_func(void* pa, void* pb) {
    return pa == pb;
}

clause_t* parser_parse_clause(char* line, arraymap_t* variables) {
    LOG_DEBUG("parser_parse_clause: Starting a new clause...\n");

    char* savePtr;
    char* token = strtok_r(line, " ", &savePtr);
    clause_t* ret = (clause_t*) malloc(sizeof(clause_t));
    ret->literals = arraylist_create();
    ret->participating_unsat = NULL;
    ret->participating_false_clauses = NULL;
    bool clause_valid = true;

    // Create temporary Hash Set to track the variables already seen
    // Type hashset<signed>
    hashset_t* seen_literals = hashset_create(literal_hash_func, literal_equal_func);

    while(token != NULL) {
        signed lit = atoi(token);
        if(lit == 0) {
            break;
        }
        signed lit_neg = -lit;
        // Check existence in hashset
        if (hashset_contains(seen_literals, (void*) (long) lit)) {
            // If the same literal exists, skip it
            // Do nothing
            LOG_DEBUG("parser_parse_clause: Skipping a literal as it's duplicated in the clause: %u\n", lit);
            token = strtok_r(NULL, " ", &savePtr);
        } else if (hashset_contains(seen_literals, (void*) (long) lit_neg)) {
            // The negation of this literal is in the same clause,
            // P v ¬P = True, so the entire clause is always true, so we skip it
            LOG_DEBUG("parser_parse_clause: Skipping a clause as there is a literal and its negation: %u\n", lit);
            clause_valid = false;
            break;
        } else {
            // Add to hashset
            hashset_add(seen_literals, (void*) (long) lit);
            literal_t * litPtr = malloc(sizeof(literal_t));
            *litPtr = lit;
            LOG_DEBUG("parser_parse_clause: Adding literal %d\n", lit);
            arraylist_insert(ret->literals, (void*)litPtr);
            variable_add_value_into_map(variables, lit, ret);
            token = strtok_r(NULL, " ", &savePtr);
        }
    }

    // Free the temporary hashset
    hashset_destroy(seen_literals, NULL, NULL);

    if (clause_valid) {
        return ret;
    } else {
        clause_destroy(ret, NULL);
        return NULL;
    }
}
