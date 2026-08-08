#ifndef _H_LBVR_H
#define _H_LBVR_H

#ifndef NULL
#include <stddef.h>
#endif

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#ifdef _WIN32
typedef long long int64;
typedef unsigned long long uint64;
#else
typedef long int64;
typedef unsigned long uint64;
#endif

/**
 * @brief returns the number of frame since the program has started.
 */
extern uint64 bvr_get_frame(void);

/**
 * @brief returns the number of second between the current
 * frame and the previous frame.
 */
extern float bvr_get_delta_time(void);

#endif