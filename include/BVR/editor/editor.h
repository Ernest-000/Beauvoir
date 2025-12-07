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

#ifndef BVR_EDITOR_SCALE
    #define BVR_EDITOR_SCALE 1.1f
#endif

enum bvr_editor_state_e {
    BVR_EDITOR_STATE_HIDDEN,
    BVR_EDITOR_STATE_HANDLE,
    BVR_EDITOR_STATE_DRAWING,
    BVR_EDITOR_STATE_RENDERING
};

typedef void (*_bvr_editor_callback)(bvr_nuklear_t* context, bvr_book_t* book);

typedef struct bvr_editor_s {
    bvr_nuklear_t gui;
    bvr_book_t* book;

    enum bvr_editor_state_e state;
    _bvr_editor_callback callback;

    struct {
        bvr_shader_t shader;

        uint32 array_buffer; 
        uint32 vertex_buffer;

        mat4x4 transform;

        struct bvr_bounds_s hierarchy_viewport;
        struct bvr_bounds_s inspector_viewport;
        
        bool is_gui_hovered;
    } device;

    struct {
        bvr_string_t name;

        uint32 type;
        void* pointer;
    } inspector_cmd;

    /*
        contains specific user data for each 
        necessary editor's components
    */
    union {
        // landscape buffer
        struct {
            uint32 cursor[2];
            uint32 taget;
        } landscape;
    } memory;
    
    struct {
        int drawmode;

        uint32 element_offset;
        uint32 element_count;
    } draw_cmd;

} bvr_editor_t;

void bvr_create_editor(bvr_editor_t* editor, bvr_book_t* book);

/*
    Attach a callback function called when drawing a user-specific UI section
*/
void bvr_editor_attach_callback(_bvr_editor_callback function);

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