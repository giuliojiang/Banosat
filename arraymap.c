#include "arraymap.h"

// Create ---------------------------------------------------------------------

arraymap_t* arraymap_create() {
    arraymap_t* ret = malloc(sizeof(arraymap_t));
    ret->arraylist = arraylist_create();
    return ret;
}

// Put ------------------------------------------------------------------------

void arraymap_put(arraymap_t* this, size_t key, void* value) {
    arrayList_t* arraylist = this->arraylist;
    // Bring arraylist up to size
    while (arraylist_size(arraylist) <= key) {
        arraylist_insert(arraylist, NULL);
    }
    // Insert item
    arraylist_set(arraylist, key, value);
}

// Get ------------------------------------------------------------------------

void* arraymap_get(arraymap_t* this, size_t key) {
    size_t size = arraylist_size(this->arraylist);
    if (key < size) {
        return arraylist_get(this->arraylist, key);
    } else {
        return NULL;
    }
}

// ForEach pair ---------------------------------------------------------------

void arraymap_foreach_pair(arraymap_t* this, arraymap_pair_consumer consumer, void* aux) {
    if (!consumer) {
        return;
    }
    
    for (size_t i = 0; i < arraylist_size(this->arraylist); i++) {
        void* value = arraylist_get(this->arraylist, i);
        if (value) {
            consumer(i, value, aux);
        }
    }
}

// Destroy --------------------------------------------------------------------

void arraymap_destroy(arraymap_t* this, arraymap_pair_consumer destroyer, void* aux) {
    // Destroy each of the elements
    arraymap_foreach_pair(this, destroyer, aux);
    // Free the arraylist
    arraylist_destroy(this->arraylist, NULL, NULL);
    // Free this
    free(this);
}

// First element --------------------------------------------------------------

// Returns the first valid pair in the arraymap
// Returns an empty pair (with a NULL pointer in return.v) if no first element is found
arraymap_pair_t arraymap_find_first_entry(arraymap_t* this) {
    return arraymap_find_next_entry(this, -1);
}

// Next element ---------------------------------------------------------------

// Returns next valid pair in the arraymap
// Returns an empty pair (with a NULL pointer in return.v) if no first element is found
arraymap_pair_t arraymap_find_next_entry(arraymap_t* this, size_t curr_index) {
    for (size_t i = (curr_index + 1); i < arraylist_size(this->arraylist); i++) {
        void* value = arraylist_get(this->arraylist, i);
        if (value) {
            arraymap_pair_t result;
            result.k = i;
            result.v = value;
            return result;
        }
    }
    arraymap_pair_t empty_result;
    empty_result.k = 0;
    empty_result.v = NULL;
    return empty_result;
}
