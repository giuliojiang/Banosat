#ifndef SAT_DECIDE_H
#define SAT_DECIDE_H

int decide_make_new_decision(context_t* context);

void decide_backtrack_one_level(context_t* context);

void decide_backtrack_to(context_t* context, size_t target_level);

#endif
