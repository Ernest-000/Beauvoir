#pragma once

#include <BVR/math.h>
#include <BVR/buffer.h>

#include <BVR/mesh.h>
#include <BVR/shader.h>
#include <BVR/image.h>

#define BVR_BLEND_DISABLE   0x000
#define BVR_BLEND_ENABLE    0x001

// Cs*As+Cd*(1-As)
#define BVR_BLEND_FUNC_ALPHA_ONE_MINUS  0x002

// Cd=Cs*1+Cd*1
#define BVR_BLEND_FUNC_ALPHA_ADD        0x004

// Cd=Cs*Cd+Cd*0
#define BVR_BLEND_FUNC_ALPHA_MULT       0x008

#define BVR_DEPTH_TEST_DISABLE  0x000
#define BVR_DEPTH_TEST_ENABLE   0x001

#define BVR_DEPTH_FUNC_NEVER    0x002
#define BVR_DEPTH_FUNC_ALWAYS   0x004
#define BVR_DEPTH_FUNC_LESS     0x008
#define BVR_DEPTH_FUNC_GREATER  0x010
#define BVR_DEPTH_FUNC_LEQUAL   0x020
#define BVR_DEPTH_FUNC_GEQUAL   0x040
#define BVR_DEPTH_FUNC_NOTEQUAL 0x080
#define BVR_DEPTH_FUNC_EQUAL    0x100

#define BVR_MAX_DRAW_COMMAND 258

struct bvr_pipeline_state_s {
    short blending;
    short depth;
    int flags;
};

struct bvr_draw_command_s {
    short order;

    uint32 array_buffer;
    uint32 vertex_buffer;
    uint32 element_buffer;

    uint16 element_offset;
    uint32 element_count;
    uint8 attrib_count;
    
    uint8 draw_mode;

    uint16 texture_type;

    bvr_shader_t* shader;
    bvr_texture_t* texture;    

    void* user_data;
} __attribute__ ((packed));

typedef struct bvr_pipeline_s {
    /*
        State use for default rendering
    */
    struct bvr_pipeline_state_s rendering_pass;
    
    /*
        State use to push the rendering framebuffer 
        to the window framebuffer.
    */
    struct bvr_pipeline_state_s swap_pass;

    uint16 command_count;
    struct bvr_draw_command_s commands[BVR_MAX_DRAW_COMMAND];

    vec3 clear_color;
} bvr_pipeline_t;

typedef struct bvr_framebuffer_s {
    uint16 width, target_width;
    uint16 height, target_height;

    uint32 buffer;
    uint32 color_buffer, depth_buffer, stencil_buffer;
    uint32 vertex_buffer, array_buffer;

    bvr_shader_t shader;
} bvr_framebuffer_t;

void bvr_pipeline_state_enable(struct bvr_pipeline_state_s* const state);
void bvr_pipeline_draw_cmd(struct bvr_draw_command_s* cmd);
void bvr_pipeline_add_draw_cmd(struct bvr_draw_command_s* cmd);
void bvr_poll_errors(void);

BVR_H_FUNC int bvr_pipeline_compare_commands(const void* a, const void* b){
    return (((struct bvr_draw_command_s*)a)->order - ((struct bvr_draw_command_s*)b)->order);
}

int bvr_create_framebuffer(bvr_framebuffer_t* framebuffer, const uint16 width, const uint16 height, const char* shader);
void bvr_framebuffer_enable(bvr_framebuffer_t* framebuffer);
void bvr_framebuffer_disable(bvr_framebuffer_t* framebuffer);
void bvr_framebuffer_clear(bvr_framebuffer_t* framebuffer, vec3 const color);
void bvr_framebuffer_blit(bvr_framebuffer_t* framebuffer);
void bvr_destroy_framebuffer(bvr_framebuffer_t* framebuffer);