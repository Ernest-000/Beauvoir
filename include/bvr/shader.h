#pragma once

#include <bvr/common.h>
#include <bvr/buffer.h>

#include <string.h>
#include <stdint.h>

// Default GLSL's version
#ifndef BVR_SHADER_VERSION 
    #define BVR_SHADER_VERSION "#version 400\n"
#endif

#define BVR_UNIFORM_CAMERA_NAME "bvr_camera"
#define BVR_UNIFORM_TRANSFORM_NAME "bvr_transform"

#define BVR_UNIFORM_GLOBAL_ILLUMINATION_NAME "bvr_global_illumination"
#define BVR_UNIFORM_SHARE_LAYER_NAME "bvr_layers"

#define BVR_UNIFORM_BLOCK_CAMERA                0x0
#define BVR_UNIFORM_BLOCK_GLOBAL_ILLUMINATION   0x1
#define BVR_UNIFORM_BLOCK_LAYERS                0x2

#define BVR_MAX_SHADER_COUNT 3
#define BVR_MAX_UNIFORM_COUNT 20
#define BVR_MAX_SHADER_BLOCK_COUNT 5

#define BVR_VERTEX_SHADER               0x001
#define BVR_FRAGMENT_SHADER             0x002
#define BVR_FRAMEBUFFER_SHADER          0x010

#define BVR_SHADER_EXT_LIGHT            0x100
#define BVR_SHADER_EXT_SHARE_LAYERS     0x200

#define BVR_SHADER_EXT_GLOBAL_ILLUMINATION BVR_SHADER_EXT_LIGHT

enum bvr_uniform_tag_e {
    BVR_UNIFORM_NONE = 0x000,
    BVR_UNIFORM_PROJECTION = 0x001,
    BVR_UNIFORM_TRANSFORM = 0x002,
    BVR_UNIFORM_LOCAL_TRANSFORM = 0x003,
    BVR_UNIFORM_TEXTURE = 0x004,
    BVR_UNIFORM_LAYER_INDEX = 0x005,
    BVR_UNIFORM_LAYER_INFO = 0x006,
    BVR_UNIFORM_COMPOSITE = 0x007
};

typedef struct bvr_shader_uniform_s {
    struct bvr_buffer_s memory;

    bvr_string_t name;
    short location;

    uint16 type;
    uint16 tags;
} bvr_shader_uniform_t;

typedef struct bvr_shader_stage_s {
    uint32 shader;
    uint16 type;
} bvr_shader_stage_t;

typedef struct bvr_shader_block_s {
    short location;
    uint16 type;
    uint32 count;
} bvr_shader_block_t;

typedef struct bvr_shader_s {
    uint32 program;

    bvr_shader_stage_t shaders[BVR_MAX_SHADER_COUNT];
    bvr_shader_uniform_t uniforms[BVR_MAX_UNIFORM_COUNT];
    bvr_shader_block_t blocks[BVR_MAX_SHADER_BLOCK_COUNT];

    uint8 shader_count;
    uint8 uniform_count, block_count;
    
    int flags;
} bvr_shader_t;


int bvr_create_shaderf(bvr_shader_t* shader, FILE* file, const int flags);
static inline int bvr_create_shader(bvr_shader_t* shader, const char* path, const int flags){
    BVR_FILE_EXISTS(path);

    // open file stream
    FILE* file = fopen(path, "rb");
    int a = bvr_create_shaderf(shader, file, flags);
    fclose(file);
    return a;
} 

void bvr_create_uniform_buffer(uint32* buffer, uint64 size, uint32 binding_point);
void bvr_enable_uniform_buffer(uint32 buffer);
void bvr_uniform_buffer_set(uint32 offset, uint64 size, void* data);

void* bvr_uniform_buffer_map(uint32 offset, uint64 size);
void bvr_uniform_buffer_close();

void bvr_destroy_uniform_buffer(uint32* buffer);

/**
 * @brief Create a new shader directly from raw strings. 
 * Howerver, this function is primarly used for internal usage. 
 * You shall not use this function, use bvr_create_shader instead :3
 * WARN: no error handling
 * @param shader
 * @param args An array of strings. Order must be: vertex , fragment, geometry...
 * @param flags Define needed shaders.
 * @return 
 */
int bvr_create_shader_raw(bvr_shader_t* shader, const char** strings, const int flags);

/**
 * int bvri_create_shader_vert_frag(bvr_shader_t* shader, const char* vert, const char* frag);
*/

/**
 * @brief Bind a new uniform to a shader
 * @param shader
 * @param type uniform's type
 * @param bvr_uniform_tag_e uniform's tag
 * @param count number of values
 * @param name uniform's name. Must be the same as in the shader code
 * @return 
 */
bvr_shader_uniform_t* bvr_shader_register_uniform(bvr_shader_t* shader, int type, enum bvr_uniform_tag_e tag, int count, const char* name);
bvr_shader_uniform_t* bvr_shader_register_texture(bvr_shader_t* shader, int type, void* texture, const char* name);
bvr_shader_block_t* bvr_shader_register_block(bvr_shader_t* shader, const char* name, int type, int count, int index);

BVR_H_FUNC bvr_shader_uniform_t* bvr_find_uniform_tag(bvr_shader_t* shader, enum bvr_uniform_tag_e tag){
    for (uint64 i = 0; i < shader->uniform_count; i++)
    {
        if(shader->uniforms[i].tags == tag){
            return &shader->uniforms[i];
        }
    }

    return NULL;
}

BVR_H_FUNC bvr_shader_uniform_t* bvr_find_uniform(bvr_shader_t* shader, const char* name){
    for (uint64 i = 1; i < shader->uniform_count; i++)
    {
        if(!shader->uniforms[i].name.length){
            continue;
        }
        
        if (strncmp(shader->uniforms[i].name.string, name, shader->uniforms[i].name.length) == 0) {
            return &shader->uniforms[i];
        }
    }

    return NULL;
}

/**
 * @brief Bind a value to an uniform
 * @param uniform
 * @param data a pointer to the data that will be referenced as uniform's value
 * @return 
 */
int bvr_shader_set_uniform_raw(bvr_shader_uniform_t* uniform, void* data);
BVR_H_FUNC int bvr_shader_set_texture_raw(bvr_shader_uniform_t* uniform, void* texture){
    return bvr_shader_set_uniform_raw(uniform, texture);
}

/**
 * @brief Bind a value to an uniform
 * @param shader
 * @param name the name of a previously registered uniform
 * @param data a pointer to the data that will be referenced as uniform's value
 * @return 
 */
int bvr_shader_set_uniform(bvr_shader_t* shader, const char* name, void* data);

// DEPRECIATE
// /**
//  * @brief Bind a float value to an uniform
//  * @param shader
//  * @param name the name of a previously registered uniform
//  * @param data a pointer to the data that will be referenced as uniform's value
//  * @return 
//  */
// BVR_H_FUNC int bvr_shader_set_uniform_f(bvr_shader_t* shader, const char* name, float value){
//     return bvr_shader_set_uniform(shader, name, &value);
// }// 

// /**
//  * @brief Bind an int value to an uniform
//  * @param shader
//  * @param name the name of a previously registered uniform
//  * @param data a pointer to the data that will be referenced as uniform's value
//  * @return 
//  */
// BVR_H_FUNC int bvr_shader_set_uniform_i(bvr_shader_t* shader, const char* name, int value){
//     return bvr_shader_set_uniform(shader, name, &value);
// }

BVR_H_FUNC int bvr_shader_set_texture(bvr_shader_t* shader, const char* name, void* texture){
    return bvr_shader_set_uniform_raw(bvr_find_uniform(shader, name), texture);
}

void bvr_shader_use_uniform(bvr_shader_uniform_t* uniform, void* data);

void bvr_shader_enable(bvr_shader_t* shader);
void bvr_shader_disable(void);
void bvr_destroy_shader(bvr_shader_t* shader);