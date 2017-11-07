// TODO: use for more definitions

#ifndef SAT_LITERAL_H
#define SAT_LITERAL_H

#include <stdbool.h>

typedef signed literal_t;

// Hash equal and hash functions ----------------------------------------------

unsigned literal_hash_func(void* input);

bool literal_equal_func(void* pa, void* pb);

#endif //SAT_LITERAL_H
