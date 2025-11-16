#pragma once

#include <BVR/config.h>
#include <BVR/math.h>

typedef enum bvr_light_type_e {
    BVR_LIGHT_NONE,
    BVR_LIGHT_GLOBAL_ILLUMINATION
} bvr_light_type_t;

struct bvr_light_s {
    bvr_light_type_t type;

    vec4 position;
    vec4 direction;
    
    float intensity;
    vec3 color;
};

typedef struct bvr_global_illumination_s {
    /*
        position.w -> ambiant intensity
        direction.w -> 
    */
    struct bvr_light_s light;
    uint32 buffer;
} bvr_global_illumination_t;