#ifndef BVR_H_IMPL
#define BVR_H_IMPL

#include <BVR/config.h>
#include <BVR/common.h>
#include <BVR/math.h>

#include <BVR/scene.h>
#include <BVR/assets.h>
#include <BVR/assets.book.h>

#ifndef BVR_NO_NUKLEAR
    #define NK_INCLUDE_FIXED_TYPES 
    #include <nuklear.h>

    #include <BVR/gui.h>
    
#endif

#endif