#include "context.h"

// decide_new_decision

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
