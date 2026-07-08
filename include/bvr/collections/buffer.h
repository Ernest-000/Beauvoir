#pragma once

#include <bvr/config.h>

/**
 * Allocate _size bytes of memory for a generic buffer.
 */
#define BVR_BUFFER_MALLOC(buffer, _size) buffer.data = malloc(_size);

/**
 * Reallocates the given buffer and resize it to _size bytes.
 */
#define BVR_BUFFER_CONST_REALLOC(buffer, _size) { \
    (buffer).size = _size; \
    (buffer).data = realloc((buffer).data, (buffer).size); \
}

/**
 * Return the number of element of a generic buffer.
 */
#define BVR_BUFFER_COUNT(buffer) ((uint64)((buffer).size / (buffer).elemsize))

#ifndef BVR_NO_GROWTH
    // MUST BE A POWER OF 2
    #define BVR_GROWTH_FACTOR 2

    /**
     * Reallocates the given buffer and add _size bytes by the Growth factor
     */
    #define BVR_BUFFER_REALLOC(buffer, _size) { \
        (buffer).size += (_size * BVR_GROWTH_FACTOR); \
        (buffer).data = realloc((buffer).data, (buffer).size); }
#else
    #define BVR_BUFFER_REALLOC(buffer, _size) BVR_BUFFER_CONST_REALLOC(buffer, _size)
#endif

/*
    Generic data pointer
*/
struct bvr_buffer_s {
    char* data;
    uint64 size;
    uint32 elemsize;
};