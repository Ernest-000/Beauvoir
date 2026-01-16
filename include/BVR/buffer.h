#pragma once

#include <BVR/config.h>

#include <stdio.h>

#ifndef BVR_BUFFER_SIZE
    #define BVR_BUFFER_SIZE 1024
#endif

#ifndef BVR_POOL_SIZE
    #define BVR_POOL_SIZE 2048
#endif

#if !defined(SEEK_NEXT)
    /* Seek from beginning of file. */
    #define SEEK_SET 0

    /* Seek from current position. */
    #define SEEK_CUR 1

    /* Set file pointer to EOF plus "offset" */
    #define SEEK_END 2       

    /* Seek to the next available memory block */
    #define SEEK_NEXT 3 
#endif

#ifdef __GNUC__
    /**
     *  This macro creates a for loop that interates through a pool.
     *  It will define each `_v` as the current used value.
     */
    #define BVR_POOL_FOR_EACH(_v, _pool) for (struct bvr_pool_block_u* b = _pool.blocks; b->next; b++)\
                                            if (b->data && (void*)memcpy(&_v, b->data, _pool.elemsize) != NULL)
    // Clang specific macro
#elif defined(__clang__) || defined(_MSC_VER)
    /**
     *  This macro creates a for loop that interates through a pool.
     *  It will define each `_v` as the current used value.
     */
    #define BVR_POOL_FOR_EACH(_v, _pool) for (struct bvr_pool_block_u* b = _pool.blocks; b->next; b++)\
                                            if (b->data && (void*)memcpy(&_v, b->data, _pool.elemsize) != NULL)
#else
    /**
     *  This macro creates a for loop that interates through a pool.
     *  It will define each `_v` as the current used value.
     */
    #define BVR_POOL_FOR_EACH(_v, _pool) for (struct bvr_pool_block_u* b = _pool.blocks; b->next; b++)\
                                            if (b->data && (void*)memcpy(&_v, b->data, _pool.elemsize) != NULL)                               
#endif

/**
 * Allocate _size bytes of memory for a generic buffer.
 */
#define BVR_BUFFER_MALLOC(buffer, _size) buffer.data = malloc(_size);

/**
 * Reallocates the given buffer and resize it to _size bytes.
 */
#define BVR_BUFFER_CONST_REALLOC(buffer, _size) { \
    buffer.size = _size; \
    buffer.data = realloc(buffer.data, buffer.size); }

/**
 * Return the number of element of a generic buffer.
 */
#define BVR_BUFFER_COUNT(buffer) (((unsigned long)(buffer.size / buffer.elemsize)))

#ifndef BVR_NO_GROWTH
    #define BVR_GROWTH_FACTOR 2

    /**
     * Reallocates the given buffer and add _size bytes by the Growth factor
     */
    #define BVR_BUFFER_REALLOC(buffer, _size) { \
        buffer.size += (_size * BVR_GROWTH_FACTOR); \
        buffer.data = realloc(buffer.data, buffer.size); }
#else
    #define BVR_BUFFER_REALLOC(buffer, _size) BVR_BUFFER_CONST_REALLOC(buffer, _size)
#endif

/*
    Generic data pointer
*/
struct __attribute__((packed)) bvr_buffer_s {
    void* data;
    unsigned long size;
    unsigned int elemsize;
};

/**
 * Memory stream
 */
typedef struct bvr_memstream_s {
    void* data;
    unsigned long long size;

    char* cursor;
    char* next;
} bvr_memstream_t;

/**
 * Pascal typed string
 */
typedef __attribute__ ((packed)) struct bvr_string_s  { 
    unsigned short length;
    char* string;
} bvr_string_t;

/**
 * Chunck of memory in a pool
 */
__attribute__ ((packed)) struct bvr_pool_block_u {
    void* data;
    struct bvr_pool_block_u* next;
};

/**
 * A constant list of equaly-sized generic objects.
 * Each element is link to another.
 */
typedef struct bvr_pool_s {
    char* data;
    char* avail;

    struct bvr_pool_block_u* blocks;
    struct bvr_pool_block_u* next_block;

    uint32 size;
    uint32 elemsize;
    uint32 count;
    uint32 capacity;
} bvr_pool_t;

/*
    Create a new memory stream which is a long pre-allocated memory space where things can be written.
    Work like a FILE* but in-memory :D
*/
void bvr_create_memstream(bvr_memstream_t* stream, const uint64 size);

char* bvr_memstream_write(bvr_memstream_t* stream, const void* data, const uint64 size);
char* bvr_memstream_read(bvr_memstream_t* stream, void* dest, const uint64 size);
char* bvr_memstream_seek(bvr_memstream_t* stream, uint64 position, int mode);
void bvr_memstream_clear(bvr_memstream_t* stream);

BVR_H_FUNC int bvr_memstream_eof(bvr_memstream_t* stream){
    return stream->cursor - (char*)stream->data >= stream->size;
}

void bvr_destroy_memstream(bvr_memstream_t* stream);

void bvr_create_string(bvr_string_t* string, const char* value);

/*
    Use an already created string to replace its value.
    Returns BVR_FALSE if it had to create a new string, BVR_TRUE otherwise 
*/
int bvr_overwrite_string(bvr_string_t* string, const char* value, uint32 length);

/*
    Concatenate a string.
    WARNING: function might be slow -> no growth factor :(
*/
void bvr_string_concat(bvr_string_t* string, const char* other);

/*
    Allocate a new string and copy other string's content.
*/
void bvr_string_create_and_copy(bvr_string_t* dest, bvr_string_t* source);

/*
    Insert a char array into a string.
*/
void bvr_string_insert(bvr_string_t* string, const uint64 offset, const char* value);

/*
    Return a constant pointer to string's char array.
*/
BVR_H_FUNC const char* bvr_string_get(bvr_string_t* string){
    if(string){
        return string->string;
    }
    return NULL;
}

/*
    Free the string.
*/
void bvr_destroy_string(bvr_string_t* string);

void bvr_create_pool(bvr_pool_t* pool, const uint64 size, const uint64 count);

/*
    Get a pointer to the next writable slot.
*/
void* bvr_pool_alloc(bvr_pool_t* pool);

/**
    Deallocate a memory block.
*/
void bvr_pool_free(bvr_pool_t* pool, void* ptr);

/**
 * Remove an element from the pool by using its value.
 */
void bvr_pool_remove(bvr_pool_t* pool, const void* value);

void bvr_destroy_pool(bvr_pool_t* pool);