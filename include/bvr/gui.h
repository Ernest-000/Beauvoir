#pragma once

#ifndef BVR_NO_NUKLEAR

#include <bvr/common.h>
#include <bvr/scene.h>
#include <bvr/actors.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include <nuklear.h>

typedef struct bvr_font_s {
    struct nk_font* font;
    struct nk_font_atlas atlas;
    struct nk_draw_null_texture null_tex;

    uint16 size;

    uint32 id;
} bvr_font_t;

typedef struct bvr_canvas_s {
    struct nk_context context;
    bvr_window_t* win;

    bvr_font_t default_font;

    struct {
        struct nk_buffer cmds;
        
        bvr_shader_t shader;

        uint32 vao;
        uint32 vbo;
        uint32 ebo;

        bvr_shader_uniform_t* projection;
        bvr_shader_uniform_t* texture;
        bvr_shader_uniform_t* texture_array;
        bvr_shader_uniform_t* texture_layer;

        // texture layer offset
        float texture_offset;

        // offsets
        uint32 vs, vp, vt, vc;

        // max counts
        uint32 vertex_count;
        uint32 element_count;

        // scaling
        float scale, alpha;
        int segment_count;

        // anti aliasing
        bool use_antialiasing;

        // callbacks, might be moved :/
        void (*copy_event)(nk_handle, const char *, int len);
        void (*paste_event)(nk_handle, struct nk_text_edit *)
    } device;
} bvr_canvas_t;

int bvr_create_canvas(bvr_canvas_t* nuklear, const bvr_book_t* book);
void bvr_canvas_new_frame(bvr_canvas_t* nuklear);
void bvr_canvas_render(bvr_canvas_t* nuklear);
void bvr_destroy_canvas(bvr_canvas_t* nuklear);

void bvr_nuklear_actor_label(bvr_canvas_t* nuklear, struct bvr_actor_s* actor);
void bvr_nuklear_vec3_label(bvr_canvas_t* nuklear, const char* text, float* value);

int bvr_create_fontf(bvr_font_t* font, FILE* file, const uint16 size);
BVR_H_FUNC int bvr_create_font(bvr_font_t* font, const char* path, const uint16 size){
    BVR_ASSERT(font);

    // prevent empty path by creating a default font
    if(!path){
        return bvr_create_fontf(font, NULL, size);
    }

    FILE* file = fopen(path, "rb");
    int success = bvr_create_fontf(font, file, size);
    fclose(file);

    return success;
}

int bvr_register_font(bvr_canvas_t* canvas, bvr_font_t* font);
void bvr_destroy_font(bvr_font_t* font);

#endif