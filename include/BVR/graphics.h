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

typedef struct bvr_framebuffer_s {
    uint16 width, target_width;
    uint16 height, target_height;

    uint32 buffer;
    uint32 color_buffer, depth_buffer, stencil_buffer;
    uint32 vertex_buffer, array_buffer;

    bvr_shader_t shader;
} bvr_framebuffer_t;

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

    uint8 draw_mode;
    uint8 attrib_count;
    int element_type;

    bvr_vertex_group_t vertex_group;
    
    bvr_shader_t* shader;
};

typedef struct bvr_pipeline_s {
    /**
     *   State use for default rendering
     */
    struct bvr_pipeline_state_s rendering_pass;
    
    /**
     *    State use to push the rendering framebuffer 
     *    to the window framebuffer.
     */
    struct bvr_pipeline_state_s swap_pass;

    /**
     *   Store all draw command and the current index
     */
    uint16 command_count;
    struct bvr_draw_command_s commands[BVR_MAX_DRAW_COMMAND];

    vec3 clear_color;

    struct {
        bvr_framebuffer_t* framebuffer;
        struct bvr_draw_command_s* command;
    } state;
} bvr_pipeline_t;

struct bvr_predefs {
    /**
     * Constant readonly shaders
     */
    struct {
        bvr_shader_t c_invalid_shader;
        bvr_shader_t c_framebuffer_shader;
        bvr_shader_t c_composite_shader;
    } c_shaders;

    bool is_available;
};

/**
 * @brief Change current pipeline state
 * @param bvr_pipeline_state_s target state
 * @return (void)
 */
void bvr_pipeline_state_enable(struct bvr_pipeline_state_s* const state);

/**
 * @brief Push a draw command to the screen without sorting.
 * @param bvr_draw_command_s
 * @return (void)
 */
void bvr_pipeline_draw_cmd(struct bvr_draw_command_s* cmd);

/**
 * @brief Add a draw command to the draw queue. The command will be sorted and drawn when the 
 * draw queue will be flushed or at the end of the frame.
 * @param bvr_draw_command_s
 * @return (void)
 */
void bvr_pipeline_add_draw_cmd(struct bvr_draw_command_s* cmd);

/** 
 * @brief Poll OpenGL errors
 */
void bvr_poll_errors(void);

/**
 * @brief Sort two draw commands
 */
BVR_H_FUNC int bvr_pipeline_compare_commands(const void* a, const void* b){
    return (((struct bvr_draw_command_s*)a)->order - ((struct bvr_draw_command_s*)b)->order);
}

int bvr_create_framebuffer(bvr_framebuffer_t* framebuffer, const uint16 width, const uint16 height, const char* shader);

/**
 * @brief Set a framebuffer as the render buffer target
 * @param framebuffer
 * @return (void)
 */
void bvr_framebuffer_enable(bvr_framebuffer_t* framebuffer);

/**
 * @brief Disable the current framebuffer and bind the default window framebuffer.
 * @param framebuffer
 * @return (void)
 */
void bvr_framebuffer_disable(bvr_framebuffer_t* framebuffer);

/**
 * @brief Clear current framebuffer
 * @param framebuffer
 * @param color
 * @return (void)
 */
void bvr_framebuffer_clear(bvr_framebuffer_t* framebuffer, vec3 const color);

/**
 * @brief Draw framebuffer's texture onto the screen
 * @param framebuffer
 * @return (void)
 */
void bvr_framebuffer_blit(bvr_framebuffer_t* framebuffer);

void bvr_destroy_framebuffer(bvr_framebuffer_t* framebuffer);

/**
 * Create a new predef object. 
 * WARN: internal usage only.
 */
void bvr_create_predefs(struct bvr_predefs* predefs);

void bvr_destroy_predefs(struct bvr_predefs* predefs);