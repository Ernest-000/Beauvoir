#pragma once

#include <stdio.h>

#ifdef _WIN32
    #include <io.h>
    
    #define F_OK 0
    #define access _access
#else
    #include <unistd.h>
#endif

#define BVR_FAILED  0x0
#define BVR_OK      0x1

/*   based on OpenGLES   */  
#define BVR_NULL                    0x00
#define BVR_FLOAT                   0x1406
#define BVR_BOOL                    0x1399
#define BVR_INT8                    0x1400
#define BVR_INT16                   0x1402
#define BVR_INT32                   0x1404
#define BVR_UNSIGNED_INT8           0x1401
#define BVR_UNSIGNED_INT16          0x1403
#define BVR_UNSIGNED_INT32          0x1405
#define BVR_VEC2                    0x1407
#define BVR_VEC3                    0x1408
#define BVR_VEC4                    0x140A
#define BVR_MAT3                    0x140B
#define BVR_MAT4                    0x140C

#define BVR_DOUBLE                  0x140D
#define BVR_INT64                   0x140E
#define BVR_UNSIGNED_INT64          0x140F

/*   custom   */  
#define BVR_TEXTURE_2D              0x141A
#define BVR_TEXTURE_2D_ARRAY        0x141B
#define BVR_TEXTURE_2D_LAYER        0x141C
#define BVR_TEXTURE_2D_COMPOSITE    0x141D
#define BVR_TEXTURE_2D_LAYER_STRUCT 0x141E

#define BVR_INCLUDE_BUFFER
#define BVR_INCLUDE_DEBUG
#define BVR_INCLUDE_IO

/*          UTILS               */
/*                              */

typedef char bvr_uuid_t[37] __attribute__ ((aligned(8), packed));

/*
    Return the size of a beauvoir type.
*/
int bvr_sizeof(const int type);

/*
    Return the name of a type as a string.
    WARNING: VERY UNSAFE, if 'name' isn't big enough it might overwrite the next memory chunk!
*/
void bvr_nameof(const int type, char* name);

/*
    java hash function
*/
unsigned int bvr_hash(const char* string);

/*
    decode a base64 string
*/
unsigned char* bvr_base64_decode(const char* string, size_t length, size_t* decoded_length);

/*
    Create a new uuid
*/
void bvr_create_uuid(bvr_uuid_t uuid);
void bvr_copy_uuid(bvr_uuid_t src, bvr_uuid_t dest);

/*
    Check if two uuid are equals
*/
int bvr_uuid_equals(bvr_uuid_t const a, bvr_uuid_t const b);

#define BVR_HAS_FLAG(x, f) ((int)((x & f) == f))

/*          DEBUG                   */
/*                                  */
#ifdef BVR_INCLUDE_DEBUG

char* bvri_string_format(const char* __string, ...);
char* bvri_get_buffer();

void bvri_wmessage(FILE* __stream, const int __line, const char* __file, const char* __message, ...);
void bvri_wassert(const char* __message, const char* __file, unsigned long long __line);
void bvri_wassert_break(const char* __message, const char* __file, unsigned long long __line);
int bvri_werror(const char* __message, int __code);
void bvri_break(const char* __file, unsigned long long __line);

#define BVR_STR(macro) #macro
#define BVR_MACRO_STR(macro) (char*)BVR_STR(macro)
#define BVR_FORMAT(message, ...)(char*)(bvri_string_format(message, __VA_ARGS__))

#define BVR_PRINT(message)(void)(bvri_wmessage(stdout, __LINE__, __FILE__, message))
#define BVR_PRINTF(message, ...)(void)(bvri_wmessage(stdout, __LINE__, __FILE__, message, __VA_ARGS__))

#define BVR_PRINT_VEC3(message, vec3)(void)(bvri_wmessage(stdout, __LINE__, __FILE__, "%s (%f %f %f)", message, vec3[0], vec3[1], vec3[2]))

#ifndef BVR_ASSERT_FORCE_EXIT
#define BVR_ASSERT(expression) (void) (                                         \
    (((expression) == 0) ? bvri_wassert_break(#expression, __FILE__, __LINE__) : 0)  \
)
#elif
#define BVR_ASSERT(expression) (void) (                                         \
    (((expression) == 0) ? bvri_wassert(#expression, __FILE__, __LINE__) : 0)  \
)
#endif

#define BVR_FILE_EXISTS(path) (void)(((access(path, F_OK) == 0) ? 0 : bvri_wassert(path, __FILE__, __LINE__)))

#define BVR_BREAK() (void)(bvri_break(__FILE__, __LINE__))

#endif