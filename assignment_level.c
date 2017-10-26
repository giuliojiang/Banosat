#include "assignment_level.h"

#include <stdlib.h>

// Create ---------------------------------------------------------------------

assignment_level_t* assignment_level_create(int level, int assignment) {
    assignment_level_t* result = malloc(sizeof(assignment_level_t));
    result->level = level;
    result->assignment = assignment;
    result->deduced_assignments = linkedlist_create();
    return result;
}

// Insert an assignment -------------------------------------------------------

void assignment_level_add_deduced_assignment(assignment_level_t* this, int new_assignment) {
    linkedlist_t* deduced_assignments = this->deduced_assignments;
    linkedlist_add_last(deduced_assignments, (void*) (long) new_assignment);
}

// Destroy --------------------------------------------------------------------

void assignment_level_destroy(assignment_level_t* this) {
    linkedlist_destroy(this->deduced_assignments, NULL, NULL);
    free(this);
}
