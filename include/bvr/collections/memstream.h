#pragma once

#include <bvr/config.h>

#include <bvr/collections/buffer.h>

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

/**
 * Memory stream is a long pre-allocated memory space where things can be written.
    Work like a FILE* but in-memory :D
 */
typedef struct bvr_memstream_s {
    void* data;
    uint64 size;

    char* cursor;
    char* next;
} bvr_memstream_t;

/*
    Create a new memory stream
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