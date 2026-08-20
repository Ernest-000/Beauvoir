#include <bvr/collections/pool.h>

#include <bvr/common.h>
#include <bvr/math.h>

#include <strings.h>
#include <malloc.h>

#define BVRI_POOL_NEXT(pool, current) ((union bvr_pool_block_u*)((union bvr_pool_block_u*)(current) + (size_t)(pool)->chunk_size))
#define BVRI_POOL_MAX(pool) ((pool)->data + ((pool)->capacity * (pool)->chunk_size))

static void bvri_set_chunk_usage(bvr_pool_t* pool, union bvr_pool_block_u* chunk, bool used);
static int bvri_get_chunk_usage(bvr_pool_t* pool, union bvr_pool_block_u* chunk);

void bvr_create_pool(bvr_pool_t* pool, const uint64 elemsize, const uint64 capacity){
    BVR_ASSERT(pool);

    pool->data = NULL;
    pool->next_free = NULL;

    pool->elemsize = elemsize;
    pool->chunk_size = MAX(elemsize, sizeof(union bvr_pool_block_u));
    pool->capacity = capacity;

    pool->data = calloc(pool->capacity, pool->chunk_size);
    BVR_ASSERT(pool->data);

    pool->usage = calloc((capacity + 7) / 8, sizeof(uint8));
    BVR_ASSERT(pool->usage);

    pool->next_free = pool->data;

    // link each chunk together
    union bvr_pool_block_u* current = pool->next_free;
    for (size_t i = 0; i < capacity - 1; i++)
    {
        current->next = BVRI_POOL_NEXT(pool, current);
        current = current->next;
    }
    
    current->next = NULL;
}

void* bvr_pool_alloc(bvr_pool_t* pool){
    BVR_ASSERT(pool);

    void* chunk = NULL;

    // no chunk available
    if(pool->next_free == NULL){
        BVR_PRINT("pool allocation failed");
        return NULL;
    }

    chunk = pool->next_free;
    pool->next_free = pool->next_free->next;

    bvri_set_chunk_usage(pool, chunk, true);

    return chunk;
}

int bvr_pool_is_available(bvr_pool_t* pool, void* chunk){
    BVR_ASSERT(pool->data);
    return bvri_get_chunk_usage(pool, chunk);
}

void bvr_pool_free(bvr_pool_t* pool, void* ptr){
    BVR_ASSERT(pool);

    if(ptr == NULL){
        return;
    }

    union bvr_pool_block_u* freed = (union bvr_pool_block_u*)ptr;
    freed->next = pool->next_free;
    pool->next_free = freed;

    bvri_set_chunk_usage(pool, freed, true);
}

void bvr_destroy_pool(bvr_pool_t* pool){
    BVR_ASSERT(pool);

    free(pool->data);
    pool->data = NULL;
    pool->next_free = NULL;
}

union bvr_pool_block_u* bvr_pool_iterate(struct bvr_pool_iterator_s* iterator){
    while (iterator->current < BVRI_POOL_MAX(iterator->pool))
    {
        union bvr_pool_block_u* block = iterator->current;
        iterator->current = BVRI_POOL_NEXT(iterator->pool, iterator->current);
        
        if(bvri_get_chunk_usage(iterator->pool, block)){
            BVR_PRINTF("%x", block);
            return block;
        }
    }
    
    return NULL;
}

static void bvri_set_chunk_usage(bvr_pool_t* pool, union bvr_pool_block_u* chunk, bool used){
    // check for OOB
    BVR_ASSERT(chunk >= pool->data && chunk < BVRI_POOL_MAX(pool));
    uint32 index = (uint32)((char*)chunk - (char*)pool->data) / pool->chunk_size;

    if(used){
        pool->usage[index / 8] |= (1 << (index % 8));
    }
    else {
        pool->usage[index / 8] &= ~(1 << (index % 8));
    }
}

static int bvri_get_chunk_usage(bvr_pool_t* pool, union bvr_pool_block_u* chunk){
    // check for OOB
    BVR_ASSERT(chunk >= pool->data && chunk < BVRI_POOL_MAX(pool));
    uint32 index = (uint32)((char*)chunk - (char*)pool->data) / pool->chunk_size; 

    return (pool->usage[index / 8] >> (index % 8)) & 1; 
}