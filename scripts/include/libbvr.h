#ifndef _LIB_BVR_H
#define _LIB_BVR_H

#include <stdio.h>

extern void bvri_wmessage(FILE* __stream, const int __line, const char* __file, const char* __message, ...);
extern void bvri_wassert(const char* __message, const char* __file, unsigned long long __line);

#define BVR_PRINT(message)(void)(bvri_wmessage(stdout, __LINE__, __FILE__, message))
#define BVR_PRINTF(message, ...)(void)(bvri_wmessage(stdout, __LINE__, __FILE__, message, __VA_ARGS__))
#define BVR_ASSERT(expression) (void) (                                         \
    (((expression) == 0) ? bvri_wassert(#expression, __FILE__, __LINE__) : 0)  \
)

#endif