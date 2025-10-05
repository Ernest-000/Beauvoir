#pragma once

#include <BVR/gui.h>
#include <BVR/scene.h>

#ifndef BVR_EDITOR_HIDDEN_INPUT
    // F5
    #define BVR_EDITOR_HIDDEN_INPUT 62
#endif

#ifndef BVR_EDITOR_SHOW_INPUT
    // f6
    #define BVR_EDITOR_SHOW_INPUT 63
#endif

typedef enum bvr_editor_state_e {
    BVR_EDITOR_STATE_HIDDEN,
    BVR_EDITOR_STATE_HANDLE,
    BVR_EDITOR_STATE_DRAWING,
    BVR_EDITOR_STATE_RENDERING
};

typedef struct bvr_editor_s {
    bvr_nuklear_t gui;
    bvr_book_t* book;

    enum bvr_editor_state_e state;

    struct {
        bvr_shader_t shader;

        uint32 array_buffer; 
        uint32 vertex_buffer;

        mat4x4 transform;
        bool is_gui_hovered;
    } device;

    struct {
        bvr_string_t name;
        uint32 user_data;
        uint32 user_data2;

        uint32 type;
        void* pointer;
    } inspector_cmd;

    struct {
        int drawmode;

        uint32 element_offset;
        uint32 element_count;
    } draw_cmd;

} bvr_editor_t;

void bvr_create_editor(bvr_editor_t* editor, bvr_book_t* book);

/*
    Prepare beauvoir editor for drawing
*/
void bvr_editor_handle();

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

void bvr_destroy_editor(bvr_editor_t* editor);