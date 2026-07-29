#pragma once

#include <bvr/config.h>

#include <bvr/collections/string.h>

#include <stdio.h>

#ifndef BVR_BUFFER_SIZE
    #define BVR_BUFFER_SIZE 1024
#endif

#define BVR_BE_TO_LE_U16 __bswap_constant_16
#define BVR_BE_TO_LE_U32 __bswap_constant_32

/**
 * @brief check if a file exists by using it's path.
 * @param path the path to the file to check
 * @return BVR_TRUE if the file exists.
 */
int bvr_fexists(const char* path);

/**
 * @brief check if a directory exists by using it's path.
 * @param path the path to the directory to check
 * @return BVR_TRUE if the dir exists.
 */
int bvr_direxists(const char* path);

/*
    Return size of a file.
*/
uint64 bvr_fsize(FILE* file);

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
int bvr_fread24_le(FILE* file);

/*
    Read a single signed int from a stream.
*/
int bvr_fread32_le(FILE* file);

/*
    Read a single signed long from a stream.
*/
int64 bvr_fread64_le(FILE* file);

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
    Read a single unsigned long from a stream.
*/
uint64 bvr_freadu64_le(FILE* file);

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
BVR_H_FUNC uint8 bvr_freadu8_be(FILE* file){
    return bvr_freadu8_le(file);
}

/*
    Read a single unsigned short from a stream and translate big-endian to little-endian.
*/
BVR_H_FUNC uint16 bvr_freadu16_be(FILE* file){
    uint16 value = bvr_freadu16_le(file);
    return BVR_BE_TO_LE_U16(value);
}

/*
    Read a single unsigned int from a stream and translate big-endian to little-endian.
*/
BVR_H_FUNC uint32 bvr_freadu32_be(FILE* file){
    uint32 value = bvr_freadu32_le(file);
    return BVR_BE_TO_LE_U32(value);
}

/*
    Read a single signed short from memory.
*/
BVR_H_FUNC uint8 bvr_mread8_le(uint8** mem){
    return (uint8)(*(*mem)++);
}

/*
    Read a single signed short from memory.
*/
short bvr_mread16_le(uint8** mem);

/*
    Read a single signed int from memory.
*/
int bvr_mread32_le(uint8** mem);

/*
    Read a single unsigned int from memory.
*/
uint32 bvr_mreadu32_le(uint8** mem);