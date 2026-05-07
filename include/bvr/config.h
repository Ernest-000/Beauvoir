#pragma once

#ifndef BVR_VERSION
    #define BVR_VERSION "v0.1"
#endif

#ifdef BVR_NO_INLINE
    #define BVR_H_FUNC static
#else
    #define BVR_H_FUNC static inline
#endif

#include <stdbool.h>

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

typedef float float32;

#define BVR_INT8_MIN   (-0x80)
#define BVR_INT8_MAX   (0x7F)
#define BVR_UINT8_MIN  (0x00)
#define BVR_UINT8_MAX  (0xFF)

#define BVR_INT16_MIN  (-0x8000)
#define BVR_INT16_MAX  (0x7FFF)
#define BVR_UINT16_MIN (0x0000)
#define BVR_UINT16_MAX (0xFFFF)

#define BVR_INT32_MIN  (-0x80000000)
#define BVR_INT32_MAX  (0x7FFFFFFF)
#define BVR_UINT32_MIN (0x00000000U)
#define BVR_UINT32_MAX (0xFFFFFFFFU)

#define BVR_INT64_MIN  (-0x8000000000000000LL)
#define BVR_INT64_MAX  (0x7FFFFFFFFFFFFFFFLL)
#define BVR_UINT64_MIN (0x0000000000000000ULL)
#define BVR_UINT64_MAX (0xFFFFFFFFFFFFFFFFULL)

#if defined(__clang__)
    #define typeof(x) __typeof__(x)
#elif defined(_MSC_VER)
    #define typeof(x) __typeof__(x)
#else
    #define typeof(x) __typeof(x)
#endif

/**
 * Original code from:
 * https://stackoverflow.com/questions/3553296/sizeof-single-struct-member-in-c
 */
#if !defined(sizeof_member)
  #define sizeof_member(_struct, _member) (sizeof(((_struct *)0)->_member))
#endif

#pragma region bitswap

/* Macros to swap the order of bytes in integer values.
   Copyright (C) 1997, 1998, 2000, 2002, 2003, 2007, 2008
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */
/* Swap bytes in 16 bit value.  */

#define __bswap_constant_16(x) \
     ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))

     
#define __bswap_constant_32(x) \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |                      \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

/*
#if defined __GNUC__ && __GNUC__ >= 2
# define __bswap_16(x) \
                                                                  \
      ({ register unsigned short int __v, __x = (x);                              \
         if (__builtin_constant_p (__x))                                      \
           __v = __bswap_constant_16 (__x);                                      \
         else                                                                      \
           __asm__ ("rorw $8, %w0"                                              \
                    : "=r" (__v)                                              \
                    : "0" (__x)                                                      \
                    : "cc");                                                      \
         __v; })
#else
// This is better than nothing.
# define __bswap_16(x) \
                                                                  \
      ({ register unsigned short int __x = (x); __bswap_constant_16 (__x); })
#endif


// Swap bytes in 32 bit value.

#if defined __GNUC__ && __GNUC__ >= 2
# if __WORDSIZE == 64 || (defined __i486__ || defined __pentium__              \
                          || defined __pentiumpro__ || defined __pentium4__   \
                          || defined __k8__ || defined __athlon__              \
                          || defined __k6__ || defined __nocona__              \
                          || defined __core2__ || defined __geode__              \
                          || defined __amdfam10__)
// To swap the bytes in a word the i486 processors and up provide the
//   `bswap' opcode.  On i386 we have to use three instructions.  
#  define __bswap_32(x) \
                                                                  \
      ({ register unsigned int __v, __x = (x);                                      \
         if (__builtin_constant_p (__x))                                      \
           __v = __bswap_constant_32 (__x);                                      \
         else                                                                      \
           __asm__ ("bswap %0" : "=r" (__v) : "0" (__x));                      \
         __v; })
# else
#  define __bswap_32(x)                                                              \
                                                                  \
      ({ register unsigned int __v, __x = (x);                                      \
         if (__builtin_constant_p (__x))                                      \
           __v = __bswap_constant_32 (__x);                                      \
         else                                                                      \
           __asm__ ("rorw $8, %w0;"                                              \
                    "rorl $16, %0;"                                              \
                    "rorw $8, %w0"                                              \
                    : "=r" (__v)                                              \
                    : "0" (__x)                                                      \
                    : "cc");                                                      \
         __v; })  
# endif
#else
# define __bswap_32(x) \
                                                                  \
      ({ register unsigned int __x = (x); __bswap_constant_32 (__x); })
#endif
*/