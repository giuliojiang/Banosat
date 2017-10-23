#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "arraylist.h"
#include "clause.h"
#include "context.h"
#include "variable.h"

clause_t* parseClause(char* line, arraymap_t* variables) {
    char* savePtr;
    char* token = strtok_r(line, " ", &savePtr);
    clause_t* ret = (clause_t*) malloc(sizeof(clause_t));
    ret->literals = arraylist_create();
    ret->participating_conflicting = NULL;
    ret->participating_unsat = NULL;
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
        if(line[0] == 'c') {
            continue;
        }
        if(line[0] == 'p') {
            char str[4];
            sscanf(line, "p %s %d %*d", str, &numVariables);
            assert(strncmp(str, "cnf", 4) == 0);
            assert(numVariables >= 0);
            printf("type: %s, nVariables: %d\n", str, numVariables);
        } else {
            arraylist_insert(clauses, parseClause(line, variables));
        }
    }

    // Create context
    context_t* context = context_create();
    context_set_formula(context, clauses);
    context_set_all_unsat(context, clauses);
    context_set_variables(context, variables);
    context_print_formula(context);

    printf("UNSAT\n");
    fclose(fp);
    free(line);
    context_destroy(context);
    exit(EXIT_SUCCESS);
}
