#include <bvr/collections/pool.h>

#include <bvr/common.h>
#include <bvr/math.h>

#include <strings.h>
#include <malloc.h>

void bvr_create_pool(bvr_pool_t* pool, const uint64 elemsize, const uint64 count){
    BVR_ASSERT(pool);

    pool->data = NULL;
    pool->next_free = NULL;

    pool->elemsize = elemsize;
    pool->chunck_size = MAX(elemsize, sizeof(union bvr_pool_block_u));
    pool->capacity = count;

    pool->data = malloc(pool->chunck_size * pool->capacity);
    BVR_ASSERT(pool->data);

    pool->next_free = pool->data;

    // link each chunck together
    union bvr_pool_block_u* current = pool->next_free;
    for (size_t i = 0; i < count - 1; i++)
    {
        BVR_PRINTF("new node %x", current);

        current->next = current + (size_t)pool->chunck_size;
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
    BVR_PRINTF("next chunk %x", pool->next_free);
    pool->next_free = pool->next_free->next;

    BVR_PRINTF("allocated chunck %x", chunk);

    return chunk;
}

void bvr_pool_free(bvr_pool_t* pool, void* ptr){
    BVR_ASSERT(pool);

    if(ptr == NULL){
        return;
    }

    union bvr_pool_block_u freed = (union bvr_pool_block_u)ptr;
    freed.next = pool->next_free;
    pool->next_free = freed.next;
}

void bvr_destroy_pool(bvr_pool_t* pool){
    BVR_ASSERT(pool);

    free(pool->data);
    pool->data = NULL;
    pool->next_free = NULL;
}