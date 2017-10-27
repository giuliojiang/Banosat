
#include "../macros.h"
#include "../hashset.h"

unsigned hashser(void* input) {
    unsigned val = (unsigned) (long) input;
    return (val * 229) % 613;
}

bool equaler(void* a, void* b) {
    unsigned val_a = (unsigned) (long) a;
    unsigned val_b = (unsigned) (long) b;
    return val_a == val_b;
}

int main(int UNUSED(argc), char** UNUSED(argv)) {
    hashset_t* set = hashset_create(hashser, equaler);

    // Put 1, 2, 3
    hashset_add(set, (void*) 1);
    hashset_add(set, (void*) 2);
    hashset_add(set, (void*) 3);

    // Test that 1, 2, 3 are in the set
    for (unsigned i = 0; i < 10; i++) {
        printf("Element %u is in the set %d\n", i, hashset_contains(set, (void*) (long) i));
    }

    hashset_destroy(set, NULL, NULL);
}
