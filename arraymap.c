#include "arraymap.h"

arraymap_t* arraymap_create() {
    arraymap_t* ret = malloc(sizeof(arraymap_t));
    ret->arraylist = arraylist_create();
    return ret;
}

void arraymap_put(arraymap_t* this, size_t key, void* value) {
    arrayList_t* arraylist = this->arraylist;
    // Bring arraylist up to size
    while (arraylist_size(arraylist) <= key) {
        arraylist_insert(arraylist, NULL);
    }
    // Insert item
    arraylist_set(arraylist, key, value);
}

void* arraymap_get(arraymap_t* this, size_t key) {
    size_t size = arraylist_size(this->arraylist);
    if (key < size) {
        return arraylist_get(this->arraylist, key);
    } else {
        return NULL;
    }
}

// ForEach pair ---------------------------------------------------------------

typedef struct arraymap_foreach_pair_data {
    size_t counter;
    arraymap_pair_consumer f;
    void* payload_aux;
} arraymap_foreach_pair_data_t;

static void arraymap_foreach_pair_consumer(void* element, void* aux) {
    arraymap_foreach_pair_data_t* data = (arraymap_foreach_pair_data_t*) aux;
    data->f(data->counter, element, data->payload_aux);
    data->counter++;
}

void arraymap_foreach_pair(arraymap_t* this, arraymap_pair_consumer consumer, void* aux) {
    arrayList_t* arraylist = this->arraylist;
    arraymap_foreach_pair_data_t data;
    data.counter = 0;
    data.f = consumer;
    data.payload_aux = aux;
    arraylist_foreach(arraylist, arraymap_foreach_pair_consumer, &data);
}

// Destroy --------------------------------------------------------------------

void arraymap_destroy(arraymap_t* this, arraymap_pair_consumer destroyer, void* aux) {
    // Destroy each of the elements
    arraymap_foreach_pair(this, destroyer, aux);
    // Free the arraylist
    arraylist_destroy(this->arraylist, NULL, NULL);
}