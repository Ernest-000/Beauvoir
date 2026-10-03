#include <bvr/collections/table.h>
#include <bvr/collections/buffer.h>
#include <bvr/common.h>
#include <bvr/math.h>
#include <bvr/io.h>

#include <malloc.h>

// #define BVRI_TABLE_GET_CHUNK_AT(table, i)((table)->entries + i * (table->chunck_size))
// #define BVRI_TABLE_GET_VALUE_AT(table, i)(BVRI_TABLE_GET_CHUNK_AT(table, i) + sizeof(struct bvr_table_chunk_s))

// fucking char* index, I always forget that C arrays are offset by the pointer's type size!!

#define BVRI_TABLE_GET_CHUNK_AT(table, i) \
    ((struct bvr_table_chunk_s*)((char*)(table)->entries + (i) * (table)->chunck_size))

#define BVRI_TABLE_GET_VALUE_AT(table, i) \
    ((char*)BVRI_TABLE_GET_CHUNK_AT(table, i) + sizeof(struct bvr_table_chunk_s))

static void bvri_table_grow(bvr_table_t* table);
static void* bvri_table_set(bvr_table_t* table, const char* key, const uint32 hash, void* data, uint32 reserved);

int bvr_create_table(bvr_table_t* table, const uint32 elemsize, const uint32 capacity){
    BVR_ASSERT(table);

    table->count = 0;
    table->capacity = (uint32)npow2(MAX(1, capacity));  // avoid 0
    table->elemsize = elemsize;
    table->chunck_size = sizeof(struct bvr_table_chunk_s) + elemsize;

    table->entries = calloc(table->capacity, table->chunck_size);
    BVR_ASSERT(table->entries);

    return BVR_TRUE;
}

void* bvr_table_get(bvr_table_t* table, const char* key){
    BVR_ASSERT(table);
    BVR_ASSERT(key);

    uint32 hash = bvr_hash(key);
    uint32 index = (hash & (uint32)(table->capacity - 1));

    while (BVRI_TABLE_GET_CHUNK_AT(table, index)->key != 0)
    {
        if(BVRI_TABLE_GET_CHUNK_AT(table, index)->key == hash){
            // returns a pointer to the chunck data
            return BVRI_TABLE_GET_VALUE_AT(table, index);
        }

        index = index + 1 < table->capacity ? index + 1 : 0; 
    }

    return NULL;
}

void* bvr_table_set(bvr_table_t* table, const char* key, void* value){
    BVR_ASSERT(table);
    BVR_ASSERT(key);

    if(table->count >= table->capacity / BVR_GROWTH_FACTOR){
        // expend
        bvri_table_grow(table);
    }

    return bvri_table_set(table, key, 0, value, 0);
}

void* bvr_table_add(bvr_table_t* table, const char* key, uint32 flag){
    BVR_ASSERT(table);
    BVR_ASSERT(key);

    if(table->count >= table->capacity / BVR_GROWTH_FACTOR){
        // expend
        bvri_table_grow(table);
    }

    return bvri_table_set(table, key, 0, NULL, flag);
}

void bvr_destroy_table(bvr_table_t* table){
    free(table->entries);
    table->entries = NULL;
}

void* bvr_table_iterate(struct bvr_table_iterator_s* iterator){
    BVR_ASSERT(iterator);
    BVR_ASSERT(iterator->table);

    while (iterator->index < iterator->table->capacity)
    {
        struct bvr_table_chunk_s* chunk = BVRI_TABLE_GET_CHUNK_AT(iterator->table, iterator->index);
        iterator->index++;

        if(chunk->key != 0){
            iterator->current = chunk;
            return (char*)chunk + sizeof(struct bvr_table_chunk_s);
        }
    }

    iterator->current = NULL;
    return NULL;
}

static void bvri_table_grow(bvr_table_t* table){
    BVR_ASSERT(table);

    uint32 n_capacity = table->capacity * BVR_GROWTH_FACTOR;
    if(n_capacity <= table->capacity){
        n_capacity = MAX(1, table->capacity * 2);
    }

    struct bvr_table_chunk_s* old_entries = table->entries;
    uint32 old_capacity = table->capacity;
    uint32 old_chunck_size = table->chunck_size;

    struct bvr_table_chunk_s* new_entries = calloc(n_capacity, table->chunck_size);
    BVR_ASSERT(new_entries);

    table->entries = new_entries;
    table->capacity = n_capacity;
    table->count = 0;

    if(old_entries){
        for (size_t i = 0; i < old_capacity; i++)
        {
            struct bvr_table_chunk_s* chunk = (struct bvr_table_chunk_s*)((char*)old_entries + i * old_chunck_size);

            if(chunk->key != 0){
                bvri_table_set(
                    table,
                    NULL,
                    chunk->key,
                    (char*)chunk + sizeof(struct bvr_table_chunk_s),
                    chunk->reserved
                );
            }
        }
    }

    free(old_entries);
}

static void* bvri_table_set(bvr_table_t* table, const char* key, const uint32 _hash, void* data, uint32 reserved){
    // no assert, values shall be previously checked

    uint32 hash = key != NULL ? bvr_hash(key) : _hash;
    uint32 index = (hash & (uint32)(table->capacity - 1));

    // try to find an available chunk
    while (BVRI_TABLE_GET_CHUNK_AT(table, index)->key != 0)
    {
        if(BVRI_TABLE_GET_CHUNK_AT(table, index)->key == hash){

            if(data){
                // copy raw data
                memcpy(
                    BVRI_TABLE_GET_VALUE_AT(table, index),
                    data, table->elemsize
                );
            }

            return BVRI_TABLE_GET_VALUE_AT(table, index);
        }

        index = index + 1 < table->capacity ? index + 1 : 0; 
    }

    // if didn't find the slot, we add it
    BVR_ASSERT(table->count < table->capacity);
    
    BVRI_TABLE_GET_CHUNK_AT(table, index)->key = hash;
    BVRI_TABLE_GET_CHUNK_AT(table, index)->reserved = reserved;
    
    table->count++;

    return BVRI_TABLE_GET_VALUE_AT(table, index);
}