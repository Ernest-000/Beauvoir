#pragma once

#include <bvr/config.h>
#include <bvr/math.h>
#include <bvr/graphics.h>

#define BVR_CAMERA_ORTHOGRAPHIC 0x1
#define BVR_CAMERA_PERSPECTIVE  0x2

typedef union bvr_camera_u {
    struct bvr_camera_ortho_s {
        // that block MUST be aligned
        uint16 mode;
        bvr_transform_t transform;
        
        uint32 buffer_object;
        // end
    
        float width;
        float height;

        float near;
        float far;

        float scale;

    } __struct_align16 ortho;

    struct bvr_camera_perspective_s {
        // that block MUST be aligned
        uint16 mode;
        bvr_transform_t transform;
        
        uint32 buffer_object;
        // end
    
        float aspect;

        float near;
        float far;

        float fov;
    } __struct_align16 perspective;
} bvr_camera_t;

/**
 * @brief create a new orthographic camera
 * 
 * @param camera
 * @param width the viewport width
 * @param height the viewport height
 * @param near near plane distance
 * @param far far plane distance
 * @param scale the camera's viewport scale
 * 
 * @return (void)
 */
void bvr_create_ortho_camera(bvr_camera_t* camera, float width, float height, float near, float far, float scale);

/**
 * @brief create a new perspective camera
 * 
 * @param camera
 * @param width the viewport width
 * @param height the viewport height
 * @param near near plane distance
 * @param far far plane distance
 * @param fov the field of view of the camera
 * 
 * @return (void)
 */
void bvr_create_persp_camera(bvr_camera_t* camera, float width, float height, float near, float far, float fov);

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
void bvr_camera_lookat(bvr_camera_t* camera, const vec3 target, const vec3 up);

/**
 * @brief transform a screen coordinate to a world coordinate
 * @param camera
 * @param screen
 * @param world
 * @return (void)
 */
void bvr_screen_to_world(bvr_camera_t* camera, vec2 screen, vec3 world);

/**
 * @brief transform a world coordinate to a screen coordinate
 * @param camera
 * @param screen
 * @param world
 * @return (void)
 */
void bvr_world_to_screen(bvr_camera_t* camera, vec3 world, vec2 screen);