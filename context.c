#include "context.h"

#include "stdlib.h"

context_t* context_create() {
    context_t* ret = malloc(sizeof(context_t));
    ret->formula = NULL;
    ret->conflicts = NULL;
    return ret;
}

void context_set_formula(context_t* this, arrayList_t* formula) {
    this->formula = formula;
}

void context_add_conflict_clause(context_t* this, clause_t* clause) {
    arraylist_insert(this->conflicts, (void*) clause);
}

void context_print_formula(context_t* this) {
    arrayList_t* formula = this->formula;
    size_t num_clauses = arraylist_size(formula);
    for (size_t i = 0; i < num_clauses; i++) {
        arraylist_print_all(((clause_t*) arraylist_get(formula, i))->literals);
    }
}

static void clearClauses(void* elem, void* aux) {
    clause_t* clause = (clause_t*) elem;
    arraylist_destroy(clause->literals, NULL, NULL);
}

void context_destroy(context_t* this) {
    arraylist_destroy(this->formula, &clearClauses, NULL);
    free(this);
}