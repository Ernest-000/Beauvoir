#pragma once

#include <stdio.h>

#include <bvr/config.h>

#ifdef _WIN32
    #include <io.h>
    
    #define F_OK 0
    #define access _access
#else
    #include <unistd.h>
#endif

// Unused
#ifdef RELEASE
    static FILE* bvri_std_stream;

    #define bvr_stdout bvri_std_stream
#else   
    #define bvr_stdout stdout
#endif

#define BVR_FALSE  0UL
#define BVR_TRUE   1UL

/*   based on OpenGLES   */  
#define BVR_NULL                    0x00
#define BVR_BOOL                    0x1399
#define BVR_INT8                    0x1400
#define BVR_UNSIGNED_INT8           0x1401
#define BVR_INT16                   0x1402
#define BVR_UNSIGNED_INT16          0x1403
#define BVR_INT32                   0x1404
#define BVR_UNSIGNED_INT32          0x1405
#define BVR_FLOAT                   0x1406
#define BVR_VEC2                    0x1407
#define BVR_VEC3                    0x1408
#define BVR_VEC4                    0x140A
#define BVR_MAT3                    0x140B
#define BVR_MAT4                    0x140C

#define BVR_DOUBLE                  0x140D
#define BVR_INT64                   0x140E
#define BVR_UNSIGNED_INT64          0x140F

/*   custom   */  
#define BVR_TEXTURE_2D              0x0DE1
#define BVR_TEXTURE_3D              0x806F
#define BVR_TEXTURE_2D_ARRAY        0x8C1A
#define BVR_TEXTURE_2D_LAYER        0x141C
#define BVR_TEXTURE_2D_COMPOSITE    0x141D

#define BVR_TEXTURE_2D_LAYER_STRUCT 0x141E

/**
 * @brief check if a type is a correct beauvoir type.
 * @param t the type to check
 * @return returns true if t is a valid beauvoir type
 */
#define BVR_IS_AVAIL_TYPE(t) (t >= BVR_TEXTURE_2D && t <= BVR_TEXTURE_2D_ARRAY)

/**
 * @brief check if a type is an available texture type.
 * @param t the type to check
 * @return returns true if t is a valid texture type
 */
#define BVR_IS_AVAIL_TEXTURE(t) ( \\
    t == BVR_TEXTURE_2D || \\
    t == BVR_TEXTURE_3D || \\
    t == BVR_TEXTURE_2D_ARRAY || \\
    t == BVR_TEXTURE_2D_LAYER || \\
    t == BVR_TEXTURE_2D_COMPOSITE || \\
    t == BVR_TEXTURE_2D_LAYER_STRUCT || \\
)

/**
 * @brief compare two types and returns the biggest of these two.
 * @param a the first type.
 * @param b the second type.
 * @return return the sizeof of the biggest of a or b.
 */
#define BVR_MAX_SIZEOF(a, b) ((sizeof(a) > sizeof(b)) ? sizeof(a) : sizeof(b))

/*          UTILS               */
/*                              */

typedef char bvr_uuid_t[37];

/**
 * @brief get the size in bytes of a Beauvoir's enum type
 * @param type the type.
 * @return the size, in byte of the target type, 
 */
int bvr_sizeof(const int type);

/*
    Return the name of a type as a string.
    WARNING: VERY UNSAFE, if 'name' isn't big enough it might overwrite the next memory chunk!
*/
void bvr_nameof(const int type, char* name);

/**
 * @brief hash a string input and return the corresponding hashed unsigned int.
 * @param string the source char array.
 * @return the corresponding hash.
 */
uint32 bvr_hash(const char* string);

/*
    decode a base64 string
*/
uint8* bvr_base64_decode(const char* string, size_t length, size_t* decoded_length);

/**
 * @brief check if a number contains a bitflag.
 * @param n the number that might contain the bitflag.
 * @param f the bitflag.
 * @return return true if the flag is marked and false otherwise.
 */
#define BVR_HAS_FLAG(n, f) ((int)((n & f) == f))

/**
 * @brief try to call a function from a function pointer.
 * @param func the function's pointer.
 * @param args the argument that will be passed to the function.
 * @return returns the same as the return valud of func.
 */
#define BVR_CALL(func, ...) ((func) ? (func)(__VA_ARGS__) : 0)

/*          DEBUG                   */
/*                                  */
#ifndef BVR_NO_DEBUG

char* bvri_string_format(const char* __string, ...);
char* bvri_get_buffer();

void bvri_wmessage(FILE* __stream, const int __line, const char* __file, const char* __message, ...);
void bvri_wassert(const char* __message, const char* __file, unsigned long long __line);
void bvri_wassert_break(const char* __message, const char* __file, unsigned long long __line);
int bvri_werror(const char* __message, int __code);
void bvri_break(const char* __file, unsigned long long __line);

#define BVR_STR(macro) #macro
#define BVR_MACRO_STR(macro) (char*)BVR_STR(macro)

/**
 * @brief format a string, just like printf, and retruns it.
 * @param message a string that specifies the data to be formated. 
 * It can also contains placeholder to print any variable types.
 * @param args variables or values corresponding to the format specifier.
 * @return Returns the formatted string. **DO NOT FREE THIS STRING.** 
 */
#define BVR_FORMAT(message, ...)(char*)(bvri_string_format(message, __VA_ARGS__))

#if defined(BVR_DEBUG)
/**
 * @brief print a formated string to the standard output.
 * @param message a string that specifies the data to be printed. 
 * It can also contains placeholder to print any variable types.
 * @param args variables or values corresponding to the format specifier.
 */
#define BVR_PRINTF(message, ...)(void)(bvri_wmessage(bvr_stdout, __LINE__, __FILE__, message, __VA_ARGS__))
#else 
#define BVR_PRINTF(message, ...) do {} while (0);
#endif

#if defined(BVR_DEBUG)
/**
 * @brief print an object or a value to the standard output.
 * @param t the object/value to print.
 */
#define BVR_PRINT(t) _Generic((t),          \
    void* : BVR_PRINTF("%x", t),            \
    char* : BVR_PRINTF("%s", t),            \
    short : BVR_PRINTF("%i", t),            \
    int : BVR_PRINTF("%i", t),              \
    long : BVR_PRINTF("%i", t),             \
    unsigned short : BVR_PRINTF("%u", t),   \
    unsigned int : BVR_PRINTF("%u", t),     \
    unsigned long : BVR_PRINTF("%u", t),    \
    double : BVR_PRINTF("%f", t),           \
    float : BVR_PRINTF("%f", t),            \
    default : BVR_PRINTF("%x", t)           \
)
#else 
#define BVR_PRINT(t) do {} while (0);
#endif

// depreciated
// #define BVR_PRINTSTR(message)(void)(bvri_wmessage(bvr_stdout, __LINE__, __FILE__, message))

#if defined(BVR_DEBUG)
#ifndef BVR_ASSERT_FORCE_EXIT
    /**
     * @brief check if an expression is true. If the expression is false, 
     * it will force the program to exit.
     * @param expression the expression tested.
     */
    #define BVR_ASSERT(expression) (void) (                                         \
        (((expression) == 0) ? bvri_wassert_break(#expression, __FILE__, __LINE__) : 0)  \
    )
#elif
    /**
     * @brief check if an expression is true. If the expression is false, 
     * it will add a break point and break.
     * @param expression the expression tested.
     */
    #define BVR_ASSERT(expression) (void) (                                         \
        (((expression) == 0) ? bvri_wassert(#expression, __FILE__, __LINE__) : 0)  \
    )
#endif
#else 
#define BVR_ASSERT(expression) (                                         \
        (((expression) == 0) ? bvri_break(__FILE__, __LINE__) : 0)  \
    )
#endif

#define BVR_FILE_EXISTS(path) (void)(((access(path, F_OK) == 0) ? 0 : bvri_wassert(path, __FILE__, __LINE__)))

#define BVR_BREAK() (void)(bvri_break(__FILE__, __LINE__))

#endif