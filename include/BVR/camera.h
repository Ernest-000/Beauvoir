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

/*
    Rotate the camera so that it will look at target
*/
void bvr_camera_lookat(bvr_page_t* page, const vec3 target, const vec3 up);