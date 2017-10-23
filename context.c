#include "context.h"

#include "stdlib.h"
#include "variable.h"

context_t* context_create() {
    context_t* ret = malloc(sizeof(context_t));
    ret->formula = NULL;
    ret->conflicts = NULL;
    ret->variables = NULL;
    ret->conflicting = linkedlist_create();
    ret->unsat = linkedlist_create();
    return ret;
}

void context_set_formula_lambda(void* item, void* aux) {
    clause_t* clause = (clause_t*) item;
    linkedlist_t* unsat = (linkedlist_t*) aux;
    linkedlist_add_last(unsat, clause);
}

void context_set_formula(context_t* this, arrayList_t* formula) {
    this->formula = formula;

    arraylist_foreach(formula, &context_set_formula_lambda, this->unsat);
}

void context_set_variables(context_t* this, arraymap_t* variables) {
    this->variables = variables;
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

static void context_destroy_variables(size_t UNUSED(key), void* value, void* UNUSED(aux)) {
    variable_t* var = (variable_t*) value;
    variable_destroy(var);
}
void context_destroy(context_t* this) {
    arraylist_destroy(this->formula, &clause_destroy, NULL);
    arraymap_destroy(this->variables, &context_destroy_variables, NULL);
    free(this);
}

void context_eval_literals(void* item, void* (aux)) {
    literal_t literal = *((literal_t*) item);
    context_t* this = (context_t*) aux;
    variable_t* variable = (variable_t*) arraymap_get(this->variables, abs(literal));
    int currAssignment = variable->currentAssignment;
    if (literal < 0) {
    
    }
}

int context_eval_clause(context_t* this, clause_t* clause) {
    arraymap_t* variables = this->variables;
    // For a clause, iterate over every literal
    // for the current variable assignment -> get abs(literal) and fetch the current assignment at that index from the variable map
    // if literal < 0, invert the assignment
    // undefined == no truths and at least one unassigned
    arraylist_foreach(clause->literals, &context_eval_literals, this);
}

// TODO: write eval_clauses

void context_assign_variable_value(context_t* this, size_t variable_index, bool new_value) {
    variable_t* variable = this->variables[variable_index];
    variable_set_value(variable, new_value);
    arrayList_t* participatingClauses = variable->participatingClauses;
    arraylist_foreach(participatingClauses, &eval_clauses, NULL);
}
