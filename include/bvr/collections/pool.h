#pragma once

#include <bvr/config.h>

#define BVR_POOL_FOR_EACH(pool, value) \
    for(uint64 _i = 0; _i < (pool).capacity; _i++) \
    if(bvr_pool_is_available(&(pool), (pool).data + _i * (pool).chunk_size) && ((value) = (pool).data + _i * (pool).chunk_size, 1))

/**
 * chunk of memory in a pool
 */
union bvr_pool_block_u {
    void* data;
    union bvr_pool_block_u* next;
};

/**
 * A constant list of equaly-sized generic objects.
 * Each element is link to another.
 */
typedef struct bvr_pool_s {
    // linked list entry point
    void* data;

    // store chunk usage as bitflags
    uint8* usage;

    // next free block
    union bvr_pool_block_u* next_free;

    // size of each element
    uint32 elemsize;

    // size of each chunk
    uint32 chunk_size;

    // maximum number of elements
    uint32 capacity;
} bvr_pool_t;

void bvr_create_pool(bvr_pool_t* pool, const uint64 elemsize, const uint64 count);

/*
    Get a pointer to the next writable slot.
*/
void* bvr_pool_alloc(bvr_pool_t* pool);

/**
    Deallocate a memory block.
*/
void bvr_pool_free(bvr_pool_t* pool, void* chunk);

/**
 * Return if a chunk is available or not.
 */
int bvr_pool_is_available(bvr_pool_t* pool, void* chunk);

void bvr_destroy_pool(bvr_pool_t* pool);