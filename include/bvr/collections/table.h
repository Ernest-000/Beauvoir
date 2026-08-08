#pragma once

#include <bvr/config.h>

#include <bvr/collections/string.h>

/**
 * @brief loop through each element of a table. 
 * Table for loops can be use as such ```struct client_s* client; BVR_TABLE_FOR_EACH(table, client) {}```.
 * @param table the table object to iterator through.
 * @param value a pointer that will pointing to the current looping element.
 */
#define BVR_TABLE_FOR_EACH(table, value) \
    struct bvr_table_iterator_s _iterator = {.table = &(table), .index = 0}; \
    while (((value) = bvr_table_iterate(&_iterator)))

struct bvr_table_chunk_s {
    // hashed key
    uint32 key;

    // for padding purpuse
    // and internal flags
    uint32 reserved;

    // the rest of the chunck follows
    // void* value;
};

typedef struct bvr_table_s {
    struct bvr_table_chunk_s* entries;
    
    uint32 capacity;
    uint32 count;

    uint32 elemsize;
    uint32 chunck_size;
} bvr_table_t;


struct bvr_table_iterator_s {
    bvr_table_t* table;

    uint32 index;
    struct bvr_table_chunk_s* current;
};

/**
 * @brief create a new table.
 * @param elemsize the size of each of the table's entries.
 * @param capacity the initial table empty slot allocated.
 */
int bvr_create_table(bvr_table_t* table, const uint32 elemsize, const uint32 capacity);

/**
 * @brief return an element from the table by using it's key.
 * @param key the key of the element.
 * @returns returns a pointer to the entry's value.
 */
void* bvr_table_get(bvr_table_t* table, const char* key);

/**
 * @brief set an element of the table, if the key cannot be found, the element is added.
 * @param key the key of the element
 * @param value the new value of the element
 * @returns returns a pointer to the entry's value.
 */
void* bvr_table_set(bvr_table_t* table, const char* key, void* value);

/**
 * @brief add an element to the table, if the key is already here, it will be no-op.
 * @param key the key of the element.
 * @param flag entry's optional flag.
 * @returns returns a pointer to the entry's value.
 */
void* bvr_table_add(bvr_table_t* table, const char* key, uint32 flag);

void bvr_destroy_table(bvr_table_t* table);

/**
 * @brief iterate a step in a table for each loop.
 */
void* bvr_table_iterate(struct bvr_table_iterator_s* iterator);