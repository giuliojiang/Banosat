#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "arraylist.h"
#include "clause.h"
#include "context.h"

clause_t* parseClause(char* line) {
    char* savePtr;
    char* token = strtok_r(line, " ", &savePtr);
    clause_t* ret = (clause_t*) malloc(sizeof(clause_t));
    ret->literals = arraylist_create();
    while(token != NULL) {
        signed lit = atoi(token);
        if(lit == 0) {
            return ret;
        }
        literal_t * litPtr = malloc(sizeof(literal_t));
        *litPtr = lit;
        arraylist_insert(ret->literals, (void*)litPtr);
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

    ssize_t read;
    char* line = NULL;
    size_t len = 0;
    // our variables
    int numVariables = -1;
    arrayList_t* clauses = arraylist_create();
    while ((read = getline(&line, &len, fp)) != -1) {
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
            arraylist_insert(clauses, parseClause(line));
        }
    }
    /*for(int x = 0; x < numClauses; x++) {
        //arraylist_print_all(clauses[x]->literals);
        arraylist_destroy(clauses[x]->literals);
        free(clauses[x]);
        printf("--\n");
    }*/

    // Create context
    context_t* context = context_create();
    context_set_formula(context, clauses);
    context_print_formula(context);

    printf("UNSAT\n");
    fclose(fp);
    free(line);
    // TODO we also need to free the content of the clause
    // make a foreach method in the arraylist? Or an iterator?
    arraylist_destroy(clauses);
    exit(EXIT_SUCCESS);
}
