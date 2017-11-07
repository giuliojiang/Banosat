#include "context.h"

#include "stdio.h"
#include "stdlib.h"
#include "variable.h"
#include "macros.h"
#include "clause.h"
#include "assignment_level.h"

// Forward declarations -------------------------------------------------------

void mergeSort(size_t *arr, size_t l, size_t r, arraymap_t* variables);

static int context_eval_clause(context_t* this, clause_t* clause);

static void context_add_clause_to_unsat(context_t* this, clause_t* clause);

static void context_add_false_clause(context_t* this, clause_t* clause);

static void context_remove_clause_from_unsat(context_t* this, clause_t* clause);

// Constructor ----------------------------------------------------------------

context_t* context_create() {
    context_t* ret = malloc(sizeof(context_t));
    ret->formula = arraylist_create();
    ret->conflicts = linkedlist_create();
    ret->variables = arraymap_create();
    ret->unsat = linkedlist_create();
    ret->false_clauses = linkedlist_create();
    ret->assignment_history = linkedlist_create();
    ret->sorted_indices = NULL;
    ret->numVariables = 0;
    ret->unit_clauses = linkedlist_create();
    return ret;
}

// Update state for variables within the clause; unsat(True/Negated)LiteralCounts
// will change for every literal within
static void context_update_clause_literal_purity_counts(context_t* this, clause_t* clause, int counter) {
  arrayList_t* literals = clause->literals;
  for (size_t j = 0; j < arraylist_size(literals); j++) {
      literal_t* a_literal = (literal_t*) arraylist_get(literals, j);
      size_t variable_index = abs(*a_literal);
      variable_t* a_variable = (variable_t*) arraymap_get(this->variables, variable_index);
      if (*a_literal > 0) {
          variable_set_unsat_true_literal_count(a_variable, a_variable->unsatTrueLiteralCount + counter);
      } else if (*a_literal < 0) {
          variable_set_unsat_negated_literal_count(a_variable, a_variable->unsatNegatedLiteralCount + counter);
      }
  }
}

// context_add_clause ---------------------------------------------------------

static void context_add_clause_setup_dependencies(context_t* this, clause_t* new_clause) {
    // Link participating clauses in the variables
    arraymap_t* variables_map = this->variables; // arraymap<size_t, variable_t*>
    arrayList_t* clause_literals = new_clause->literals; // arraylist<literal_t*>
    for (unsigned i = 0; i < arraylist_size(clause_literals); i++) {
        literal_t* a_literal = (literal_t*) arraylist_get(clause_literals, i);
        size_t variable_index = abs(*a_literal);
        if (!arraymap_get(variables_map, variable_index)) {
            arraymap_put(variables_map, variable_index, variable_create());
        }
        variable_t* a_variable_struct = (variable_t*) arraymap_get(variables_map, variable_index);
        variable_insert_clause(a_variable_struct, new_clause);
    }

    // Try to evaluate the clause
    int clause_eval = context_eval_clause(this, new_clause);

    // Add to unsat if necessary
    if (clause_eval == 0) {
        context_add_clause_to_unsat(this, new_clause);
    }

    // Add to false if necessary
    if (clause_eval == -1) {
        context_add_false_clause(this, new_clause);
    }

    // Update purity counts
    context_update_clause_literal_purity_counts(this, new_clause, 1);
}

void context_add_clause(context_t* this, clause_t* new_clause) {
    // Add to the formula
    arrayList_t* formula = this->formula; // arraylist<clause_t*>
    arraylist_insert(formula, new_clause);
    context_add_clause_setup_dependencies(this, new_clause);
}

void context_add_conflict_clause(context_t* this, clause_t* new_clause) {
    // Add to conflicts
    linkedlist_t* conflicts = this->conflicts; // linkedlist<clause_t*>
    linkedlist_add_last(conflicts, new_clause);
    context_add_clause_setup_dependencies(this, new_clause);
}

bool context_remove_first_conflict_clause(context_t* this) {
    linkedlist_t* conflicts = this->conflicts; // linkedlist<clause_t*>
    if (linkedlist_size(conflicts) == 0) {
        return false;
    }

    linkedlist_node_t* first_node = conflicts->head->next;
    clause_t* removal_clause = (clause_t*) first_node->value;

    // Remove from unsat
    if (removal_clause->participating_unsat) {
        context_remove_clause_from_unsat(this, removal_clause);
    }

    // Remove from false
    if (removal_clause->participating_false_clauses) {
        linkedlist_remove_node(this->false_clauses, removal_clause->participating_false_clauses);
        removal_clause->participating_false_clauses = NULL;
    }

    // Remove from unit clauses
    if (removal_clause->participating_unit_clause) {
        linkedlist_remove_node(this->unit_clauses, removal_clause->participating_unit_clause);
        removal_clause->participating_unit_clause = NULL;
    }

    // Remove links from referenced variables
    arraymap_t* variables_map = this->variables; // arraymap<size_t, variable_t*>
    linkedlist_t* variables_removal_nodes = removal_clause->variables_removal_nodes; // linkedlist<linkedlist_node<clause_t*>>
    arrayList_t* clause_literals = removal_clause->literals; // arraylist<literal_t*>
    linkedlist_node_t* curr = variables_removal_nodes->head->next;
    for (unsigned i = 0; i < arraylist_size(clause_literals); i++) {
        literal_t* a_literal = (literal_t*) arraylist_get(clause_literals, i);
        size_t variable_index = abs(*a_literal);
        variable_t* a_variable_struct = (variable_t*) arraymap_get(variables_map, variable_index);

        // Get the corresponding removal node for the variable
        linkedlist_node_t* next_removal = curr->value;
        // Remove from the variables
        linkedlist_remove_node(a_variable_struct->participatingClauses, next_removal);

        curr = curr->next;
    }

    // Remove from linkedlist
    linkedlist_remove_node(conflicts, first_node);

    // Free the clause
    clause_destroy(removal_clause, NULL);

    return true;
}

// context_set_variables ------------------------------------------------------

// Copies all the keys from variable (arraymap<literal, variable_t> into aux
static void context_populate_sorted_indices(size_t key, void *UNUSED(value), void *aux) {
    // Local static variable to keep track of the index.
    // Doing it this way so that I don't need to make a struct to store it in aux.
    static size_t index = 0;
    size_t* aList = aux;
    aList[index++] = key;
}

// Finalizes the variables, and sorts them in decreasing order of the number of
// clauses they participate in
// After this call, no additional variables should be added to the context.
// It is legal to add new clauses (typically conflict clauses) but they must not
// contain any new variable
void context_finalize_variables(context_t* this, size_t numVariables) {
    this->numVariables = numVariables;
    size_t* sortedIndices = calloc(sizeof(size_t), numVariables);
    arraymap_foreach_pair(this->variables, &context_populate_sorted_indices, sortedIndices);
    // Sorts the array based on the number of clauses in which each variable appears in
    // Descending order.
    mergeSort(sortedIndices, 0, numVariables-1, this->variables);
    this->sorted_indices = sortedIndices;
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
    arraymap_destroy(this->variables, &context_destroy_variables, NULL);
    arraylist_destroy(this->formula, &clause_destroy, NULL);

    // Destroy the assignment_history
    linkedlist_t* assignment_history = this->assignment_history; // linkedlist<assignment_level_t*>
    linkedlist_destroy(assignment_history, assignment_history_destroyer_func, NULL);
    linkedlist_destroy(this->false_clauses, NULL, NULL);
    linkedlist_destroy(this->unsat, NULL, NULL);

    for (linkedlist_node_t* curr = this->conflicts->head->next; curr != this->conflicts->tail; curr = curr->next) {
        clause_t* a_clause = curr->value;
        clause_destroy(a_clause, NULL);
    }

    linkedlist_destroy(this->conflicts, NULL, NULL);
    linkedlist_destroy(this->unit_clauses, NULL, NULL);
    free(this->sorted_indices);
    free(this);
}

// context_assign_variable_value ----------------------------------------------

// Evaluates a single clause
//
// Return:
//     -1  The clause is False
//      0  The clause is unknown
//     +1  The clause is True
static int context_eval_clause(context_t* this, clause_t* clause) {
    arraymap_t* variables = this->variables;
    int unassigned_literals = 0;
    for (size_t i = 0; i < arraylist_size(clause->literals); i++) {
        literal_t* literal = (literal_t*) clause->literals->array[i];
        variable_t* variable = (variable_t*) arraymap_get(variables, abs(*literal));
        int currAssignment = variable->currentAssignment;
        int literal_eval = *literal * currAssignment;
        if (literal_eval < 0) {
            // Got a false literal
            // do nothing
        } else if (literal_eval == 0) {
            // Got an unassigned literal
            // Increment number of unassigned literals in this clause
            clause->an_unassigned_literal = literal;
            unassigned_literals++;
        } else {
            // Got a positive literal
            // The clause is true immediately
            if (clause->participating_unit_clause) {
                linkedlist_remove_node(this->unit_clauses, clause->participating_unit_clause);
                clause->participating_unit_clause = NULL;
            }
            return 1;
        }
    }

    // Update the unit clause: add to the linked list, or remove from it
    if (unassigned_literals == 1) {
        if (!clause->participating_unit_clause) {
            linkedlist_node_t* new_node = linkedlist_add_last(this->unit_clauses, clause);
            clause->participating_unit_clause = new_node;
        }
    }
    if (unassigned_literals != 1) {
        if (clause->participating_unit_clause) {
            linkedlist_remove_node(this->unit_clauses, clause->participating_unit_clause);
            clause->participating_unit_clause = NULL;
        }
    }

    if (unassigned_literals > 0) {
        // There were no positive literals, but some of them don't have an assignment
        return 0;
    } else {
        // Everything in this clause has an assignment, but there were no positive literals
        return -1;
    }
}

static void context_add_clause_to_unsat(context_t* this, clause_t* clause) {
    // Check if the clause is already in there
    if (clause->participating_unsat) {
        LOG_DEBUG("context_add_clause_to_unsat: attempting to add a clause to unsat list, but the clause's participating unsat field is not NULL, so it's probably already in the unsat list. Aborting...\n");
        LOG_DEBUG("The Clause that was being added is:\n");
        clause_print(clause);
        abort();
    }

    linkedlist_t* unsat = this->unsat; // linkedlist<clause_t*>
    linkedlist_node_t* unsat_node = linkedlist_add_last(unsat, clause);
    clause->participating_unsat = unsat_node;
}

static void context_remove_clause_from_unsat(context_t* this, clause_t* clause) {
    LOG_DEBUG("context_remove_clause_from_unsat: removing following clause\n");
    clause_print(clause);
    if (!clause->participating_unsat) {
        LOG_DEBUG("this clause was already removed\n");
        return;
    }
    context_update_clause_literal_purity_counts(this, clause, -1);
    linkedlist_remove_node(this->unsat, clause->participating_unsat);
    clause->participating_unsat = NULL;
}

static void context_add_false_clause(context_t* this, clause_t* clause) {
    LOG_DEBUG("\nAdding false clause:\n");
    clause_print(clause);
    linkedlist_node_t* false_clause_node = linkedlist_add_last(this->false_clauses, clause);
    if (clause->participating_false_clauses) {
        LOG_FATAL("This clause already participates in false\n");
    }
    clause->participating_false_clauses = false_clause_node;
}

// Assigns a value to a variable
// Updates the variables map, unsat and false clauses lists and links
void context_assign_variable_value(context_t* this, size_t variable_index, bool new_value) {
    variable_t* variable = (variable_t*) arraymap_get(this->variables, variable_index);
    // Initially variables are self-deduced (arbitrary decision) unless a call
    // to add_deduced_assignment is made
    linkedlist_add_last(variable->deduced_from, (void*) variable_index);
    variable_set_value(variable, new_value);

    linkedlist_t* participatingClauses = variable->participatingClauses;
    for (linkedlist_node_t* curr = participatingClauses->head->next; curr != participatingClauses->tail; curr = curr->next) {
        clause_t* clause = curr->value;
        int eval = context_eval_clause(this, clause);
        if (eval > 0) {
            context_remove_clause_from_unsat(this, clause);
        } else if (eval < 0) {
            context_remove_clause_from_unsat(this, clause);
            context_add_false_clause(this, clause);
        }
    }
}

// context_unassign_variable --------------------------------------------------

void context_unassign_variable(context_t* this, size_t variable_index) {

    // update assignment value to 0 in the variable map
    arraymap_t* variables = this->variables; // {unsigned -> variable_t*}
    variable_t* the_variable = (variable_t*) arraymap_get(variables, variable_index);
    linkedlist_destroy(the_variable->deduced_from, NULL, NULL);
    the_variable->deduced_from = linkedlist_create(); // Reset the deduced_from field
    variable_set_raw_value(the_variable, 0); // 0 is the undefined value

    // get the participating clauses of the variable
    linkedlist_t* participating_clauses = the_variable->participatingClauses; // arraylist<clause_t*>

    for (linkedlist_node_t* curr = participating_clauses->head->next; curr != participating_clauses->tail; curr = curr->next) {
        clause_t* a_clause = curr->value;

        // if participating_false_clauses is not NULL, remove the clause from the false_clauses
        // because if it's in the false clauses, the clause is currently false, but removing
        // an assignment will make it undefined
        linkedlist_node_t* participating_false_clause_node = a_clause->participating_false_clauses;
        if (participating_false_clause_node) {
            LOG_DEBUG("\nRemoving false clause:\n");
            clause_print(participating_false_clause_node->value);
            linkedlist_remove_node(this->false_clauses, participating_false_clause_node);
            a_clause->participating_false_clauses = NULL;
        }

        // if participating_unsat is NULL, add this clause to the unsat
        // because this means the clause was well-defined (either T or F), but now
        // undoing a variable assignment makes it undefined again
        linkedlist_node_t* participating_unsat = a_clause->participating_unsat;
        int clause_eval = context_eval_clause(this, a_clause);
        if (!participating_unsat && (clause_eval == 0)) {
            context_add_clause_to_unsat(this, a_clause);
            context_update_clause_literal_purity_counts(this, a_clause, 1);
        }
    }

}

// context_run_bcp ------------------------------------------------------------

static void add_deduced_assignment(context_t* this, int new_assignment, clause_t* current_clause) {
    // Get current assignment level
    linkedlist_t* assignment_history = this->assignment_history; // linkedlist<assignment_level_t*>
    linkedlist_node_t* last_assignment_node = assignment_history->tail->prev;
    if (!last_assignment_node || (last_assignment_node == assignment_history->head)) {
        LOG_FATAL("add_deduced_assignment: Could not find a valid assignment node to operate on\n");
    }
    assignment_level_t* the_assignment_level = (assignment_level_t*) last_assignment_node->value;
    // Add to the assignment level's deduced assignments
    assignment_level_add_deduced_assignment(the_assignment_level, new_assignment);
    // Get the primary assignment of the current level and set as variable's deduction parent
    unsigned current_var_index = abs(new_assignment);
    arraymap_t* variables_map = this->variables; // arraymap<size_t, variable_t*>
    variable_t* current_var_struct = arraymap_get(variables_map, current_var_index);

    // Add the deduced_from links
    if (!current_clause) {
        // If current clause is NULL, then the variable is self-deduced and does not depend
        // on other deductions
        linkedlist_add_last(current_var_struct->deduced_from, (void*) (unsigned long) abs(the_assignment_level->assignment));
    } else {
        // If clauses are not null, then the assignment is deduced from the other variables
        // in the clause
        arrayList_t* current_clause_literals = current_clause->literals; // arraylist<literal_t*>
        for (unsigned i = 0; i < arraylist_size(current_clause_literals); i++) {
            literal_t* a_lit_ptr = arraylist_get(current_clause_literals, i);
            size_t a_var_idx = abs(*a_lit_ptr);
            if (abs(new_assignment) != abs(a_var_idx)) {
                linkedlist_add_last(current_var_struct->deduced_from, (void*) a_var_idx);
            }
        }
    }
}

// Returns true if BCP has made some progress
// Returns false if BCP could not find any singleton-clause to work on
static bool context_run_bcp_once(context_t* this) {
    // get unsatisfied clauses
    linkedlist_t* unit_unsat = this->unit_clauses; // linkedlist<clause_t*>
    if (linkedlist_size(unit_unsat) == 0) {
        return false;
    }

    clause_t* a_clause = (clause_t*) unit_unsat->head->next->value;

    // Found a variable on which to do BCP
    literal_t* literal_ptr = a_clause->an_unassigned_literal;
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
    LOG_DEBUG("BCP deciding to assign %lu to be %d\n", variable_index, new_assignment);
    context_assign_variable_value(this, variable_index, new_assignment);
    add_deduced_assignment(this, literal_value, a_clause);
    return true;
}

// Returns an evaluation value of the formula
int context_run_bcp(context_t* this, unsigned* iterations) {
    while (true) {
        int formula_value = context_evaluate_formula(this);
        LOG_DEBUG("context_run_bcp: Formula is currently true/false? %d\n", formula_value);
        if (formula_value == 0) {
            bool res = context_run_bcp_once(this);
            if (!res) {
                // BCP did not make any progress, return an evaluation
                return context_evaluate_formula(this);
            }
            // Otherwise, continue looping
            *iterations += 1;
        } else {
            // We already know if formula is true or false, return
            return formula_value;
        }
    }
}

// Returns true if PLP has made some progress
// Returns false if PLP could not find any pure literals to work on
static bool context_run_plp_once(context_t* this) {
    arraymap_t* variables = this->variables;
    for (size_t i = 0; i < arraylist_size(variables->arraylist); i++) {
        void* value = arraylist_get(variables->arraylist, i);
        if (!value) {
            continue;
        }
        variable_t* variable = (variable_t*) value;
        if (variable->currentAssignment) { // Variable is assigned, nothing to do.
            continue;
        }
        int var_purity = variable_get_purity(variable);
        if (var_purity != 0) {
            if (var_purity > 0) {
                // Decide that the variable must be set to true, PLP makes progress
                context_assign_variable_value(this, i, true);
                add_deduced_assignment(this, (int) i, NULL);
            } else {
                // Decide that the variable must be set to false, PLP makes progress
                context_assign_variable_value(this, i, false);
                add_deduced_assignment(this, -(int) i, NULL);
            }
            return true;
        }
    }
    // No pure literals found, PLP makes no progress
    return false;
}

// Returns an evaluation value of the formula
int context_run_plp(context_t* this) {
    while(true) {
        int formula_value = context_evaluate_formula(this);
        LOG_DEBUG("context_run_plp: Formula is currently true/false? %d\n", formula_value);
        if (formula_value == 0) {
            bool res = context_run_plp_once(this);
            if (!res) {
                // PLP made no progress, return an evaluation
                return context_evaluate_formula(this);
            }
            // Otherwise, continue looping
        } else {
            // We already know formula result, return
            return formula_value;
        }
    }
}

// context_print_current_state ------------------------------------------------

void context_print_current_state_variable_printer(size_t key, void* value, void* UNUSED(aux)) {
    if (ENABLE_DEBUG) {
        variable_t* the_variable = (variable_t*) value;
        LOG_DEBUG("\t%lu:\t%d\n", key, the_variable->currentAssignment);
    }
}

// clause_list is a linkedlist_t<clause_t*>
void context_print_current_state_print_clause_list(linkedlist_t* clause_list) {
    if (ENABLE_DEBUG) {
        size_t curr_index = 0;
        for (linkedlist_node_t* curr = clause_list->head->next;
            curr != clause_list->tail;
            curr = curr->next) {
            clause_t* elem = (clause_t*) curr->value;
            arrayList_t* clause_literals = elem->literals;
            LOG_DEBUG("Clause %lu: ", curr_index);
            for (size_t j = 0; j < arraylist_size(clause_literals); j++) {
                literal_t* a_literal = (literal_t*) arraylist_get(clause_literals, j);
                LOG_DEBUG("%d\t", *a_literal);
            }
            LOG_DEBUG("\n");
            curr_index++;
        }
    }
}

void context_print_current_state(context_t* this) {
    if (ENABLE_DEBUG) {
        // Clauses of the formula
        LOG_DEBUG("\nFormula clauses:\n");
        arrayList_t* formula = this->formula;
        for (size_t i = 0; i < arraylist_size(formula); i++) {
            clause_t* elem = (clause_t*) arraylist_get(formula, i);
            // Print the literals
            LOG_DEBUG("Clause %lu = ", i);
            arrayList_t* clause_literals = elem->literals;
            for (size_t j = 0; j < arraylist_size(clause_literals); j++) {
                literal_t* a_literal = (literal_t*) arraylist_get(clause_literals, j);
                LOG_DEBUG("%d\t", *a_literal);
            }
            LOG_DEBUG("\n");
        }
        // Variables map
        LOG_DEBUG("\nVariable map:\n");
        arraymap_t* variables = this->variables;
        if (!variables) {
            LOG_DEBUG("is NULL\n");
        } else {
            arraymap_foreach_pair(variables, context_print_current_state_variable_printer, NULL);
        }
        // Unsatisfied clauses
        LOG_DEBUG("\nUnsat clauses:\n");
        context_print_current_state_print_clause_list(this->unsat);
        // False clauses
        LOG_DEBUG("\nFalse clauses:\n");
        context_print_current_state_print_clause_list(this->false_clauses);

        LOG_DEBUG("\nSorted indices:\n");
        for(size_t i = 0; i < arraylist_size(this->variables->arraylist) - 1; i++) {
            LOG_DEBUG("Elem %ld -> literal %ld\n", i, this->sorted_indices[i]);
        }
        LOG_DEBUG("\n");
    }
}

void context_print_result_variables(const context_t* ctx) {
    arrayList_t* variableArray = ctx->variables->arraylist;
    const size_t listSize = arraylist_size(variableArray);
    for(size_t i = 0; i < listSize; i++) {
        variable_t* var = (variable_t*) arraylist_get(variableArray, i);
        if(!var) {
            continue;
        }
        // Since this variable is assigned to 0 we could assign it to either -1 or 1
        // let's just assign it to 1
        int assignment = !var->currentAssignment ? 1 : var->currentAssignment;
        printf("%d", assignment * (int) i);
        if(i != (listSize - 1)) {
            printf(" ");
        }
    }
    printf("\n");
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
    // The sorted_indices array is full of 0s if empty, so if there is no first variable, 0 will be returned.
    return this->sorted_indices[0];
}

// context_get_next_variable_index --------------------------------------------
// Maybe improvement: store the last index and then start from there,
// if that yields nothing start from 0?
static size_t context_find_next_sorted_index(context_t* this) {
    static size_t previousIndex = 0; // Store the index of the last variable we decided and start from there
    for(size_t i = previousIndex; i < this->numVariables; i++) {
        const size_t key = this->sorted_indices[i];
        const variable_t* next_variable_data = (variable_t*) arraymap_get(this->variables, key);
        // Somehow the key points to an invalid variable (?)
        if(!next_variable_data) {
            LOG_FATAL("%s:%lu: key %lu pointed to a NULL variable!\n", __FILE__, __LINE__, key);
            break;
        }
        if(!next_variable_data->currentAssignment) {
            previousIndex = i;
            return key;
        }
    }

    // We couldn't find anything starting from the previous index, so we just start from 0
    for(size_t i = 0; i < previousIndex; i++) {
        const size_t key = this->sorted_indices[i];
        const variable_t* next_variable_data = (variable_t*) arraymap_get(this->variables, key);
        // Somehow the key points to an invalid variable (?)
        if(!next_variable_data) {
            LOG_FATAL("%s:%lu: key %lu pointed to a NULL variable!\n", __FILE__, __LINE__, key);
            break;
        }
        if(!next_variable_data->currentAssignment) {
            previousIndex = i;
            return key;
        }
    }
    // Nothing is left to be assigned
    return 0;
}

size_t context_get_next_unassigned_variable(context_t* this) {
    return context_find_next_sorted_index(this);
}


static void merge(size_t* arr, size_t l, size_t m, size_t r, arraymap_t* variables)
{
    size_t i, j, k;
    const size_t n1 = m - l + 1; // How big is the left part of the array?
    const size_t n2 =  r - m; // and the right?

    size_t *L = malloc(n1 * sizeof(size_t)), *R = malloc(sizeof(size_t)*n2); // TEMP

    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2)
    {
        // -1 is needed because of find_next_entry finding the next entry and not the closest one
        const arraymap_pair_t v1 = arraymap_find_next_entry(variables, L[i]-1);
        const arraymap_pair_t v2 = arraymap_find_next_entry(variables, R[j]-1);
        // The unchecked cast is possible because of sorted_indexes being built from variable,
        // therefore if a literal is there it must exist
        const size_t length1 = linkedlist_size(((variable_t*)v1.v)->participatingClauses);
        const size_t length2 = linkedlist_size(((variable_t*)v2.v)->participatingClauses);
        // Descending
        if (length1 >= length2)
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    /* Copy the remaining elements of L[], if there
       are any */
    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Same for R
    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
    free(L);
    free(R);
}

void mergeSort(size_t *arr, size_t l, size_t r, arraymap_t* variables)
{
    if (l < r)
    {
        size_t m = l+(r-l)/2;

        mergeSort(arr, l, m, variables);
        mergeSort(arr, m+1, r, variables);

        merge(arr, l, m, r, variables);
    }
}

// context_get_clauses_count --------------------------------------------------

unsigned context_get_conflicts_count(context_t* this) {
    return linkedlist_size(this->conflicts);
}

// context_get_first_false_clause ---------------------------------------------

clause_t* context_get_first_false_clause(context_t* this) {
    linkedlist_t* false_clauses = this->false_clauses; // linkedlist<clause_t*>
    if (linkedlist_size(false_clauses) == 0) {
        LOG_DEBUG("context_get_first_false_clause: False clauses list is empty\n");
        abort();
    }

    return (clause_t*) false_clauses->head->next->value;
}

// context_get_primary_assignment_of ------------------------------------------

// acc is linkedlist_t<int> is the result accumulator
void context_get_primary_assignment_of(context_t* this, unsigned query_variable_index, linkedlist_t* acc) {
    arraymap_t* variables = this->variables; // arraymap<size_t, variable_t*>
    variable_t* query_variable = arraymap_get(variables, query_variable_index);
    if (!query_variable) {
        LOG_DEBUG("context_get_primary_assignment_of: Could not find variable %u from variables map\n", query_variable_index);
        abort();
    }
    linkedlist_t* deduction_parent_variables = query_variable->deduced_from; // linkedlist<size_t>
    if (linkedlist_size(deduction_parent_variables) == 0) {
        LOG_DEBUG("context_get_primary_assignment_of: The deduced_from of variable %u is length 0. This should never happen because even if the variable is an arbitrary assignment it should be deduced from itself.\n", query_variable_index);
        abort();
    }

    // For each of the parent links, add to the result if it's a primary assignment, otherwise
    // go up the tree recursively
    for (linkedlist_node_t* curr = deduction_parent_variables->head->next; curr != deduction_parent_variables->tail; curr = curr->next) {
        size_t a_parent_idx = (size_t) curr->value;
        variable_t* parent_variable = arraymap_get(variables, a_parent_idx);
        int parent_value = parent_variable->currentAssignment;
        if (parent_value == 0) {
            LOG_DEBUG("context_get_primary_assignment_of: The deduction parent is currently unassigned. But a parent of a deducted variable must have an assignment in order to be the parent. Query variable = [%u], Parent variable = [%lu]\n", query_variable_index, a_parent_idx);
            abort();
        }

        // If the parent index is the same as the query variable, we have found a root assignment and
        // we add it to the accumulator
        if (a_parent_idx == query_variable_index) {
            int result = parent_value > 0 ? a_parent_idx : -a_parent_idx;
            linkedlist_add_last(acc, (void*) (long) result);
            return;
        } else {
            // Otherwise we recursively call and find parents
            context_get_primary_assignment_of(this, a_parent_idx, acc);
            return;
        }
    }

}
