#include "context.h"

#include "stdio.h"
#include "stdlib.h"
#include "variable.h"
#include "macros.h"
#include "clause.h"

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

// context_assign_variable_value ----------------------------------------------

static int context_eval_clause(context_t* this, clause_t* clause) {
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

static void context_remove_clause_from_unsat(context_t* this, clause_t* clause) {
    linkedlist_remove_node(this->unsat, clause->participating_unsat);
    clause->participating_unsat = NULL;
}

static void context_add_false_clause(context_t* this, clause_t* clause) {
    linkedlist_node_t* false_clause_node = linkedlist_add_last(this->false_clauses, clause);
    clause->participating_false_clauses = false_clause_node;
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

// context_unassign_variable --------------------------------------------------

void context_unassign_variable(context_t* this, size_t variable_index) {

    // update assignment value to 0 in the variable map
    arraymap_t* variables = this->variables; // {unsigned -> variable_t*}
    variable_t* the_variable = (variable_t*) arraymap_get(variables, variable_index);
    variable_set_raw_value(the_variable, 0); // 0 is the undefined value

    // get the participating clauses of the variable
    arrayList_t* participating_clauses = the_variable->participatingClauses; // arraylist<clause_t*>

    // for each clause
    for (int i = 0; i < arraylist_size(participating_clauses); i++) {
        clause_t* a_clause = arraylist_get(participating_clauses, i);

        // if participating_false_clauses is not NULL, remove the clause from the false_clauses
        // because if it's in the false clauses, the clause is currently false, but removing
        // an assignment will make it undefined
        linkedlist_node_t* participating_false_clause_node = a_clause->participating_false_clauses;
        if (participating_false_clause_node) {
            linkedlist_remove_node(this->false_clauses, participating_false_clause_node);
            a_clause->participating_false_clauses = NULL;
        }

        // if participating_unsat is NULL, add this clause to the unsat
        // because this means the clause was well-defined (either T or F), but now
        // undoing a variable assignment makes it undefined again
        linkedlist_node_t* participating_unsat = a_clause->participating_unsat;
        if (!participating_unsat) {
            linkedlist_t* unsat_list = this->unsat;
            linkedlist_node_t* newly_added_node = linkedlist_add_last(unsat_list, a_clause);
            a_clause->participating_unsat = newly_added_node;
        }
    }
}

// context_run_bcp ------------------------------------------------------------

typedef struct unassigned_variables_in_clause {
    size_t count;       // Number of unassigned literals in a clause
    literal_t* literal; // The first unassigned literal found
} unassigned_variables_in_clause_t;

// Counts number of unassigned variables in a clause, and returns the first
// unassigned variable, if any
// If there is a false clause in the formula, this function will not be run
// because BCP is run after the check on the entire formula
static unassigned_variables_in_clause_t count_unassigned_variables_in_clause(context_t* this, clause_t* the_clause) {
    // Get variables map
    arraymap_t* variables = this->variables; // {unsigned -> variable_t*}

    // Initialize counter
    unassigned_variables_in_clause_t result;
    result.count = 0;
    result.literal = NULL;

    // Get the literals
    arrayList_t* literals = the_clause->literals; // arraylist<literal_t*>

    // Loop through the literals
    for (int i = 0; i < arraylist_size(literals); i++) {
        literal_t* a_literal_ptr = (literal_t*) arraylist_get(literals, i);

        // Increment counter if the literal is unassigned in variables map
        size_t variable_index = abs(*a_literal_ptr);

        // Get the variable data
        variable_t* variable_data = arraymap_get(variables, variable_index);
        if (variable_data->currentAssignment == 0) {
            result.count++;
            if (!result.literal) {
                result.literal = a_literal_ptr;
            }
        }
    }

    return result;
}

static bool context_run_bcp_once

void context_run_bcp(context_t* this) {

    // Get unsatisfied clauses list

    //

}

// context_print_current_state ------------------------------------------------

void context_print_current_state_variable_printer(size_t key, void* value, void* UNUSED(aux)) {
    variable_t* the_variable = (variable_t*) value;
    printf("\t%lu:\t%d\n", key, the_variable->currentAssignment);
}

// clause_list is a linkedlist_t<clause_t*>
void context_print_current_state_print_clause_list(linkedlist_t* clause_list) {
    size_t curr_index = 0;
    for (linkedlist_node_t* curr = clause_list->head->next;
         curr != clause_list->tail;
         curr = curr->next) {
        clause_t* elem = (clause_t*) curr->value;
        arrayList_t* clause_literals = elem->literals;
        printf("Clause %u: ", curr_index);
        for (size_t j = 0; j < arraylist_size(clause_literals); j++) {
            literal_t* a_literal = (literal_t*) arraylist_get(clause_literals, j);
            printf("%d\t", *a_literal);
        }
        printf("\n");
        curr_index++;
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
