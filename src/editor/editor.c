#include <bvr/editor/editor.h>
#include <bvr/editor/editor_style.h>
#include <bvr/editor/editor_flags.h>
#include <bvr/editor/editor_components.h>

#ifndef BVR_NO_NUKLEAR


#include <bvr/common.h>
#include <bvr/window.h>
#include <bvr/actors.h>
#include <bvr/landscape.h>

#include <bvr/assets.h>
#include <bvr/gl.h>

#include <limits.h>
#include <float.h>

#define BVR_EDITOR_VERTEX_BUFFER_SIZE 1000

#define BVR_NK_RECT(rect) nk_rect(rect.coords[0], rect.coords[1], rect.width, rect.height)

// global functionalities
// depreciate
//static void bvri_editor_import_asset(bvr_string_t* string);
//static void bvri_load_landscape(bvr_string_t* path);

// render buffers
static int bvri_create_editor_render_buffers(uint32* array_buffer, uint32* vertex_buffer, uint64 vertex_size);
static void bvri_bind_editor_buffers(uint32 array_buffer, uint32 vertex_buffer);
static void bvri_set_editor_buffers(float* vertices, uint32 vertices_count, uint8 stride);
static void bvri_draw_editor_buffer(int drawmode, uint32 element_offset, uint32 element_count); 
static void bvri_destroy_editor_render_buffers(uint32* array_buffer, uint32* vertex_buffer);

static bvr_editor_t* __editor;

void bvr_create_editor(bvr_editor_t* editor, bvr_book_t* book){
    BVR_ASSERT(editor);
    BVR_ASSERT(book);
    
    if(__editor){
        BVR_PRINT("warning, override previous binded editor!");
        bvr_destroy_editor(__editor);
    }
    
    __editor = editor;

    struct nk_style* style = &editor->gui.context.style;

    editor->book = book;
    editor->callback = NULL;
    editor->state = BVR_EDITOR_STATE_HANDLE;
    
    editor->inspector_cmd.object = NULL;
    editor->inspector_cmd.component = NULL;
    editor->inspector_cmd.type = BVR_NULL;

    editor->draw_cmd.drawmode = 0;
    editor->draw_cmd.element_offset = 0;
    editor->draw_cmd.element_count = 0;

    editor->hierarchy.viewport.width = 200.0f / BVR_EDITOR_SCALE;
    editor->hierarchy.viewport.height = book->window.framebuffer.height * 0.8f / BVR_EDITOR_SCALE;
    editor->hierarchy.viewport.coords[0] = 2.0;
    editor->hierarchy.viewport.coords[1] = 2.0;
    
    editor->inspector.viewport.width = 350.0f / BVR_EDITOR_SCALE;
    editor->inspector.viewport.height = book->window.framebuffer.height * 0.8f / BVR_EDITOR_SCALE;
    editor->inspector.viewport.coords[0] = book->window.framebuffer.width - editor->inspector.viewport.width;
    editor->inspector.viewport.coords[1] = 2.0;

    {
        const char* vertex_shader = 
            "#version 400\n"
            "layout(location=0) in vec3 in_position;\n"
            "layout(std140) uniform bvr_camera {\n"
	        "mat4 bvr_projection;\n"
	        "mat4 bvr_view;\n"
            "};\n"
            "void main() {\n"
            "	gl_Position = bvr_projection * bvr_view * vec4(in_position, 1.0);\n"
            "}";

        const char* fragment_shader = 
            "#version 400\n"
            "uniform vec3 bvr_color;\n"
            "void main() {\n"
            	"gl_FragColor = vec4(bvr_color, 1.0);\n"
            "}";
        
        const char* shaders[2] = { vertex_shader, fragment_shader };
        BVR_ASSERT(bvr_create_shader_raw(&editor->device.shader, shaders, BVR_VERTEX_SHADER | BVR_FRAGMENT_SHADER));
        BVR_ASSERT(bvr_shader_register_uniform(&editor->device.shader, BVR_VEC3, 0, 1, "bvr_color"));

        BVR_IDENTITY_MAT4(editor->device.transform);
        BVR_IDENTITY_VEC3(editor->draw_cmd.color);
        
        bvr_shader_set_uniform(&editor->device.shader, "bvr_color", &editor->draw_cmd.color);
    }

    if(!bvri_create_editor_render_buffers(
        &editor->device.array_buffer, 
        &editor->device.vertex_buffer, 
        BVR_EDITOR_VERTEX_BUFFER_SIZE
    )){
        BVR_ASSERT(0 && "failed to create editor buffers");
    }
    
    bvr_create_string(&editor->inspector_cmd.name, NULL);
    
    bvr_create_canvas(&editor->gui, book);
    bvr_nk_style(&editor->gui.context);
}

bvr_editor_t* bvr_get_editor_instance(){
    return __editor;
}

void bvr_editor_attach_callback(_bvr_editor_callback function){
    BVR_ASSERT(function);
    
    if(__editor){
        __editor->callback = function;
    }
}

void bvr_editor_new_frame(){
    BVR_ASSERT(__editor);

    bvr_canvas_new_frame(&__editor->gui);
    
    if(bvr_key_down(BVR_EDITOR_HIDDEN_INPUT)){
        __editor->state = BVR_EDITOR_STATE_HIDDEN;
    }
    if(bvr_key_down(BVR_EDITOR_SHOW_INPUT)){
        __editor->state = BVR_EDITOR_STATE_HANDLE;
    }

    if(__editor->state != BVR_EDITOR_STATE_HIDDEN){
        __editor->state = BVR_EDITOR_STATE_DRAWING;
    }

    __editor->device.is_gui_hovered = 0;
}void bvr_editor_draw_page_hierarchy(){
    if(__editor->state == BVR_EDITOR_STATE_HIDDEN) {
        return;
    }

    struct nk_context* p_context = &__editor->gui.context;

    //if(nk_begin(p_context, BVR_FORMAT("scene '%s'", __editor->book->page->name.string), BVR_NK_RECT(__editor->hierarchy.viewport), 
    //    BVR_NK_WINDOW_DEFAULT)){
    //    
    //    nk_layout_space_begin(p_context, NK_DYNAMIC, 1, 1);
    //    
    //    if(nk_tree_push(p_context, NK_TREE_TAB, BVR_FORMAT("%s", __editor->book->page->name.string), NK_MAXIMIZED)){
    //        bvr_nk_draw_button(__editor, "main camera", BVR_EDITOR_CAMERA, &__editor->book->page->camera);
    //
    //        if(nk_tree_push(p_context, NK_TREE_TAB, "actors", NK_MAXIMIZED)){
    //
    //            struct bvr_actor_s** pp_actor; 
    //            // BVR_POOL_FOR_EACH(pp_actor, __editor->book->page->actors){
    //            //     bvr_nk_draw_button_actor(__editor, *pp_actor);
    //            // }
    //
    //            nk_tree_pop(p_context);
    //        }
    //
    //        if(nk_tree_push(p_context, NK_TREE_TAB, "parameters", NK_MAXIMIZED)){
    //            bvr_nk_draw_button(__editor, "globals", 0, NULL);
    //            bvr_nk_draw_button(__editor, "audio", BVR_EDITOR_AUDIO, &__editor->book->audio);
    //            bvr_nk_draw_button(__editor, "user", BVR_EDITOR_USER, NULL);
    //            bvr_nk_draw_button(__editor, "graphics", BVR_EDITOR_PIPELINE, &__editor->book->pipeline);
    //
    //            if(__editor->book->asset_stream.data){
    //                bvr_nk_draw_button(__editor, "assets", BVR_EDITOR_ASSETS, &__editor->book->asset_stream);
    //            }
    //
    //            nk_tree_pop(p_context);
    //        }
    //
    //        nk_tree_pop(p_context);
    //    }
    //    
    //    __editor->device.is_gui_hovered |= nk_window_is_hovered(p_context);
    //
    //    struct nk_rect bounds = nk_window_get_bounds(p_context);
    //    __editor->hierarchy.viewport.width = bounds.w;
    //    __editor->hierarchy.viewport.height = bounds.h;
    //    __editor->hierarchy.viewport.coords[0] = bounds.x;
    //    __editor->hierarchy.viewport.coords[1] = bounds.y;
    //
    //    nk_end(p_context);
    //}
}

void bvr_editor_draw_inspector(){
    if(__editor->state == BVR_EDITOR_STATE_HIDDEN) {
        return;
    }

    if(nk_begin(&__editor->gui.context, "sample", nk_rect(100, 100, 200, 200), BVR_NK_WINDOW_DEFAULT | NK_WINDOW_MOVABLE)){
        nk_layout_row_dynamic(&__editor->gui.context, 15, 2);

        nk_label(&__editor->gui.context, "coucou", NK_TEXT_ALIGN_LEFT);
        
        if(nk_button_label(&__editor->gui.context, "BUTTON")){

        }

        nk_end(&__editor->gui.context);
    }

    /*if(nk_begin(&__editor->gui.context, BVR_FORMAT("inspector '%s'", __editor->inspector_cmd.name.string), 
        BVR_NK_RECT(__editor->inspector.viewport), 
        BVR_NK_WINDOW_DEFAULT | NK_WINDOW_MOVABLE)){
    
        __editor->draw_cmd.drawmode = 0;
        __editor->draw_cmd.element_offset = 0;
        __editor->draw_cmd.element_count = 0;
        __editor->device.is_gui_hovered |= nk_window_is_hovered(&__editor->gui.context);

        // insert command callback here
        BVR_CALL(__editor->inspector_cmd.component, &__editor->gui, __editor->inspector_cmd.object);

        struct nk_rect bounds = nk_window_get_bounds(&__editor->gui.context);
        __editor->inspector.viewport.width = bounds.w;
        __editor->inspector.viewport.height = bounds.h;
        __editor->inspector.viewport.coords[0] = bounds.x;
        __editor->inspector.viewport.coords[1] = bounds.y;

        nk_end(&__editor->gui.context);
    }*/
}

void bvr_editor_render(){
    BVR_ASSERT(__editor);

    if(__editor->state == BVR_EDITOR_STATE_HIDDEN) {
        return;
    }

    if(__editor->state != BVR_EDITOR_STATE_DRAWING){
        BVR_PRINTF("unexpected editor state. Expected %i but %i showed up!", 
            BVR_EDITOR_STATE_DRAWING, __editor->state
        );
        __editor->state = BVR_EDITOR_STATE_RENDERING;
        return;
    }
    __editor->state = BVR_EDITOR_STATE_RENDERING;

    if(__editor->draw_cmd.drawmode){
        bvr_shader_enable(&__editor->device.shader);

        bvri_bind_editor_buffers(__editor->device.array_buffer, __editor->device.vertex_buffer);
        bvri_draw_editor_buffer(__editor->draw_cmd.drawmode, __editor->draw_cmd.element_offset, __editor->draw_cmd.element_count);
        bvri_bind_editor_buffers(0, 0);

        bvr_shader_disable();
    }

    bvr_canvas_render(&__editor->gui);
}

void bvr_editor_draw_vertex_buffer(float* vertices, uint32 offset, uint32 vertices_count, uint8 stride, int drawmode){
    BVR_ASSERT(vertices);
    BVR_ASSERT(vertices_count > 0);

    BVR_ASSERT(__editor);

    bvri_bind_editor_buffers(__editor->device.array_buffer, __editor->device.vertex_buffer);
    bvri_set_editor_buffers(vertices, vertices_count, stride);
    bvri_bind_editor_buffers(0, 0);

    __editor->draw_cmd.drawmode = drawmode;
    __editor->draw_cmd.element_offset = offset;
    __editor->draw_cmd.element_count = vertices_count - offset;
}

void bvr_editor_draw_element_buffer(float* vertices, uint32 vertices_count, uint8 stride);

void bvr_destroy_editor(bvr_editor_t* editor){
    bvr_destroy_string(&editor->inspector_cmd.name);
    bvr_destroy_canvas(&editor->gui);
}

int bvri_create_editor_render_buffers(uint32* array_buffer, uint32* vertex_buffer, uint64 vertex_size){
    BVR_ASSERT(vertex_buffer);

    glGenVertexArrays(1, array_buffer);
    glGenBuffers(1, vertex_buffer);

    glBindVertexArray(*array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);

    glBufferData(GL_ARRAY_BUFFER, vertex_size * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    //glDisableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return BVR_TRUE;
}

void bvri_bind_editor_buffers(uint32 array_buffer, uint32 vertex_buffer){
    glBindVertexArray(array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
}

void bvri_set_editor_buffers(float* vertices, uint32 vertices_count, uint8 stride){
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_count * stride * sizeof(float), vertices);
    
    //glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, stride, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    //glDisableVertexAttribArray(0);
    
}

void bvri_draw_editor_buffer(int drawmode, uint32 element_offset, uint32 element_count){
    //glEnableVertexAttribArray(0);
    glDrawArrays(drawmode, element_offset, element_count);
    //glDisableVertexAttribArray(0);
}

void bvri_destroy_editor_render_buffers(uint32* array_buffer, uint32* vertex_buffer){
    glDeleteVertexArrays(1, array_buffer);
    glDeleteBuffers(1, vertex_buffer);
}

#endif