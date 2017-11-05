#include "engine.h"

#include "stdio.h"

#include "literal.h"
#include "decide.h"
#include "hashset.h"

static void engine_create_conflict_clause(context_t* context) {
    // Check current number of clauses
    unsigned total_clauses_count = context_get_clauses_count(context);
    if (total_clauses_count > MAX_CLAUSES_COUNT) {
        LOG_DEBUG("Maximum number of clauses reached, not adding this conflict clause...\n");
        return;
    }

    // Get the first false clause
    clause_t* false_clause = context_get_first_false_clause(context);
    if (!false_clause) {
        LOG_DEBUG("engine_create_conflict_clause: no false clause retrieved despite the formula beging false. Aborting...");
        abort();
    }

    // Get all the variables of the false clause
    clause_print(false_clause);
    arrayList_t* false_clause_literals = false_clause->literals; // arraylist<literal_t*>

    // Create the conflict clause
    hashset_t* seen_literals = hashset_create(literal_hash_func, literal_equal_func); // hashset<signed>
    clause_t* conflict_clause = clause_create();
    for (unsigned i = 0; i < arraylist_size(false_clause_literals); i++) {
        literal_t* a_literal_ptr = arraylist_get(false_clause_literals, i);
        unsigned query_variable_index = abs(*a_literal_ptr);
        linkedlist_t* primary_assignment_variables = linkedlist_create(); // linkedlist<int>
        context_get_primary_assignment_of(context, query_variable_index, primary_assignment_variables);
        for (linkedlist_node_t* curr = primary_assignment_variables->head->next; curr != primary_assignment_variables->tail; curr = curr->next) {
            int a_primary_parent = (int) (long) curr->value;
            bool add_res = clause_add_literal(conflict_clause, -a_primary_parent, seen_literals);
            if (!add_res) {
                // Signal that this clause should not be added to the context as it's always true
                // But then if the conflict clause is always true it's a contradiction of the current
                // state of the formula itself, therefore we abort execution as it's a fatal error
                LOG_DEBUG("engine_create_conflict_clause: Generation of a conflict clause generated a clause which is always True...");
                LOG_DEBUG("Generated from false clause:\n");
                clause_print(false_clause);
                abort();
            }
        }
        linkedlist_destroy(primary_assignment_variables, NULL, NULL);
    }
    hashset_destroy(seen_literals, NULL, NULL);

    // Add the conflict clause to the context
    context_add_clause(context, conflict_clause);
}

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
            // Attempt creating a conflict clause
            engine_create_conflict_clause(context);
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
