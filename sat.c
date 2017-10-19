#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "arraylist.h"
#include "clause.h"

clause_t* parseClause(const char* line) {
    char* savePtr;
    char* token = strtok_r(line, " ", &savePtr);
    clause_t* ret = (clause_t*) malloc(sizeof(clause_t));
    ret->literals = createArrayList();
    while(token != NULL) {
        signed lit = atoi(token);
        if(lit == 0) {
            return ret;
        }
        literal_t * litPtr = malloc(sizeof(literal_t));
        *litPtr = lit;
        insert(ret->literals, (void*)litPtr);
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
    size_t i = 0;
    // our variables
    int numVariables = -1;
    int numClauses = -1;
    clause_t** clauses = NULL;
    while ((read = getline(&line, &len, fp)) != -1) {
        if(line[0] == 'c') {
            continue;
        }
        if(line[0] == 'p') {
            char str[4];
            sscanf(line, "p %s %d %d", str, &numVariables, &numClauses);
            assert(strncmp(str, "cnf", 4) == 0);
            assert(numVariables >= 0);
            assert(numClauses >= 0);
            clauses = malloc(numClauses * sizeof(clause_t*));
            assert(clauses);
            printf("type: %s, nClauses: %d, nVariables: %d\n", str, numClauses, numVariables);
        } else {
            clauses[i++] = parseClause(line);
        }
    }
    for(int x = 0; x < numClauses; x++) {
        printAll(clauses[x]->literals);
        destroy(clauses[x]->literals);
        free(clauses[x]);
        printf("--\n");
    }
    printf("UNSAT\n");
    fclose(fp);
    free(line);
    free(clauses);
    exit(EXIT_SUCCESS);
}
