#include "context.h"

// decide_new_decision --------------------------------------------------------

// Returns a signed integer, where the absolute value is the variable index
// and the sign is the new value to be assigned
// Returns 0 if no new decision is possible
// Updates the decision history, but does NOT assign the new value in the formula
int decide_make_new_decision(context_t* context) {
    // Get last assigned level
    assignment_level_t* last_level = context_get_last_assignment_level(context);

    // If the last level is NULL, create an assignment for the first variable
    if (!last_level) {
        size_t first_variable_index = context_get_first_variable_index(context);
        if (first_variable_index == 0) {
            // No first variable -> empty formula?
            return 0;
        }
        context_apply_new_decision_level(context, first_variable_index, first_variable_index > 0);
        return first_variable_index; // Return first variable = T
    }

    // Otherwise, if the previous assignment was True, make it False
    int last_assignment = last_level->assignment;
    if (last_assignment > 0) {
        last_level->assignment = -last_assignment;
        return -last_assignment;
    }

    // If the previous assignment was False, get the next variable and make it True
    size_t next_variable_index = context_get_next_variable_index(context, abs(last_assignment));
    // If this was the last variable, return 0
    if (next_variable_index == 0) {
        return 0;
    }
    // Create a new level with the new variable
    context_apply_new_decision_level(context, next_variable_index, true);
    return next_variable_index;
}

// decide_backtrack_to --------------------------------------------------------

void decide_backtrack_one_level(context_t* context) {
    // Get the last assignment level
    assignment_level_t* last_assignment_level = context_remove_last_assignment_level(context);
    if (!last_assignment_level) {
        return;
    }
    // Undo the derived assignments
    linkedlist_t* deduced_assignments = last_assignment_level->deduced_assignments;
    for (linkedlist_node_t* curr = deduced_assignments->head->next; curr != deduced_assignments->tail; curr = curr->next) {
        int elem = (int) curr->value;
        // Undo the assignment
        context_unassign_variable(context, abs(elem));
    }
    // Undo the primary assignment
    context_unassign_variable(context, abs(last_assignment_level->assignment));
    // Destroy the assignment level
    assignment_level_destroy(last_assignment_level);
}

// Backtracks to <target_level>, without undoing target_level
void decide_backtrack_to(context_t* context, size_t target_level) {
    while (true) {
        // Get the last assignment level
        assignment_level_t* last_assignment_level = context_get_last_assignment_level(context);
        if (!last_assignment_level || last_assignment_level->level <= target_level) {
            // Already reached desired level
            return;
        }
        decide_backtrack_one_level(context);
    }
}
