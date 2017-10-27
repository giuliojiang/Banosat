#include "hashset.h"

// Helper functions -----------------------------------------------------------

static bool hashset_bucket_contains(hashset_t* this, linkedlist_t* bucket, void* elem);

// Computes the bucket index
static unsigned hashset_compute_bucket_index(hashset_t* this, void* elem) {
    hashset_hash_func hash_func = this->hash_func;
    return hash_func(elem) % HASHSET_NUM_BUCKETS;
}

// Returns the bucket linkedlist<void*> given bucket index
static linkedlist_t* hashset_find_bucket(hashset_t* this, unsigned bucket_index) {
    arraymap_t* buckets = this->buckets; // arraymap<unsigned, linkedlist_t*<void*>>
    return arraymap_get(buckets, bucket_index);
}

// Returns the bucket linkedlist<void*> if existing, otherwise creates it and returns it
static linkedlist_t* hashset_find_or_create_bucket(hashset_t* this, unsigned bucket_index) {
    linkedlist_t* bucket = hashset_find_bucket(this, bucket_index);
    if (bucket) {
        return bucket;
    } else {
        bucket = linkedlist_create();
        arraymap_t* buckets = this->buckets; // arraymap<unsigned, linkedlist_t*<void*>>
        arraymap_put(buckets, bucket_index, (void*) bucket);
        return bucket;
    }
}

// hashset_create -------------------------------------------------------------

hashset_t* hashset_create(hashset_hash_func hash_function, hashset_equal_func equal_function) {
    // malloc
    hashset_t* result = malloc(sizeof(hashset_t));
    // Initialize elements
    result->hash_func = hash_function;
    result->equal_func = equal_function;
    result->buckets = arraymap_create();

    return result;
}

// hashset_add ----------------------------------------------------------------

void hashset_add(hashset_t* this, void* elem) {
    unsigned bucket_index = hashset_compute_bucket_index(this, elem);
    linkedlist_t* bucket = hashset_find_or_create_bucket(this, bucket_index); // linkedlist<void*>
    bool already_present = hashset_bucket_contains(this, bucket, elem);
    if (!already_present) {
        linkedlist_add_last(bucket, elem);
    }
}

// hashset_contains -----------------------------------------------------------

// Arg <bucket>: linkedlist_t*<void*>
static bool hashset_bucket_contains(hashset_t* this, linkedlist_t* bucket, void* elem) {
    hashset_equal_func equal_func = this->equal_func;
    for (linkedlist_node_t* curr = bucket->head->next; curr != bucket->tail; curr = curr->next) {
        void* curr_elem = curr->value;
        if (equal_func(curr_elem, elem)) {
            return true;
        }
    }
    return false;
}

bool hashset_contains(hashset_t* this, void* elem) {
    // Compute hashcode
    unsigned bucket_index = hashset_compute_bucket_index(this, elem);
    // Get the corresponding bucket
    linkedlist_t* bucket = hashset_find_bucket(this, bucket_index);
    // Check bucket
    if (!bucket) {
        return false;
    } else {
        return hashset_bucket_contains(this, bucket, elem);
    }
}

// hashset_destroy ------------------------------------------------------------

void hashset_destroy(hashset_t* this, hashset_free_func free_func, void* aux) {
    // Get the buckets
    arraymap_t* buckets = this->buckets; // arraymap<unsigned, linkedlist_t*<void*>>
    // Free each bucket
    for (size_t i = 0; i < arraylist_size(buckets->arraylist); i++) {
        void* value = arraylist_get(buckets->arraylist, i);
        if (value) {
            linkedlist_t* bucket = (linkedlist_t*) value; // linkedlist<void*>
            // Free linkedlist contents
            if (free_func) {
                for (linkedlist_node_t* curr = bucket->head->next; curr != bucket->tail; curr = curr->next) {
                    void* linkedlist_elem = curr->value;
                    free_func(linkedlist_elem, aux);
                }
            }
            // Free linkedlist
            linkedlist_destroy(bucket, NULL, NULL);
        }
    }
    // Free the arraymap
    arraymap_destroy(buckets, NULL, NULL);
    // Free the hashset
    free(this);
}
