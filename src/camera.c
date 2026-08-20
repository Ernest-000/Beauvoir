#include <bvr/camera.h>

#include <bvr/shader.h>
#include <bvr/book.h>

#include <bvr/gl.h>

/**
 * @brief set the view buffer with a new matrix.
 */
static inline void bvri_set_view_buffer(bvr_camera_t* camera, mat4x4 view){
    glBindBuffer(GL_UNIFORM_BUFFER, camera->ortho.buffer_object);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4x4), sizeof(mat4x4), &view[0][0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

/**
 * @brief set the projection buffer with a new matrix.
 */
static inline void bvri_set_proj_buffer(bvr_camera_t* camera, mat4x4 proj){
    glBindBuffer(GL_UNIFORM_BUFFER, camera->ortho.buffer_object);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4x4), &proj[0][0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

/**
 * @brief calculate the orthohgraphic projection matrix and push it 
 * to the uniform buffer object.
 */
static void bvri_calc_ortho(bvr_camera_t* camera){
    mat4x4 projection;
    BVR_IDENTITY_MAT4(projection);

    float width =   1.0f / camera->ortho.width * camera->ortho.scale;
    float height =  1.0f / camera->ortho.height * camera->ortho.scale;
    float farnear = 1.0f / (camera->ortho.far - camera->ortho.near);

    projection[0][0] = 2.0f * -width;
    projection[1][1] = 2.0f * height;
    projection[2][2] = farnear;
    projection[3][0] = -width;
    projection[3][1] = -height;
    projection[3][2] = -camera->ortho.near * farnear;
    projection[3][3] =  1.0f;

    bvri_set_proj_buffer(camera, projection);
}

/**
 * @brief calculate the perpective projection matrix and push it 
 * to the uniform buffer object.
 */
static void bvri_calc_persp(bvr_camera_t* camera){
    mat4x4 projection;
    BVR_IDENTITY_MAT4(projection);
    // TODO: implement
    bvri_set_proj_buffer(camera, projection);
}

void bvr_camera_lookat(bvr_camera_t* camera, const vec3 target, const vec3 y){
    mat4x4 view;
    BVR_IDENTITY_MAT4(view);
    
    vec3 fwd, side, up;
    vec3_sub(fwd, target, camera->ortho.transform.position);
    vec3_norm(fwd, fwd);
    
    vec3_mul_cross(side, fwd, y);
    
    vec3_mul_cross(up, side, fwd);
    vec3_norm(up, up);

    view[0][0] = side[0];
    view[1][0] = side[1];
    view[2][0] = side[2];
    view[3][0] = -vec3_dot(side, camera->ortho.transform.position);
    view[0][1] = up[0];
    view[1][1] = up[1];
    view[2][1] = up[2];
    view[3][1] = -vec3_dot(up, camera->ortho.transform.position);
    view[0][2] = -fwd[0];
    view[1][2] = -fwd[1];
    view[2][2] = -fwd[2];
    view[3][2] = -vec3_dot(fwd, camera->ortho.transform.position);
    
    bvri_set_view_buffer(camera, view);
}

void bvr_create_ortho_camera(bvr_camera_t* camera, float width, float height, float near, float far, float scale){
    BVR_ASSERT(camera);

    camera->ortho.mode = BVR_CAMERA_ORTHOGRAPHIC;
    camera->ortho.width = width;
    camera->ortho.height = height;
    camera->ortho.near = near;
    camera->ortho.far = far;
    camera->ortho.scale = scale;

    BVR_IDENTITY_VEC3(camera->ortho.transform.position);
    BVR_IDENTITY_VEC4(camera->ortho.transform.rotation);
    BVR_IDENTITY_VEC3(camera->ortho.transform.scale);
    BVR_IDENTITY_MAT4(camera->ortho.transform.local);
    BVR_IDENTITY_MAT4(camera->ortho.transform.world);

    // create a new uniform buffer
    // might want to check if a previous ubo for camera exists?
    glGenBuffers(1, &camera->ortho.buffer_object);
    glBindBuffer(GL_UNIFORM_BUFFER, camera->ortho.buffer_object);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(mat4x4), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    // binding point
    glBindBufferRange(GL_UNIFORM_BUFFER, 
        BVR_UNIFORM_BLOCK_CAMERA, 
        camera->ortho.buffer_object, 
        0, 2 * sizeof(mat4x4)
    );
}

void bvr_create_persp_camera(bvr_camera_t* camera, float width, float height, float near, float far, float fov){
    BVR_ASSERT(camera);

    camera->perspective.mode = BVR_CAMERA_ORTHOGRAPHIC;
    camera->perspective.aspect = height / width;
    camera->perspective.near = near;
    camera->perspective.far = far;
    camera->perspective.fov = fov;

    BVR_IDENTITY_VEC3(camera->perspective.transform.position);
    BVR_IDENTITY_VEC4(camera->perspective.transform.rotation);
    BVR_IDENTITY_VEC3(camera->perspective.transform.scale);
    BVR_IDENTITY_MAT4(camera->perspective.transform.local);
    BVR_IDENTITY_MAT4(camera->perspective.transform.world);

    // create a new uniform buffer
    // might want to check if a previous ubo for camera exists?
    glGenBuffers(1, &camera->perspective.buffer_object);
    glBindBuffer(GL_UNIFORM_BUFFER, camera->perspective.buffer_object);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(mat4x4), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    // binding point
    glBindBufferRange(GL_UNIFORM_BUFFER, 
        BVR_UNIFORM_BLOCK_CAMERA, 
        camera->perspective.buffer_object, 
        0, 2 * sizeof(mat4x4)
    );
}

void bvr_update_camera(bvr_camera_t* camera){
    if(camera == NULL || camera->ortho.buffer_object == 0){
        BVR_PRINT("missing camera");
        return;
    }

    mat4x4 view;
    vec3 front, right, up, side;
    BVR_IDENTITY_MAT4(view);

    // calculate the direction to get the front vector
    const vec3 up_axis = {0.0f, 1.0f, 0.0f};
    front[0] = cos(deg_to_rad(camera->ortho.transform.rotation[2]) + BVR_HALF_PI) * cos(deg_to_rad(camera->ortho.transform.rotation[1]));
    front[1] = sin(deg_to_rad(camera->ortho.transform.rotation[1]));
    front[2] = sin(deg_to_rad(camera->ortho.transform.rotation[2]) + BVR_HALF_PI) * cos(deg_to_rad(camera->ortho.transform.rotation[1]));

    // normalize direction
    vec3_norm(front, front);

    // calculate the right
    vec3_mul_cross(right, front, up_axis);
    vec3_norm(right, right);

    // find the up
    vec3_mul_cross(up, right, front);
    vec3_norm(up, up);
    vec3_norm(front, front);

    // get the side vector
    vec3_mul_cross(side, front, up);
    vec3_norm(side, side);

    vec3_mul_cross(up, side, front);
    vec3_norm(up, up);

    // create the matrix
    view[0][0] = side[0];
    view[1][0] = side[1];
    view[2][0] = side[2];
    view[3][0] = -vec3_dot(side, camera->ortho.transform.position);
    view[0][1] = up[0];
    view[1][1] = up[1];
    view[2][1] = up[2];
    view[3][1] = -vec3_dot(up, camera->ortho.transform.position);
    view[0][2] = -front[0];
    view[1][2] = -front[1];
    view[2][2] = -front[2];
    view[3][2] = vec3_dot(front, camera->ortho.transform.position);
    view[0][3] = 0.0f;
    view[1][3] = 0.0f;
    view[1][3] = 0.0f;
    view[3][3] = 1.0f;

    bvri_set_view_buffer(camera, view);

    if(camera->ortho.mode == BVR_CAMERA_ORTHOGRAPHIC){
        bvri_calc_ortho(camera);
    }
    else {
        bvri_calc_persp(camera);
    }
}

void bvr_screen_to_world(bvr_camera_t* camera, vec2 p_screen, vec3 p_world){
    BVR_ASSERT(camera);

    if(camera->perspective.mode == BVR_CAMERA_PERSPECTIVE){
        BVR_PRINT("invalid feature for perspective camera");
        return;
    }

    if(!vec2_dot(p_screen, p_screen)){
        return;
    }

    vec4 world, screen;
    mat4x4 projection, view, inv;
    
    glBindBuffer(GL_UNIFORM_BUFFER, camera->ortho.buffer_object);

    mat4x4* buffer_ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, 2 * sizeof(mat4x4), GL_MAP_READ_BIT);
    if(buffer_ptr){
        memcpy(projection, &buffer_ptr[0], sizeof(mat4x4));
        memcpy(view, &buffer_ptr[1], sizeof(mat4x4));

        glUnmapBuffer(GL_UNIFORM_BUFFER);
        glBindBuffer(GL_UNIFORM_BUFFER, camera->ortho.buffer_object);

        screen[0] = 2.0f * (p_screen[0] / camera->ortho.width) - 1.0f;
        screen[1] = 1.0f - 2.0f * (p_screen[1] / (camera->ortho.height));
        screen[2] = -1.0f;
        screen[3] = 1.0f;
        
        mat4_mul(projection, projection, view);
        mat4_inv(inv, projection);

        mat4_mul_vec4(world, inv, screen);

        p_world[0] = world[0] * world[3] / 2;
        p_world[1] = world[1] * world[3] / 2;
        p_world[2] = world[2] * world[3] / 2;
        
        return;
    }

    glUnmapBuffer(GL_UNIFORM_BUFFER);

    p_world[0] = 0.0f;
    p_world[1] = 0.0f;
    p_world[2] = 0.0f;
}

void bvr_world_to_screen(bvr_camera_t* camera, vec3 p_world, vec2 screen){
    BVR_ASSERT(camera);

    if(camera->perspective.mode == BVR_CAMERA_PERSPECTIVE){
        BVR_PRINT("invalid feature for perspective camera");
        return;
    }

    if(!vec3_dot(p_world, p_world)){
        return;
    }

    vec4 world;
    mat4x4 projection, view;
    
    glBindBuffer(GL_UNIFORM_BUFFER, camera->ortho.buffer_object);

    mat4x4* buffer_ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, 2 * sizeof(mat4x4), GL_MAP_READ_BIT);
    if(buffer_ptr){
        memcpy(projection, &buffer_ptr[0], sizeof(mat4x4));
        memcpy(view, &buffer_ptr[1], sizeof(mat4x4));

        glUnmapBuffer(GL_UNIFORM_BUFFER);

        world[0] = p_world[0];
        world[1] = p_world[1];
        world[2] = p_world[2];
        world[3] = 0.0f;

        mat4_mul(projection, projection, view);
        mat4_mul_vec4(world, projection, world);

        screen[0] = (world[0] + 0.5f) * camera->ortho.width;
        screen[1] = (world[1] + 0.5f) * camera->ortho.height;
        return;
    }

    glUnmapBuffer(GL_UNIFORM_BUFFER);

    screen[0] = 0.0f;
    screen[1] = 0.0f;
}