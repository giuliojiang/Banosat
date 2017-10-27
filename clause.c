#include "clause.h"
#include "macros.h"

#include <stdio.h>

void clause_print(clause_t* this) {
#ifdef DEBUG
    arrayList_t* clause_literals = this->literals;
    LOG_DEBUG("Clause: ");
    for (size_t j = 0; j < arraylist_size(clause_literals); j++) {
        literal_t* a_literal = (literal_t*) arraylist_get(clause_literals, j);
        LOG_DEBUG("%d\t", *a_literal);
    }
    LOG_DEBUG("\n");
#endif
}
