#pragma once

#include <BVR/config.h>
#include <BVR/buffer.h>

#include <stdint.h>
#include <stdio.h>

#pragma endregion

#define BVR_BE_TO_LE_U16 __bswap_16
#define BVR_BE_TO_LE_U32 __bswap_32

/*
    Return size of a file.
*/
uint64 bvr_get_file_size(FILE* file);

/*
    Read all the file and copy data into a string.
*/
int bvr_read_file(bvr_string_t* string, FILE* file);

/*
    Read a single signed short from a stream.
*/
short bvr_fread16_le(FILE* file);

/*
    Read a single signed int from a stream.
*/
int bvr_fread32_le(FILE* file);

/*
    Read a single unsigned char from a stream.
*/
uint8 bvr_freadu8_le(FILE* file);

/*
    Read a single unsigned short from a stream.
*/
uint16 bvr_freadu16_le(FILE* file);

/*
    Read a single unsigned int from a stream.
*/
uint32 bvr_freadu32_le(FILE* file);

/*
    Read a single float from a stream.
*/
float bvr_freadf(FILE* file);

/*
    Read a null terminate string from a stream.
*/
void bvr_freadstr(char* string, uint64 size, FILE* file);

/*
    Read a single unsigned char from a stream and translate big-endian to little-endian.
*/
static inline uint8 bvr_freadu8_be(FILE* file){
    return bvr_freadu8_le(file);
}

/*
    Read a single unsigned short from a stream and translate big-endian to little-endian.
*/
static inline uint16 bvr_freadu16_be(FILE* file){
    uint16 value = bvr_freadu16_le(file);
    return BVR_BE_TO_LE_U16(value);
}

/*
    Read a single unsigned int from a stream and translate big-endian to little-endian.
*/
static inline uint32 bvr_freadu32_be(FILE* file){
    uint32 value = bvr_freadu32_le(file);
    return BVR_BE_TO_LE_U32(value);
}