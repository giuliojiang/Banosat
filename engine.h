#ifndef SAT_ENGINE_H
#define SAT_ENGINE_H

#include <stdbool.h>

#include "context.h"

static const unsigned MAX_CONFLICT_CLAUSES = 32;

bool engine_run_solver(context_t* context);

#endif
