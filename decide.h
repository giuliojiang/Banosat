#ifndef SAT_DECIDE_H
#define SAT_DECIDE_H

#include "context.h"

// Go to the next enumerated possibility.
// If the last explored node was a True, branches off to the False node on same level
// If the last explored node was a False, step down a level (to the parent) and recurse
// To be used if the previous assigned caused the formula to become False.
// Returns a signed integer, where the absolute value is the variable index
// and the sign is the new value to be assigned
// Returns 0 if no new decision is possible
// Updates the decision history, but does NOT assign the new value in the formula
int decide_make_new_decision(context_t* context);

// Assign the next variable to a value
// To be used if the previous assignment was inconclusive for satisfiability
// Returns a signed integer, where the absolute value is the variable index
// and the sign is the new value to be
// Returns 0 if no new decision is possible
// Updates the decision history, but does NOT assign the new value in the formula
int decide_step_up_decision(context_t* context);

void decide_backtrack_one_level(context_t* context);

// Backtracks to <target_level>, without undoing target_level
void decide_backtrack_to(context_t* context, int target_level);

#endif
