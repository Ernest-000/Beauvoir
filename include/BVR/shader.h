#pragma once

#include <BVR/utils.h>
#include <BVR/buffer.h>

#include <BVR/assets.h>

#include <stdint.h>

#define BVR_UNIFORM_CAMERA_NAME "bvr_camera"
#define BVR_UNIFORM_TRANSFORM_NAME "bvr_transform"
#define BVR_UNIFORM_GLOBAL_ILLUMINATION_NAME "bvr_global_illumination"

#define BVR_UNIFORM_BLOCK_CAMERA                0x0
#define BVR_UNIFORM_BLOCK_GLOBAL_ILLUMINATION   0x1

#define BVR_MAX_SHADER_COUNT 7
#define BVR_MAX_UNIFORM_COUNT 20
#define BVR_MAX_SHADER_BLOCK_COUNT 5

#define BVR_VERTEX_SHADER 0x001
#define BVR_FRAGMENT_SHADER 0x002
#define BVR_FRAMEBUFFER_SHADER 0x004

typedef struct bvr_shader_uniform_s {
    struct bvr_buffer_s memory;

    bvr_string_t name;
    short location;
    int type;
} bvr_shader_uniform_t;

typedef struct bvr_shader_stage_s {
    uint32 shader;
    int type;
} bvr_shader_stage_t;

typedef struct bvr_shader_block_s {
    short location;
    uint16 type;
    uint32 count;
} bvr_shader_block_t;

typedef struct bvr_shader_s {
    uint32 program;

    bvr_shader_stage_t shaders[BVR_MAX_SHADER_COUNT] __attribute__ ((packed));
    bvr_shader_uniform_t uniforms[BVR_MAX_UNIFORM_COUNT] __attribute__ ((packed));
    bvr_shader_block_t blocks[BVR_MAX_SHADER_BLOCK_COUNT] __attribute__ ((packed));

    uint8 shader_count;
    uint8 uniform_count, block_count;
    
    int flags;
    struct bvr_asset_reference_s asset;
} bvr_shader_t;


int bvr_create_shaderf(bvr_shader_t* shader, FILE* file, const int flags);
static inline int bvr_create_shader(bvr_shader_t* shader, const char* path, const int flags){
    BVR_FILE_EXISTS(path);

    // link to an asset
    bvr_uuid_t* id = bvr_register_asset(path, BVR_OPEN_READ);
    if(id){
        shader->asset.origin = BVR_ASSET_ORIGIN_PATH;
        bvr_copy_uuid(*id, shader->asset.pointer.asset_id);
    }

    // open file stream
    FILE* file = fopen(path, "rb");
    int a = bvr_create_shaderf(shader, file, flags);
    fclose(file);
    return a;
} 

void bvr_create_uniform_buffer(uint32* buffer, uint64 size);
void bvr_enable_uniform_buffer(uint32 buffer);
void bvr_uniform_buffer_set(uint32 offset, uint64 size, void* data);

void* bvr_uniform_buffer_map(uint32 offset, uint64 size);
void bvr_uniform_buffer_close();

void bvr_destroy_uniform_buffer(uint32* buffer);

/*
    Create an empty shader only with vertex and fragment shaders.
    Internal usages only.

    TODO: try to find another way to define this function
*/
int bvri_create_shader_vert_frag(bvr_shader_t* shader, const char* vert, const char* frag);

/*
    Bind a new shader uniform.
*/
bvr_shader_uniform_t* bvr_shader_register_uniform(bvr_shader_t* shader, int type, int count, const char* name);
bvr_shader_uniform_t* bvr_shader_register_texture(bvr_shader_t* shader, int type, void* texture, const char* name);
bvr_shader_block_t* bvr_shader_register_block(bvr_shader_t* shader, const char* name, int type, int count, int index);

BVR_H_FUNC bvr_shader_uniform_t* bvr_find_uniform(bvr_shader_t* shader, const char* name){
    for (uint64 i = 1; i < shader->uniform_count; i++)
    {
        if(!shader->uniforms[i].name.length){
            continue;
        }
        
        if (strcmp(shader->uniforms[i].name.string, name) == 0) {
            return &shader->uniforms[i];
        }
    }

    return NULL;
}

void bvr_shader_set_uniformi(bvr_shader_uniform_t* uniform, void* data);
void bvr_shader_set_texturei(bvr_shader_uniform_t* uniform, void* texture);

void bvr_shader_set_uniform(bvr_shader_t* shader, const char* name, void* data);
BVR_H_FUNC void bvr_shader_set_texture(bvr_shader_t* shader, const char* name, void* texture){
    bvr_shader_set_texturei(bvr_find_uniform(shader, name), texture);
}

void bvr_shader_use_uniform(bvr_shader_uniform_t* uniform, void* data);


void bvr_shader_enable(bvr_shader_t* shader);
void bvr_shader_disable(void);
void bvr_destroy_shader(bvr_shader_t* shader);