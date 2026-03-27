#pragma once

#include <bvr/gui.h>

#ifndef BVR_NO_NUKLEAR

#include <bvr/scene.h>

#ifndef BVR_EDITOR_HIDDEN_INPUT
    // F5
    #define BVR_EDITOR_HIDDEN_INPUT 62
#endif

#ifndef BVR_EDITOR_SHOW_INPUT
    // f6
    #define BVR_EDITOR_SHOW_INPUT 63
#endif

#ifndef BVR_EDITOR_SCALE
    #define BVR_EDITOR_SCALE 1.1f
#endif

enum bvr_editor_state_e {
    BVR_EDITOR_STATE_HIDDEN,
    BVR_EDITOR_STATE_HANDLE,
    BVR_EDITOR_STATE_DRAWING,
    BVR_EDITOR_STATE_RENDERING
};

typedef void (*_bvr_editor_callback)(bvr_canvas_t* context, bvr_book_t* book);
typedef void (*_bvr_editor_component_draw_callback)(bvr_canvas_t* context, void* user);

struct bvri_editor_window_s {
    struct bvr_bounds_s viewport;
};

typedef struct bvr_editor_s {
    bvr_canvas_t gui;
    bvr_book_t* book;

    enum bvr_editor_state_e state;
    _bvr_editor_callback callback;

    struct bvri_editor_window_s inspector;
    struct bvri_editor_window_s hierarchy;

    struct {
        bvr_shader_t shader;

        uint32 array_buffer; 
        uint32 vertex_buffer;

        mat4x4 transform;
        
        bool is_gui_hovered;
    } device;

    struct {
        bvr_string_t name;

        void* object;
        uint32 type;

        _bvr_editor_component_draw_callback component;
    } inspector_cmd;
    
    struct {
        int drawmode;

        uint32 element_offset;
        uint32 element_count;
        vec3 color;
    } draw_cmd;

} bvr_editor_t;

void bvr_create_editor(bvr_editor_t* editor, bvr_book_t* book);

/**
 * Returns a pointer to the current editor instance
 */
bvr_editor_t* bvr_get_editor_instance();

/*
    Attach a callback function called when drawing a user-specific UI section
*/
void bvr_editor_attach_callback(_bvr_editor_callback function);

/*
    Prepare beauvoir editor for drawing
*/
void bvr_editor_new_frame();

/*
    Should be executed before `bvr_editor_draw_inspector()`
*/
void bvr_editor_draw_page_hierarchy();

/*
    Should be executed after `bvr_editor_draw_page_hierarchy()`
*/
void bvr_editor_draw_inspector();

/*
    Render editor to the screen
*/
void bvr_editor_render();

/**
 * Update editor's draw command buffer.
 */
void bvr_editor_draw_vertex_buffer(
    float* vertices, uint32 offset, 
    uint32 vertices_count, uint8 stride, int drawmode
);

void bvr_destroy_editor(bvr_editor_t* editor);

#endif 