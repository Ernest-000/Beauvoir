#include <BVR/camera.h>

#include <BVR/scene.h>

static void bvri_create_orthocamera(bvr_camera_t* camera, bvr_framebuffer_t* framebuffer, float near, float far, float scale);
static void bvri_create_perspcamera(bvr_camera_t* camera, bvr_framebuffer_t* framebuffer, float near, float far, float fov);

void bvr_camera_lookat(bvr_page_t* page, const vec3 target, const vec3 y){
    mat4x4 view;
    BVR_IDENTITY_MAT4(view);
    
    vec3 fwd, side, up;
    vec3_sub(fwd, target, page->camera.transform.position);
    vec3_norm(fwd, fwd);
    
    vec3_mul_cross(side, fwd, y);
    
    vec3_mul_cross(up, side, fwd);
    vec3_norm(up, up);

    view[0][0] = side[0];
    view[1][0] = side[1];
    view[2][0] = side[2];
    view[3][0] = -vec3_dot(side, page->camera.transform.position);
    view[0][1] = up[0];
    view[1][1] = up[1];
    view[2][1] = up[2];
    view[3][1] = -vec3_dot(up, page->camera.transform.position);
    view[0][2] = -fwd[0];
    view[1][2] = -fwd[1];
    view[2][2] = -fwd[2];
    view[3][2] = -vec3_dot(fwd, page->camera.transform.position);
    
    bvr_camera_set_view(page, view);
}

static void bvri_create_orthocamera(bvr_camera_t* camera, bvr_framebuffer_t* framebuffer, float near, float far, float scale){

}

static void bvri_create_perspcamera(bvr_camera_t* camera, bvr_framebuffer_t* framebuffer, float near, float far, float fov){

}