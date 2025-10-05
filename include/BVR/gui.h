#pragma once

#include <BVR/window.h>
#include <BVR/actors.h>
#include <stdint.h>

#define BVR_INCLUDE_NUKLEAR

#ifdef BVR_INCLUDE_NUKLEAR

typedef struct bvr_nuklear_s {
    void* context;
    bvr_window_t* window;

    float scale;

    struct nk_font_atlas* atlas;
    int antialiasing;
    uint32 vertex_buffer_length;
    uint32 element_buffer_length;
} bvr_nuklear_t;

int bvr_create_nuklear(bvr_nuklear_t* nuklear, bvr_window_t* window);
void bvr_nuklear_handle(bvr_nuklear_t* nuklear);
void bvr_nuklear_render(bvr_nuklear_t* nuklear);
void bvr_destroy_nuklear(bvr_nuklear_t* nuklear);

void bvr_nuklear_actor_label(bvr_nuklear_t* nuklear, struct bvr_actor_s* actor);
void bvr_nuklear_vec3_label(bvr_nuklear_t* nuklear, const char* text, float* value);

#endif