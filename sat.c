#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "arraylist.h"
#include "clause.h"
#include "context.h"
#include "variable.h"
#include "engine.h"
#include "parser.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("sat accepts only 1 argument which is the filename of the formula.\n");
        exit(EXIT_FAILURE);
    }

    FILE *fp = fopen(argv[1], "r");
    if(!fp) {
        printf("File %s does not exist", argv[1]);
        exit(EXIT_FAILURE);
    }

    char* line = NULL;
    size_t len = 0;
    // our variables
    int numVariables = -1;
    arrayList_t* clauses = arraylist_create();
    arraymap_t* variables = arraymap_create();
    while ((getline(&line, &len, fp)) != -1) {
        if(strlen(line) == 1 && strncmp(line, "\0", 1)) { // Deals with empty lines
            continue;
        }
        if(line[0] == 'c') {
            continue;
        }
        if(line[0] == 'p') {
            char str[4];
            sscanf(line, "p %s %d %*d", str, &numVariables);
            assert(strncmp(str, "cnf", 4) == 0);
            assert(numVariables >= 0);
            LOG_DEBUG("type: %s, nVariables: %d\n", str, numVariables);
        } else {
            clause_t* parsed_clause = parser_parse_clause(line, variables);
            if (parsed_clause) {
                arraylist_insert(clauses, parsed_clause);
            }
        }
    }

    // Create context
    context_t* context = context_create();
    context_set_variables(context, variables, (size_t)numVariables);
    context_set_formula(context, clauses);
    context_print_current_state(context);
    
    bool satisfiable = engine_run_solver(context);
    context_print_current_state(context);
    LOG_DEBUG("\nMAIN: Satisfiability is %d\n", satisfiable);
    
    if (satisfiable) {
        printf("SAT\n");
        context_print_result_variables(context);
    } else {
        printf("UNSAT\n");
    }

    fclose(fp);
    free(line);
    context_destroy(context);
    exit(EXIT_SUCCESS);
}
