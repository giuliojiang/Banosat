#include "parser.h"

#include "string.h"

#include "arraymap.h"
#include "hashset.h"
#include "variable.h"
#include "literal.h"

clause_t* parser_parse_clause(char* line, context_t* context) {
    LOG_DEBUG("parser_parse_clause: Starting a new clause...\n");

    char* savePtr;
    char* token = strtok_r(line, " ", &savePtr);
    clause_t* ret = clause_create();
    bool clause_valid = true;

    // Create temporary Hash Set to track the variables already seen
    // Type hashset<signed>
    hashset_t* seen_literals = hashset_create(literal_hash_func, literal_equal_func);

    while(token != NULL) {
        signed lit = atoi(token);
        if(lit == 0) {
            break;
        }

        context_initialize_variable(context, abs(lit));

        bool contains_negation = false;
        clause_add_literal(ret, lit, seen_literals, &contains_negation);

        if (contains_negation) {
            clause_valid = false;
        }

        token = strtok_r(NULL, " ", &savePtr);
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
