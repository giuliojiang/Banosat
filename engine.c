#include "engine.h"

#include "stdio.h"

#include "decide.h"

bool engine_run_solver(context_t* context) {
    int decision = 0;
    // Decide
    decision = decide_step_up_decision(context);
    LOG_DEBUG("\nengine_run_solver: Decision step initial: decided %d\n", decision);
    while (true) {
        if (decision == 0) {
            return false;
        }
        // Assign to the formula
        LOG_DEBUG("\nengine_run_solver: Assigning decision to the formula\n");
        context_assign_variable_value(context, abs(decision), decision > 0);
        context_print_current_state(context);
        // BCP
        LOG_DEBUG("\nengine_run_solver: Running BCP...\n");
        context_run_bcp(context);
        context_print_current_state(context);
        // PLP
        LOG_DEBUG("\nengine_run_solver: Running PLP...\n");
        context_run_plp(context);
        context_print_current_state(context);
        // Check state
        int formula_eval = context_evaluate_formula(context);
        if (formula_eval > 0) {
            // SAT
            return true;
        } else if (formula_eval < 0) {
            // Formula is false, backtrack
            LOG_DEBUG("\nengine.c: decide with\n");
            context_print_current_state(context);
            decision = decide_make_new_decision(context);
            LOG_DEBUG("\nengine_run_solver: Decision step backtrack: decided %d\n", decision);
        } else {
            // Formula is inconclusive, step up a level
            LOG_DEBUG("\nengine.c: decide with\n");
            context_print_current_state(context);
            decision = decide_step_up_decision(context);
            LOG_DEBUG("\nengine_run_solver: Decision step step up: decided %d\n", decision);
        }
    }
}
