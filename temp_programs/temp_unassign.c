#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "../arraylist.h"
#include "../clause.h"
#include "../context.h"
#include "../variable.h"

clause_t* parseClause(char* line, arraymap_t* variables) {
    char* savePtr;
    char* token = strtok_r(line, " ", &savePtr);
    clause_t* ret = (clause_t*) malloc(sizeof(clause_t));
    ret->literals = arraylist_create();
    ret->participating_unsat = NULL;
    ret->participating_false_clauses = NULL;
    while(token != NULL) {
        signed lit = atoi(token);
        if(lit == 0) {
            return ret;
        }
        literal_t * litPtr = malloc(sizeof(literal_t));
        *litPtr = lit;
        arraylist_insert(ret->literals, (void*)litPtr);
        variable_add_value_into_map(variables, lit, ret);
        token = strtok_r(NULL, " ", &savePtr);
    }
    return ret;
}

int main(int UNUSED(argc), char **argv) {

    FILE *fp = fopen("../tests/test4.cnf", "r");
    if(!fp) {
        fprintf(stderr, "File %s does not exist", argv[1]);
        exit(EXIT_FAILURE);
    }

    char* line = NULL;
    size_t len = 0;
    // our variables
    int numVariables = -1;
    arrayList_t* clauses = arraylist_create();
    arraymap_t* variables = arraymap_create();
    while ((getline(&line, &len, fp)) != -1) {
        if(line[0] == 'c') {
            continue;
        }
        if(line[0] == 'p') {
            char str[4];
            sscanf(line, "p %s %d %*d", str, &numVariables);
            assert(strncmp(str, "cnf", 4) == 0);
            assert(numVariables >= 0);
            fprintf(stderr, "type: %s, nVariables: %d\n", str, numVariables);
        } else {
            arraylist_insert(clauses, parseClause(line, variables));
        }
    }

    // Create context
    context_t* context = context_create();
    context_print_current_state(context);
    
    context_set_formula(context, clauses);
    context_set_variables(context, variables);
    
    context_print_current_state(context);
    
    // FORMULA IS 
    // p cnf 3 4
    // 1 0
    // -1 2 3 0
    // 1 -2 3 0
    // 1 2 -3 0

    // Try to assign variable 1 = T
    context_assign_variable_value(context, 1, true);
    fprintf(stderr, "\n\n===AFTER ASSIGNING 1=T ======\n\n");
    context_print_current_state(context);
    
    // Try to un-assign variable 1
    context_unassign_variable(context, 1);
    fprintf(stderr, "\n\n===AFTER UNASSIGNING 1 ======\n\n");
    context_print_current_state(context);
    
    fprintf(stderr, "UNSAT\n");
    fclose(fp);
    free(line);
    context_destroy(context);
    exit(EXIT_SUCCESS);
}
