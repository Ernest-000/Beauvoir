#pragma once

#include <bvr/config.h>

#include <string.h>

#ifdef _WIN32
/**
 * Safer way to copy a raw C string. 
 * This macro will add a null terminated char at the end of the string
 */
#define BVR_STRCPY(_dst, _src, _length) { \
    *_dst = '\0'; \
    strncat(_dst, _src, _length + 1); \
}
#else
/**
 * Safer way to copy a raw C string. 
 * This macro will add a null terminated char at the end of the string
 */
#define BVR_STRCPY(_dst, _src, _length) { \
    *_dst = '\0'; \
    strlcat(_dst, _src, _length + 1); \
}
#endif

/**
 * return 1 if two strings are equals
 */
#define BVR_STRCMP(_a, _b) (strcmp((_a), (_b)) == 0)

/**
 * return 1 if two strings are equals
 */
#define BVR_STRNCMP(_a, _b, _n) (strncmp((_a), (_b), (_n)) == 0)

/**
 * Pascal typed string
 */
typedef struct bvr_string_s  { 
    uint16 length;
    char* string;
} bvr_string_t;

/*
    Creates a new string
*/
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