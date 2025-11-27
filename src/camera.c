#include <BVR/camera.h>

#include <BVR/shader.h>
#include <BVR/scene.h>

static void bvri_update_view(bvr_camera_t* camera, mat4x4 matrix);

static bvr_camera_t* bvri_create_orthocamera(bvr_camera_t* camera, bvr_framebuffer_t* framebuffer, float near, float far, float scale);
static bvr_camera_t* bvri_create_perspcamera(bvr_camera_t* camera, bvr_framebuffer_t* framebuffer, float near, float far, float fov);

static void bvri_update_orthocamera(bvr_camera_t* camera);
static void bvri_update_perspcamera(bvr_camera_t* camera);

void bvr_camera_lookat(bvr_camera_t* camera, const vec3 target, const vec3 y){
    mat4x4 view;
    BVR_IDENTITY_MAT4(view);
    
    vec3 fwd, side, up;
    vec3_sub(fwd, target, camera->transform.position);
    vec3_norm(fwd, fwd);
    
    vec3_mul_cross(side, fwd, y);
    
    vec3_mul_cross(up, side, fwd);
    vec3_norm(up, up);

    view[0][0] = side[0];
    view[1][0] = side[1];
    view[2][0] = side[2];
    view[3][0] = -vec3_dot(side, camera->transform.position);
    view[0][1] = up[0];
    view[1][1] = up[1];
    view[2][1] = up[2];
    view[3][1] = -vec3_dot(up, camera->transform.position);
    view[0][2] = -fwd[0];
    view[1][2] = -fwd[1];
    view[2][2] = -fwd[2];
    view[3][2] = -vec3_dot(fwd, camera->transform.position);
    
    bvri_update_view(camera, view);
}

void bvr_screen_to_world(bvr_camera_t* camera, vec2 p_screen, vec3 p_world){
    BVR_ASSERT(camera);

    if(!vec2_dot(p_screen, p_screen)){
        return;
    }

    vec4 world, screen;
    mat4x4 projection, view, inv;
    
    bvr_enable_uniform_buffer(camera->buffer);

    mat4x4* buffer_ptr = bvr_uniform_buffer_map(0, 2 * sizeof(mat4x4));
    if(buffer_ptr){
        memcpy(projection, &buffer_ptr[0], sizeof(mat4x4));
        memcpy(view, &buffer_ptr[1], sizeof(mat4x4));

        bvr_uniform_buffer_close();
        bvr_enable_uniform_buffer(0);

        screen[0] = 2.0f * (p_screen[0] / (camera->framebuffer->width)) - 1.0f;
        screen[1] = 1.0f - 2.0f * (p_screen[1] / (camera->framebuffer->height));
        screen[2] = 0.0f;
        screen[3] = 1.0f;
        
        mat4_mul(projection, projection, view);
        mat4_inv(inv, projection);
        mat4_mul_vec4(world, inv, screen);

        p_world[3] = 1.0f / world[3] * 2.0f;
        p_world[0] = world[0] * world[3];
        p_world[1] = world[1] * world[3];
        p_world[2] = world[2] * world[3];
        return;
    }

    bvr_uniform_buffer_close();

    p_world[0] = 0.0f;
    p_world[1] = 0.0f;
    p_world[2] = 0.0f;
}

void bvr_create_camera(bvr_camera_t* camera, const bvr_framebuffer_t* target, int mode, float near, float far, float scale){
    BVR_ASSERT(camera);
    BVR_ASSERT(target);

    camera->mode = mode;
    camera->framebuffer = target;
    camera->near = near;
    camera->far = far;
    camera->field_of_view.scale = scale;
    
    BVR_IDENTITY_VEC3(camera->transform.position);
    BVR_IDENTITY_VEC4(camera->transform.rotation);
    BVR_IDENTITY_VEC3(camera->transform.scale);
    BVR_IDENTITY_MAT4(camera->transform.matrix);

    bvr_create_uniform_buffer(&camera->buffer, 2 * sizeof(mat4x4), BVR_UNIFORM_BLOCK_CAMERA);
}

void bvr_update_camera(bvr_camera_t* camera){
    if(!camera || !camera->framebuffer){
        BVR_PRINT("missing camera!");
        return;
    }

    if(!camera->buffer){
        BVR_PRINT("failed to find camera's uniform buffer");
        return;
    }

    mat4x4 view;
    vec3 front, right, up, side;
    BVR_IDENTITY_MAT4(view);

    // calculate directionds
    const vec3 up_axis = {0.0f, 1.0f, 0.0f};
    front[0] = cos(deg_to_rad(camera->transform.rotation[2]) + M_PI_2) * cos(deg_to_rad(camera->transform.rotation[1]));
    front[1] = sin(deg_to_rad(camera->transform.rotation[1]));
    front[2] = sin(deg_to_rad(camera->transform.rotation[2]) + M_PI_2) * cos(deg_to_rad(camera->transform.rotation[1]));

    // normalize direction
    vec3_norm(front, front);

    vec3_mul_cross(right, front, up_axis);
    vec3_norm(right, right);

    vec3_mul_cross(up, right, front);
    vec3_norm(up, up);
    vec3_norm(front, front);

    vec3_mul_cross(side, front, up);
    vec3_norm(side, side);

    vec3_mul_cross(up, side, front);
    vec3_norm(up, up);

    view[0][0] = side[0];
    view[1][0] = side[1];
    view[2][0] = side[2];
    view[3][0] = -vec3_dot(side, camera->transform.position);
    view[0][1] = up[0];
    view[1][1] = up[1];
    view[2][1] = up[2];
    view[3][1] = -vec3_dot(up, camera->transform.position);
    view[0][2] = -front[0];
    view[1][2] = -front[1];
    view[2][2] = -front[2];
    view[3][2] = vec3_dot(front, camera->transform.position);
    view[0][3] = 0.0f;
    view[1][3] = 0.0f;
    view[1][3] = 0.0f;
    view[3][3] = 1.0f;

    bvr_enable_uniform_buffer(camera->buffer);
    bvr_uniform_buffer_set(sizeof(mat4x4), sizeof(mat4x4), &view[0][0]);

    // update each projection depending on camera's mode
    if(camera->mode == BVR_CAMERA_ORTHOGRAPHIC){
        bvri_update_orthocamera(camera);
    }
    else {
        bvri_update_perspcamera(camera);
    }

    bvr_enable_uniform_buffer(0);
}

static void bvri_update_view(bvr_camera_t* camera, mat4x4 matrix) {
    bvr_enable_uniform_buffer(camera->buffer);
    bvr_uniform_buffer_set(sizeof(mat4x4), sizeof(mat4x4), &matrix[0][0]);
    bvr_enable_uniform_buffer(0);
}

static bvr_camera_t* bvri_create_orthocamera(bvr_camera_t* camera, bvr_framebuffer_t* framebuffer, float near, float far, float scale){
    // not used for now
    return camera;
}

static bvr_camera_t* bvri_create_perspcamera(bvr_camera_t* camera, bvr_framebuffer_t* framebuffer, float near, float far, float fov){
    // not used for now
    return NULL;
}

static void bvri_update_orthocamera(bvr_camera_t* camera){
    mat4x4 projection;
    BVR_IDENTITY_MAT4(projection);

    float width =   1.0f / camera->framebuffer->target_width * camera->field_of_view.scale;
    float height =  1.0f / camera->framebuffer->target_height * camera->field_of_view.scale;
    float farnear = 1.0f / (camera->far - camera->near);

    projection[0][0] = 2.0f * -width;
    projection[1][1] = 2.0f * height;
    projection[2][2] = farnear;
    projection[3][0] = -width;
    projection[3][1] = -height;
    projection[3][2] = -camera->near * farnear;
    projection[3][3] =  1.0f;

    bvr_uniform_buffer_set(0, sizeof(mat4x4), &projection[0][0]);
}

static void bvri_update_perspcamera(bvr_camera_t* camera){
    mat4x4 projection;
    BVR_IDENTITY_MAT4(projection);

    bvr_uniform_buffer_set(0, sizeof(mat4x4), &projection[0][0]);
}