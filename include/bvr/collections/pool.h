#pragma once

#include <bvr/config.h>

#ifndef BVR_POOL_SIZE
    #define BVR_POOL_SIZE 2048
#endif

// #ifdef __GNUC__
//     /**
//      *  This macro creates a for loop that interates through a pool.
//      *  It will define each `_v` as a pointer to the current used value.
//      */
//     #define BVR_POOL_FOR_EACH(_v, _pool) for (struct bvr_pool_block_s* b = _pool.first; b->next; b++)\
//                                             if (b && (void*)memcpy(&_v, b, _pool.elemsize) != NULL)
//     // Clang specific macro
// #elif defined(__clang__) || defined(_MSC_VER)
//     /**
//      *  This macro creates a for loop that interates through a pool.
//      *  It will define each `_v` as the current used value.
//      */
//     #define BVR_POOL_FOR_EACH(_v, _pool) for (struct bvr_pool_block_u* b = _pool.first; b->next; b++)\
//                                             if (b->data && (void*)memcpy(&_v, &b->data, sizeof(uint64)) != NULL)
// #else
//     /**
//      *  This macro creates a for loop that interates through a pool.
//      *  It will define each `_v` as the current used value.
//      */
//     #define BVR_POOL_FOR_EACH(_v, _pool) for (struct bvr_pool_block_u* b = _pool.first; b->next; b++)\
//                                             if (b->data && (void*)memcpy(&_v, &b->data, sizeof(uint64)) != NULL)
// #endif

#define BVR_POOL_FOR_EACH(_v, _pool) while(0)

/**
 * Chunck of memory in a pool
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
    // malloc entry point
    void* data;

    union bvr_pool_block_u* next_free;

    uint32 elemsize;
    uint32 chunck_size;
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
void bvr_pool_free(bvr_pool_t* pool, void* ptr);

void bvr_destroy_pool(bvr_pool_t* pool);