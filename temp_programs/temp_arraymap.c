#include "../arraymap.h"
#include <stdio.h>
#include <stdlib.h>

static void arraymap_print_consumer(size_t key, void* value, void* aux) {
    printf("[%ld] [%ld] [%ld]\n", key, value, aux);
}

int main(int argc, char** argv) {
    // Create arraymap
    arraymap_t* the_map = arraymap_create();
    // Add 10 - 100
    arraymap_put(the_map, 10, (void*) 100);
    // Add 11 - 110
    arraymap_put(the_map, 11, (void*) 110);
    // Add 5 - 50
    arraymap_put(the_map, 5, (void*) 50);
    // Print everything
    arraymap_foreach_pair(the_map, arraymap_print_consumer, 666);
    // Free
    arraymap_destroy(the_map, NULL, NULL);
    return 0;
}
