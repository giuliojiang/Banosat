#include "literal.h"

#include <stdlib.h>

// input will be cast to a signed
unsigned literal_hash_func(void* input) {
    signed literal = (signed) (long) input;
    unsigned result = abs(literal);
    return result;
}

// pa and pb are considered to be of type signed
bool literal_equal_func(void* pa, void* pb) {
    return pa == pb;
}
