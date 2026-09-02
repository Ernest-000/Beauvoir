#pragma once

#ifndef BVR_CLASS_NAME
    #define BVR_CLASS_NAME "BEAUVOIR"
#endif

#ifndef BVR_VERSION
    #define BVR_VERSION "v0.1"
#endif

#ifndef BVR_TARGET_FRAME_PER_SEC
    #define BVR_TARGET_FRAME_PER_SEC 60
#endif

#ifndef BVR_TARGET_FRAMERATE
    #define BVR_TARGET_FRAMERATE 60
#endif

#ifdef BVR_NO_INLINE
    #define BVR_H_FUNC static
#else
    #define BVR_H_FUNC static inline
#endif

#include <stdbool.h>

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

typedef float float32;

#define BVR_INT8_MIN   (-0x80)
#define BVR_INT8_MAX   (0x7F)
#define BVR_UINT8_MIN  (0x00)
#define BVR_UINT8_MAX  (0xFF)

#define BVR_INT16_MIN  (-0x8000)
#define BVR_INT16_MAX  (0x7FFF)
#define BVR_UINT16_MIN (0x0000)
#define BVR_UINT16_MAX (0xFFFF)

#define BVR_INT24_MAX (0x7FFFFF)

#define BVR_INT32_MIN  (-0x80000000)
#define BVR_INT32_MAX  (0x7FFFFFFF)
#define BVR_UINT32_MIN (0x00000000U)
#define BVR_UINT32_MAX (0xFFFFFFFFU)

#define BVR_INT64_MIN  (-0x8000000000000000LL)
#define BVR_INT64_MAX  (0x7FFFFFFFFFFFFFFFLL)
#define BVR_UINT64_MIN (0x0000000000000000ULL)
#define BVR_UINT64_MAX (0xFFFFFFFFFFFFFFFFULL)

/**
 * define compiler specific macros  
*/ 

#if defined(__clang__)
    #define typeof(x) __typeof__(x)

    #define __struct_align16 __attribute__((aligned(16)))
    #define __struct_align4 __attribute__((aligned(4)))
#elif defined(_MSC_VER)
    #define typeof(x) __typeof__(x)

    #define __struct_align16 __declspec(align(#))
    #define __struct_align4 __declspec(align(#))
#else
    #define typeof(x) __typeof(x)

    #define __struct_align16 __attribute__((aligned(16)))
    #define __struct_align4 __attribute__((aligned(4)))
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
