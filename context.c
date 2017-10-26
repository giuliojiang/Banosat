#include "context.h"

#include "stdio.h"
#include "stdlib.h"
#include "variable.h"
#include "macros.h"
#include "clause.h"
#include "assignment_level.h"

context_t* context_create() {
    context_t* ret = malloc(sizeof(context_t));
    ret->formula = arraylist_create();
    ret->variables = NULL;
    ret->conflicts = arraylist_create();
    ret->unsat = linkedlist_create();
    ret->false_clauses = linkedlist_create();
    ret->assignment_history = linkedlist_create();
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

static void assignment_history_destroyer_func(linkedlist_node_t* node, void* UNUSED(aux)) {
    assignment_level_t* value = node->value;
    assignment_level_destroy(value);
}

void context_destroy(context_t* this) {
    arraylist_destroy(this->formula, &clause_destroy, NULL);
    arraymap_destroy(this->variables, &context_destroy_variables, NULL);

    // Destroy the assignment_history
    linkedlist_t* assignment_history = this->assignment_history; // linkedlist<assignment_level_t*>
    linkedlist_destroy(assignment_history, assignment_history_destroyer_func, NULL);

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
    fprintf(stderr, "context_remove_clause_from_unsat: removing following clause\n");
    clause_print(clause);
    if (!clause->participating_unsat) {
        fprintf(stderr, "this clause was already removed\n");
        return;
    }
    linkedlist_remove_node(this->unsat, clause->participating_unsat);
    clause->participating_unsat = NULL;
}

static void context_add_false_clause(context_t* this, clause_t* clause) {
    linkedlist_node_t* false_clause_node = linkedlist_add_last(this->false_clauses, clause);
    clause->participating_false_clauses = false_clause_node;
}

// Assigns a value to a variable
// Updates the variables map, unsat and false clauses lists and links
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
    for (size_t i = 0; i < arraylist_size(participating_clauses); i++) {
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
    for (size_t i = 0; i < arraylist_size(literals); i++) {
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

static void add_deduced_assignment(context_t* this, int new_assignment) {
    // Get current assignment level
    linkedlist_t* assignment_history = this->assignment_history; // linkedlist<assignment_level_t*>
    linkedlist_node_t* last_assignment_node = assignment_history->tail->prev;
    if (!last_assignment_node || (last_assignment_node == assignment_history->head)) {
        fprintf(stderr, "add_deduced_assignment: Could not find a valid assignment node to operate on\n");
        abort();
    }
    assignment_level_t* the_assignment_level = (assignment_level_t*) last_assignment_node->value;
    // Add to the assignment level's deduced assignments
    assignment_level_add_deduced_assignment(the_assignment_level, new_assignment);
}

// Returns true if BCP has made some progress
// Returns false if BCP could not find any singleton-clause to work on
static bool context_run_bcp_once(context_t* this) {
    // get unsatisfied clauses
    linkedlist_t* unsat = this->unsat; // linkedlist<clause_t*>

    // Search for clauses with 1 unassigned variable
    for (linkedlist_node_t* curr = unsat->head->next; curr != unsat->tail; curr = curr->next) {
        clause_t* a_clause = (clause_t*) curr->value;
        unassigned_variables_in_clause_t unassigned_variables = count_unassigned_variables_in_clause(this, a_clause);
        if (unassigned_variables.count == 1) {
            // Found a variable on which to do BCP
            literal_t* literal_ptr = unassigned_variables.literal;
            literal_t literal_value = *literal_ptr;
            // Get variable index
            size_t variable_index = abs(literal_value);
            bool new_assignment;
            if (literal_value > 0) {
                // Assign true
                new_assignment = true;
            } else {
                // Assign false
                new_assignment = false;
            }
            // Make the assignment and update the assignment history
            fprintf(stderr, "BCP deciding to assign %lu to be %d\n", variable_index, new_assignment);
            context_assign_variable_value(this, variable_index, new_assignment);
            add_deduced_assignment(this, literal_value);
            return true;
        }
    }

    // No BCP processed
    return false;
}

// Returns an evaluation value of the formula
int context_run_bcp(context_t* this) {
    while (true) {
        int formula_value = context_evaluate_formula(this);
        fprintf(stderr, "context_run_bcp: Formula is currently true/false? %d\n", formula_value);
        if (formula_value == 0) {
            bool res = context_run_bcp_once(this);
            if (!res) {
                // BCP did not make any progress, return an evaluation
                return context_evaluate_formula(this);
            }
            // Otherwise, continue looping
        } else {
            // We already know if formula is true or false, return
            return formula_value;
        }
    }
}

// context_print_current_state ------------------------------------------------

void context_print_current_state_variable_printer(size_t key, void* value, void* UNUSED(aux)) {
    variable_t* the_variable = (variable_t*) value;
    fprintf(stderr, "\t%lu:\t%d\n", key, the_variable->currentAssignment);
}

// clause_list is a linkedlist_t<clause_t*>
void context_print_current_state_print_clause_list(linkedlist_t* clause_list) {
    size_t curr_index = 0;
    for (linkedlist_node_t* curr = clause_list->head->next;
         curr != clause_list->tail;
         curr = curr->next) {
        clause_t* elem = (clause_t*) curr->value;
        arrayList_t* clause_literals = elem->literals;
        fprintf(stderr, "Clause %lu: ", curr_index);
        for (size_t j = 0; j < arraylist_size(clause_literals); j++) {
            literal_t* a_literal = (literal_t*) arraylist_get(clause_literals, j);
            fprintf(stderr, "%d\t", *a_literal);
        }
        fprintf(stderr, "\n");
        curr_index++;
    }
}

void context_print_current_state(context_t* this) {
    // Clauses of the formula
    fprintf(stderr, "\nFormula clauses:\n");
    arrayList_t* formula = this->formula;
    for (size_t i = 0; i < arraylist_size(formula); i++) {
        clause_t* elem = (clause_t*) arraylist_get(formula, i);
        // Print the literals
        fprintf(stderr, "Clause %lu = ", i);
        arrayList_t* clause_literals = elem->literals;
        for (size_t j = 0; j < arraylist_size(clause_literals); j++) {
            literal_t* a_literal = (literal_t*) arraylist_get(clause_literals, j);
            fprintf(stderr, "%d\t", *a_literal);
        }
        fprintf(stderr, "\n");
    }
    // Variables map
    fprintf(stderr, "\nVariable map:\n");
    arraymap_t* variables = this->variables;
    if (!variables) {
        fprintf(stderr, "is NULL\n");
    } else {
        arraymap_foreach_pair(variables, context_print_current_state_variable_printer, NULL);
    }
    // Unsatisfied clauses
    fprintf(stderr, "\nUnsat clauses:\n");
    context_print_current_state_print_clause_list(this->unsat);
    // False clauses
    fprintf(stderr, "\nFalse clauses:\n");
    context_print_current_state_print_clause_list(this->false_clauses);
    
    fprintf(stderr, "\n");
}

// context_evaluate_formula ---------------------------------------------------

// Returns:
// -1 False
//  0 Unknown
// +1 True
int context_evaluate_formula(context_t* this) {
    // Check false_clauses list
    linkedlist_t* false_clauses = this->false_clauses; // linkedlist<clause_t*>
    if (linkedlist_size(false_clauses) > 0) {
        return -1;
    }

    // Check unsat list
    linkedlist_t* unsat = this->unsat; // linkedlist<clause_t*>
    if (linkedlist_size(unsat) == 0) {
        return 1;
    }

    // Otherwise we don't know enough about the formula yet
    return 0;
}

// context_apply_new_decision_level -----------------------------------------------------

// The decision step can make a variable decision assignment.
// We create a new decision history level
// Does NOT actually assign the new variable
void context_apply_new_decision_level(context_t* this, size_t variable_index, bool new_value) {
    // Update the assignment history with a new node
    int assignment_value = new_value ? variable_index : -variable_index;
    linkedlist_t* assignment_history = this->assignment_history; // linkedlist<assignment_level_t*>
    assignment_level_t* new_level = assignment_level_create(linkedlist_size(assignment_history), assignment_value);
    linkedlist_add_last(assignment_history, new_level);
}

// context_get_last_assignment_level ------------------------------------------

assignment_level_t* context_get_last_assignment_level(context_t* this) {
    // Get the assignment history
    linkedlist_t* assignment_history = this->assignment_history; // linkedlist<assignment_level_t*>
    // Return null if it's empty
    if (linkedlist_size(assignment_history) == 0) {
        return NULL;
    }
    // Get last element
    linkedlist_node_t* assignment_history_last_node = assignment_history->tail->prev;
    assignment_level_t* last_assignment_level = (assignment_level_t*) assignment_history_last_node->value;
    return last_assignment_level;
}

// context_remove_last_assignment_level ---------------------------------------

// Removes the last assignment level
// Does not free the assignment_level
assignment_level_t* context_remove_last_assignment_level(context_t* this) {
    linkedlist_t* assignment_history = this->assignment_history; // linkedlist<assignment_level_t*>
    assignment_level_t* last_assignment_level = (assignment_level_t*) linkedlist_remove_last(assignment_history);
    return last_assignment_level;
}

// context_get_first_variable_index -------------------------------------------

// Returns the first variable index in the mapping
// Returns 0 if variable map contains nothing
size_t context_get_first_variable_index(context_t* this) {
    // Get the variables map
    arraymap_t* variables = this->variables; // arraymap<unsigned, variable_t*>
    arraymap_pair_t first_variable_data = arraymap_find_first_entry(variables);
    if (!first_variable_data.v) {
        // No first element found
        return 0;
    } else {
        // The key of the map is the variable index
        return first_variable_data.k;
    }
}

// context_get_next_variable_index --------------------------------------------

// Returns the index of the next variable in the mapping
// Returns 0 if reaches the end of the map
size_t context_get_next_variable_index(context_t* this, size_t previous) {
    // Get the variables map
    arraymap_t* variables = this->variables; // arraymap<unsigned, variable_t*>
    arraymap_pair_t next_variable_data = arraymap_find_next_entry(variables, previous);
    if (!next_variable_data.v) {
        // Reached the end of the map
        return 0;
    } else {
        return next_variable_data.k;
    }
}
