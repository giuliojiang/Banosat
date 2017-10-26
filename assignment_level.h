#ifndef SAT_ASSIGNMENT_LEVEL_H
#define SAT_ASSIGNMENT_LEVEL_H

#include "linkedlist.h"

typedef struct assignment_level {
    int level; // The level number
    int assignment; // abs value is variable index, sign is value
    linkedlist_t* deduced_assignments; // linkedlist<int> more assignments deduced by BCP or PLP
} assignment_level_t;

assignment_level_t* assignment_level_create(int level, int assignment);

void assignment_level_add_deduced_assignment(assignment_level_t* this, int new_assignment);

void assignment_level_destroy(assignment_level_t* this);

#endif
