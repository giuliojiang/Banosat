#include "context.h"

#include "stdio.h"
#include "stdlib.h"
#include "variable.h"
#include "macros.h"

context_t* context_create() {
    context_t* ret = malloc(sizeof(context_t));
    ret->formula = arraylist_create();
    ret->variables = NULL;
    ret->conflicts = arraylist_create();
    ret->unsat = linkedlist_create();
    ret->false_clauses = linkedlist_create();
    return ret;
}

void context_set_formula_lambda(void* item, void* aux) {
    clause_t* clause = (clause_t*) item;
    linkedlist_t* unsat = (linkedlist_t*) aux;
    linkedlist_node_t* new_clause_node = linkedlist_add_last(unsat, clause);
    clause->participating_unsat = new_clause_node;
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

int context_eval_clause(context_t* this, clause_t* clause) {
    arraymap_t* variables = this->variables;
    for (size_t i = 0; i < arraylist_size(clause->literals); i++) {
        literal_t literal = *((literal_t*) clause->literals->array[i]);
        variable_t* variable = (variable_t*) arraymap_get(variables, abs(literal));
        int currAssignment = variable->currentAssignment;
        if (literal < 0) {
            currAssignment = -currAssignment;
        }
        if (currAssignment > 0) {
            return 1;
        }
        if (currAssignment == 0) {
            return 0;
        }
    }
    return -1;
}

void context_remove_clause_from_unsat(context_t* this, clause_t* clause) {
    linkedlist_remove_node(this->unsat, clause->participating_unsat);
}

void context_assign_variable_value(context_t* this, size_t variable_index, bool new_value) {
    variable_t* variable = (variable_t*) arraymap_get(this->variables, variable_index);
    variable_set_value(variable, new_value);
    arrayList_t* participatingClauses = variable->participatingClauses;
    for (size_t i = 0; i < arraylist_size(participatingClauses); i++) {
        clause_t* clause = arraylist_get(participatingClauses, i);
        int eval = context_eval_clause(this, clause);
        if (eval > 0) {
            context_remove_clause_from_unsat(this, clause);
        } else if (eval < 0) {
            context_add_false_clause(this, clause);
        }
    }
}

void context_add_false_clause(context_t* this, clause_t* clause) {
    linkedlist_node_t* false_clause_node = linkedlist_add_last(this->false_clauses, clause);
    clause->participating_false_clauses = false_clause_node;
}

// context_print_current_state ------------------------------------------------

void context_print_current_state_variable_printer(size_t key, void* value, void* UNUSED(aux)) {
    variable_t* the_variable = (variable_t*) value;
    printf("\t%lu:\t%d\n", key, the_variable->currentAssignment);
}

// clause_list is a linkedlist_t<clause_t*>
void context_print_current_state_print_clause_list(linkedlist_t* clause_list) {
    for (linkedlist_node_t* curr = clause_list->head->next;
         curr != clause_list->tail;
         curr = curr->next) {
        clause_t* elem = (clause_t*) curr->value;
        arrayList_t* clause_literals = elem->literals;
        printf("Clause: ");
        for (size_t j = 0; j < arraylist_size(clause_literals); j++) {
            literal_t* a_literal = (literal_t*) arraylist_get(clause_literals, j);
            printf("%d\t", *a_literal);
        }
        printf("\n");
    }
}

void context_print_current_state(context_t* this) {
    // Clauses of the formula
    printf("\nFormula clauses:\n");
    arrayList_t* formula = this->formula;
    for (size_t i = 0; i < arraylist_size(formula); i++) {
        clause_t* elem = (clause_t*) arraylist_get(formula, i);
        // Print the literals
        printf("Clause %lu = ", i);
        arrayList_t* clause_literals = elem->literals;
        for (size_t j = 0; j < arraylist_size(clause_literals); j++) {
            literal_t* a_literal = (literal_t*) arraylist_get(clause_literals, j);
            printf("%d\t", *a_literal);
        }
        printf("\n");
    }
    // Variables map
    printf("\nVariable map:\n");
    arraymap_t* variables = this->variables;
    if (!variables) {
        printf("is NULL\n");
    } else {
        arraymap_foreach_pair(variables, context_print_current_state_variable_printer, NULL);
    }
    // Unsatisfied clauses
    printf("\nUnsat clauses:\n");
    context_print_current_state_print_clause_list(this->unsat);
    // False clauses
    printf("\nFalse clauses:\n");
    context_print_current_state_print_clause_list(this->false_clauses);
    
    printf("\n");
}
