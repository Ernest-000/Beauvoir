#ifndef BVR_H_IMPL
#define BVR_H_IMPL

#ifdef _cplusplus
extern "C" {
#endif

#include <bvr/config.h>
#include <bvr/common.h>
#include <bvr/math.h>

#include <bvr/camera.h>

#include <bvr/book.h>
#include <bvr/graphics.h>
#include <bvr/actors.h>
#include <bvr/animation.h>
#include <bvr/landscape.h>
#include <bvr/assets.h>

#ifndef BVR_NO_NUKLEAR
    #include <bvr/gui.h>
    
    #include <nuklear.h>
#endif

#ifdef _cplusplus
}
#endif

#endif