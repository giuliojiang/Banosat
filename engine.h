#ifndef SAT_ENGINE_H
#define SAT_ENGINE_H

#include <stdbool.h>

#include "context.h"

static const unsigned MAX_CLAUSES_COUNT = 2048;

bool engine_run_solver(context_t* context);

#endif
