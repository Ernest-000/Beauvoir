#include <BVR/buffer.h>
#include <BVR/common.h>

#include <malloc.h>
#include <string.h>

#ifndef BVR_NO_GROWTH

#define BVR_GROWTH_FACTOR 2

static int bvri_grow_buffer(void* ptr, uint64* size);

static int bvri_grow_buffer(void* ptr, uint64* size){
    if(ptr && size){
        ptr = realloc(ptr, (*size *= BVR_GROWTH_FACTOR));
        return ptr != NULL;
    }

    return 0;
}

#endif

void bvr_create_memstream(bvr_memstream_t* stream, unsigned long long const size){
    BVR_ASSERT(stream);

    if(stream->data){
        return;
    }

    stream->data = NULL;
    stream->size = size;
    stream->cursor = NULL;

    if(size){
        stream->data = malloc(size);
        stream->cursor = stream->data;
        stream->next = stream->data;
        BVR_ASSERT(stream->data);

        memset(stream->data, 0, stream->size);
    }
}

char* bvr_memstream_write(bvr_memstream_t* stream, const void* data, const uint64 size){
    BVR_ASSERT(stream && stream->data);

    if(stream->cursor - (char*)stream->data + size < stream->size){
        if(data){
            memcpy(stream->cursor, data, size);
        }

        stream->cursor += size;
        stream->next += size;
    }
    else {

#ifndef BVR_NO_GROWTH
        if(stream->cursor - (char*)stream->data + size < stream->size * BVR_GROWTH_FACTOR){
            bvri_grow_buffer(stream->data, &stream->size);
            
            if(data){
                memcpy(stream->cursor, data, size);
            }
            
            stream->cursor += size;
            stream->next += size;
        }
        else {
            BVR_ASSERT(0 || "out of bounds!");

        }
#else
        BVR_ASSERT(0 || "out of bounds!");
#endif

    }

    return stream->cursor;
}

char* bvr_memstream_read(bvr_memstream_t* stream, void* dest, const uint64 size){
    BVR_ASSERT(stream && stream->data);
    BVR_ASSERT(dest);

    if(stream->cursor - (char*)stream->data + size < stream->size){
        memcpy(dest, stream->cursor, size);
        stream->cursor += size;
    }
    else {
        BVR_ASSERT(0 || "out of bounds!");
    }

    return stream->cursor;
}

char* bvr_memstream_seek(bvr_memstream_t* stream, uint64 position, int mode){
    BVR_ASSERT(stream);

    switch (mode)
    {
    case SEEK_CUR:
        {
            if(stream->cursor - (char*)stream->data + position < stream->size){
                stream->cursor += position;
            } 
            else {
                BVR_ASSERT(0 || "out of bounds!");
            }
        }
        break;

    case SEEK_SET:
        {
            if(position <= stream->size){
                stream->cursor = stream->data + position;
            }
            else {
                BVR_ASSERT(0 || "out of bounds!");
            }
        }
        break;

    case SEEK_END:
        {
            if(position <= stream->size){
                stream->cursor = stream->data + (stream->size - position);
            }
            else {
                BVR_ASSERT(0 || "out of bounds!");
            }
        }
        break;

    case SEEK_NEXT:
        {
            stream->cursor = stream->next;
        }
        break;

    default:
        BVR_ASSERT(0 || "invalid seeking mode!");
        break;
    }

    return stream->cursor;
}

void bvr_memstream_clear(bvr_memstream_t* stream){
    BVR_ASSERT(stream);

    stream->cursor = stream->data;
    stream->next = stream->data;
    memset(stream->data, 0, stream->size);
}

void bvr_destroy_memstream(bvr_memstream_t* stream){
    BVR_ASSERT(stream);

    // clear all stream data
    // so that it will be an empty chunk if we're trying to access
    // to the stream's data after it has been freed.
    memset(stream->data, 0, stream->size);

    free(stream->data);

    stream->size = 0;
    stream->cursor = NULL;
    stream->data = NULL;
}

void bvr_create_string(bvr_string_t* string, const char* value){
    BVR_ASSERT(string);

    string->length = 0;
    string->string = NULL;

    if(value){
        string->length = strlen(value) + 1;
        string->string = malloc(string->length);
        BVR_ASSERT(string->string);

        memcpy(string->string, value, string->length - 1);
        string->string[string->length - 1] = '\0';
    }
}

int bvr_overwrite_string(bvr_string_t* string, const char* value, uint32 length){
    BVR_ASSERT(string);

    if(!string->string){
        bvr_create_string(string, value);
        return BVR_FALSE;
    }

    if(length == 0){
        length = strlen(value);
    }

    if(value){

        if(string->length < length){
            string->string = realloc(string->string, length);
            BVR_ASSERT(string->string);
        }

        string->length = length;
        memcpy(string->string, value, string->length - 1);
        string->string[string->length - 1] = '\0';
    }

    return BVR_TRUE;
}

void bvr_string_concat(bvr_string_t* string, const char* other){
    BVR_ASSERT(string);
    
    if(other) {
        
        // string is already allocated
        if(string->string){
            unsigned int size = string->length;

            // new size = size - 1 (EOF) + strlen(other) + 1 (new EOF) 
            string->length = string->length + strlen(other);
            string->string = realloc(string->string, string->length);
            BVR_ASSERT(string->string);

            strcat(string->string, other);
            string->string[string->length - 1] = '\0';
        }
        else {
            string->length = strlen(other) + 1;
            string->string = malloc(string->length);
            
            memcpy(string->string, other, string->length - 1);
            string->string[string->length - 1] = '\0';
        }
    }
}

void bvr_string_create_and_copy(bvr_string_t* dest, bvr_string_t* source){
    BVR_ASSERT(dest);

    if(source) {
        dest->length = source->length;
        dest->string = malloc(dest->length);
        BVR_ASSERT(dest->string);

        memcpy(dest->string, source->string, dest->length);
        dest->string[dest->length - 1] = '\0';
    }
}

void bvr_string_insert(bvr_string_t* string, const uint64 offset, const char* value){
    BVR_ASSERT(string);
    BVR_ASSERT(string->string);
    BVR_ASSERT(value);
    
    bvr_string_t prev;
    bvr_string_create_and_copy(&prev, string);

    if(value) {
        const uint64 vlen = strlen(value);
        //BVR_PRINTF("string %x ; string size %i", string->data, string->length);

        string->length += vlen;
        string->string = realloc(string->string, string->length);
        BVR_ASSERT(string->string);

        memset(string->string, 0, string->length);

        strncpy(string->string, prev.string, offset);
        string->string[offset] = '\0';
        strncat(string->string, value, vlen);
        strncat(string->string, &prev.string[offset], prev.length - offset);
    }

    bvr_destroy_string(&prev);
}

void bvr_destroy_string(bvr_string_t* string){
    BVR_ASSERT(string);
    free(string->string);
    string->string = NULL;
    string->length = 0;
}

void bvr_create_pool(bvr_pool_t* pool, const uint64 size, const uint64 count){
    BVR_ASSERT(pool);
    
    pool->data = NULL;
    pool->next_block = NULL;
    pool->avail = NULL;

    pool->capacity = count + 1;
    pool->size = (size + sizeof(struct bvr_pool_block_u)) * pool->capacity;
    pool->elemsize = size;
    pool->count = 0;

    // allocate enough space for the linked list and raw memory space
    pool->data = malloc(pool->size);
    BVR_ASSERT(pool->data);

    pool->next_block = (void*)pool->data;
    pool->blocks = (void*)pool->data;
    pool->avail = &pool->data[pool->capacity * sizeof(struct bvr_pool_block_u)];

    // link all chunk together
    for (size_t i = 0; i < pool->capacity; i++)
    {
        pool->blocks[i].data = NULL;
        pool->blocks[i].next = &pool->blocks[i + 1];
    }

    pool->blocks[pool->capacity - 1].next = NULL;
}

void* bvr_pool_alloc(bvr_pool_t* pool){
    BVR_ASSERT(pool);

    // check if a next block is available and if data can be added
    if(pool->next_block == NULL || 
        pool->avail + pool->elemsize >= pool->data + pool->size){
        return NULL;
    }

    struct bvr_pool_block_u* block = pool->next_block;
    pool->next_block = pool->next_block->next;

    block->data = pool->avail;
    pool->avail += pool->elemsize;

    pool->count++;
    return block->data;
}

void* bvr_pool_try_get(bvr_pool_t* pool, int index){
    BVR_ASSERT(pool);
    
    if(index < 0 && index >= pool->count){
        return NULL;
    }
    
    return pool->blocks[index].data;
}

void bvr_pool_free(bvr_pool_t* pool, void* ptr){
    BVR_ASSERT(pool);

    if(ptr == NULL) {
        return;
    }

    // set current block as free 
    // and push it as the next allocated block
    struct bvr_pool_block_u* block = (struct bvr_pool_block_u*)ptr;
    uint32 index = ((char*)block - pool->data) / sizeof(struct bvr_pool_block_u);

    if(index){      
        memmove(block->data, block->data + pool->elemsize, 
            (pool->capacity - index - 1) * pool->elemsize
        );

        // remap 
        for (struct bvr_pool_block_u* b = &pool->blocks[index]; b->next; b++)
        {
            if(b->data){
                b->data -= pool->elemsize;
            }
        }
    }

    block->next = pool->next_block;
    block->data = NULL;

    pool->next_block = block;
    pool->count--;
}

void bvr_pool_remove(bvr_pool_t* pool, const void* value){
    BVR_ASSERT(pool);
    BVR_ASSERT(value);

    for (struct bvr_pool_block_u* b = pool->blocks; b->next; b++)
    {
        if(b->data && value == *(void**)b->data){
            bvr_pool_free(pool, b);
            return;
        }
    }
}

void bvr_destroy_pool(bvr_pool_t* pool){
    BVR_ASSERT(pool);

    free(pool->data);

    pool->data = NULL;
    pool->avail = NULL;
    pool->blocks = NULL;
    pool->next_block = NULL;
}