#include "decide.h"

#include "stdio.h"

#include "context.h"

// decide_new_decision --------------------------------------------------------

int decide_make_new_decision(context_t* context) {
    // Get last assigned level
    assignment_level_t* last_level = context_get_last_assignment_level(context);

    // If the last level is NULL, all possibilities have been exhausted
    if (!last_level) {
        return 0;
    }

    // Otherwise, if the previous assignment was True, backtrack 1 level and make a false assignment
    int last_assignment = last_level->assignment;
    if (last_assignment > 0) {
        decide_backtrack_one_level(context);
        context_apply_new_decision_level(context, abs(last_assignment), false);
        return -abs(last_assignment);
    } else {
        // If the previous assignment was False, undo the current level and recurse with shallower level
        decide_backtrack_one_level(context);
        return decide_make_new_decision(context);
    }
}

// decide_step_up_decision ----------------------------------------------------

int decide_step_up_decision(context_t* context) {
    // Get last assigned level
    assignment_level_t* last_level = context_get_last_assignment_level(context);
    if (!last_level) {
        // First run of decision, get the first variable
        size_t first_variable_index = context_get_first_variable_index(context);
        context_apply_new_decision_level(context, first_variable_index, true);
        return (int) first_variable_index;
    }
    size_t last_variable_index = abs(last_level->assignment);
    // Get next variable index
    size_t next_variable_index = context_get_next_variable_index(context, last_variable_index);
    if (next_variable_index == 0) {
        // Reached the end of the tree
        // This is an ill situation because it means that all variables were assigned,
        // But formula is still inconclusive
        // I am aborting here as I feel we should investigate about what happened in this case
        LOG_FATAL("decide_step_up_decision: could not find any more variables to be assigned. However this function should only be called if the formula was inconclusive. How can it be inconclusive if all variables have already been assigned?");
        return 0;
    }
    // Create next level
    context_apply_new_decision_level(context, next_variable_index, true);
    return (int) next_variable_index;
}

// decide_backtrack_to --------------------------------------------------------

void decide_backtrack_one_level(context_t* context) {
    // Get the last assignment level
    assignment_level_t* last_assignment_level = context_remove_last_assignment_level(context);
    if (!last_assignment_level) {
        return;
    }
    // Undo the derived assignments
    linkedlist_t* deduced_assignments = last_assignment_level->deduced_assignments; // linkedlist<int>
    for (linkedlist_node_t* curr = deduced_assignments->head->next; curr != deduced_assignments->tail; curr = curr->next) {
        int elem = (long) curr->value;
        // Undo the assignment
        context_unassign_variable(context, abs(elem));
    }
    // Undo the primary assignment
    context_unassign_variable(context, abs(last_assignment_level->assignment));
    // Destroy the assignment level
    assignment_level_destroy(last_assignment_level);
}

void decide_backtrack_to(context_t* context, int target_level) {
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
