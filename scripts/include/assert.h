#ifndef _ASSERT_H
#define _ASSERT_H

#include <stdio.h>

#define BVR_PRINT(message)(void)(bvri_wmessage(bvr_stdout, __LINE__, __FILE__, message))
#define BVR_PRINTF(message, ...)(void)(bvri_wmessage(bvr_stdout, __LINE__, __FILE__, message, __VA_ARGS__))
#define BVR_ASSERT(expression) (void) ((((expression) == 0) ? bvri_wassert(#expression, __FILE__, __LINE__) : 0))

extern void bvri_wmessage(FILE* __stream, const int __line, const char* __file, const char* __message, ...);
extern void bvri_wassert(const char* __message, const char* __file, unsigned long long __line);

#endif