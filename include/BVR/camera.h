#pragma once

#include <BVR/config.h>
#include <BVR/math.h>
#include <BVR/graphics.h>

#define BVR_CAMERA_ORTHOGRAPHIC 0x1
#define BVR_CAMERA_PERSPECTIVE  0x2

typedef struct bvr_camera_s {
    uint32 mode;

    struct bvr_transform_s transform;
    bvr_framebuffer_t* framebuffer;
    uint32 buffer; /* uniform buffer object reference */
    
    float near;
    float far;
    union bvr_camera_field_of_view_u
    {
        // ortho scale
        float scale;

        // perspective scale
        float fov;
    } field_of_view;
} bvr_camera_t;

/**
 * @brief create a new camera
 * 
 * @param camera
 * @param target
 * @param mode ```BVR_CAMERA_ORTHOGRAPHIC``` or ```BVR_CAMERA_PERSPECTIVE```
 * @param near near plane distance
 * @param far far plane distance
 * @param scale fov or camera's scale (depending on camera's mode)
 * 
 * @return (void)
 */
void bvr_create_camera(bvr_camera_t* camera, const bvr_framebuffer_t* target, int mode, float near, float far, float scale);

/**
 * @brief update camera's view matrix
 * @param camera
 * @return (void)
 */
void bvr_update_camera(bvr_camera_t* camera);

/**
 * @brief rotate a camera so that it will look at target
 * @param 
 * @param target
 * @param up
 * @return (void)
 */
void bvr_camera_lookat(bvr_camera_t*, const vec3 target, const vec3 up);

/**
 * @brief transform a screen coordinate to a world coordinate
 * @param camera
 * @param screen
 * @param world
 * @return (void)
 */
void bvr_screen_to_world(bvr_camera_t* camera, vec2 screen, vec3 world);