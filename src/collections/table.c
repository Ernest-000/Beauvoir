#include <bvr/collections/table.h>
#include <bvr/collections/buffer.h>
#include <bvr/common.h>
#include <bvr/math.h>
#include <bvr/io.h>

#include <malloc.h>

#define BVRI_TABLE_GET_CHUNK_AT(table, i) \
    ((struct bvr_table_chunk_s*)((uint8*)(table->entries) + (i) * (table->chunck_size)))

#define BVRI_TABLE_GET_VALUE_AT(table, i) \
    ((uint8*)BVRI_TABLE_GET_CHUNK_AT(table, i) + sizeof(struct bvr_table_chunk_s))

/**
 * @brief expend an hash table.
 */
static int bvri_table_grow(bvr_table_t* table);

/**
 * @brief add or/and set an element of an array of hashtable chuncks.
 * @param entries the hashtable chunk array (must be pre-allocated).
 * @param capacity the maximum amount of data that can be added to the table.
 * @param chunck_size the size of each slot.
 * @param count a pointer to the count variable (will be incremented when a new element is added).
 * @param key the key of the value to add/set.
 * @param key the hash of the value to add/set.
 * @param data the source data to copy.
 * @param reserved the new value of the reserved flag.
 */
static void* bvri_table_set(struct bvr_table_chunk_s* entries, const uint32 capacity, const uint32 chunk_size, uint32* count,
    const char* key, const uint32 _hash, void* data, uint32 reserved);

int bvr_create_table(bvr_table_t* table, const uint32 elemsize, const uint32 capacity){
    BVR_ASSERT(table);

    table->count = 0;
    table->capacity = (uint32)npow2(capacity);
    table->elemsize = elemsize;
    table->chunck_size = sizeof(struct bvr_table_chunk_s) + elemsize;
    table->entries = NULL;

    if(capacity){
        table->entries = calloc(table->capacity, table->chunck_size);
        BVR_ASSERT(table->entries);
    }

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
        if(!bvri_table_grow(table)){
            return NULL;
        }
    }

    return bvri_table_set(
        table->entries, table->capacity, table->chunck_size,
        &table->count, key, 0, value, 0
    );
}

void* bvr_table_add(bvr_table_t* table, const char* key, uint32 flag){
    BVR_ASSERT(table);
    BVR_ASSERT(key);

    if(table->count >= table->capacity / BVR_GROWTH_FACTOR){
        // expend
        if(!bvri_table_grow(table)){
            return NULL;
        }
    }

    return bvri_table_set(
        table->entries, table->capacity, table->chunck_size,
        &table->count, key, 0, NULL, flag
    );
}

void bvr_destroy_table(bvr_table_t* table){
    free(table->entries);
    table->entries = NULL;
}

void* bvr_table_iterate(struct bvr_table_iterator_s* iterator){
    BVR_ASSERT(iterator);

    while (iterator->index < iterator->table->capacity)
    {
        int current = iterator->index++;
        if(BVRI_TABLE_GET_CHUNK_AT(iterator->table, current)->key != 0){
            iterator->current = BVRI_TABLE_GET_CHUNK_AT(iterator->table, current);
            return BVRI_TABLE_GET_VALUE_AT(iterator->table, current);
        }
    }
    
    return NULL;
}

static int bvri_table_grow(bvr_table_t* table){
    BVR_ASSERT(table);

    uint32 n_count = 0;
    uint32 n_capacity = table->capacity * BVR_GROWTH_FACTOR;
    if(n_capacity < table->capacity){
        // overflow
        return BVR_FALSE;
    }

    struct bvr_table_chunk_s* entries = calloc(n_capacity, table->chunck_size);
    BVR_ASSERT(entries);
    
    for (size_t i = 0; i < table->capacity; i++)
    {
        if(BVRI_TABLE_GET_CHUNK_AT(table, i)->key != 0){
            bvri_table_set(
                entries, n_capacity, table->chunck_size,
                &n_count, NULL, BVRI_TABLE_GET_CHUNK_AT(table, i)->key,
                BVRI_TABLE_GET_VALUE_AT(table, i), BVRI_TABLE_GET_CHUNK_AT(table, i)->reserved
            );
        }
    }
    
    free(table->entries);
    table->entries = entries;
    table->capacity = n_capacity;
    return BVR_TRUE;
}

static void* bvri_table_set(struct bvr_table_chunk_s* entries, const uint32 capacity, const uint32 chunksz, uint32* count,
    const char* key, const uint32 _hash, void* data, uint32 res){
    // no assert, values shall be previously checked
    const uint32 elemsz = chunksz - sizeof(struct bvr_table_chunk_s);
    
    uint32 hash = key != NULL ? bvr_hash(key) : _hash;
    uint32 index = (hash & (uint32)(capacity - 1));
    
    void* value = NULL;
    struct bvr_table_chunk_s* chunk = NULL;
    for (size_t i = index; i < capacity; i++)
    {
        chunk = (void*)((uint8*)entries + i * chunksz);
        value = (void*)chunk + sizeof(struct bvr_table_chunk_s);

        // found an empty slot
        if(chunk->key == 0){
            break;
        }

        if(chunk->key == hash){
            if(data){
                memcpy(value, data, elemsz);
            }

            return value;
        }

        if(i + 1 >= capacity){
            i = 0;
        }
    }
    
    
    // when the table is full
    if(*count >= capacity){
        return NULL;
    }

    chunk->key = hash;
    chunk->reserved = res;
    if(data){
        memcpy(value, data, elemsz);
    }

    (*count)++;

    return value;
}