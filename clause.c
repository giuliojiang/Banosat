#include "clause.h"

#include <stdio.h>

void clause_print(clause_t* this) {
    arrayList_t* clause_literals = this->literals;
    fprintf(stderr, "Clause: ");
    for (size_t j = 0; j < arraylist_size(clause_literals); j++) {
        literal_t* a_literal = (literal_t*) arraylist_get(clause_literals, j);
        printf("%d\t", *a_literal);
    }
    printf("\n");
}
