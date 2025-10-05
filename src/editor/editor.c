#include <BVR/editor/editor.h>
#include <BVR/editor/flags.h>

#include <BVR/buffer.h>
#include <BVR/utils.h>
#include <BVR/window.h>
#include <BVR/actors.h>

#include <BVR/assets.h>
#include <BVR/assets.book.h>

#include <GLAD/glad.h>

#define NK_INCLUDE_FIXED_TYPES 
#include <nuklear.h>

#include <limits.h>
#include <float.h>

#define BVR_EDITOR_VERTEX_BUFFER_SIZE 1000

#define BVR_HIERARCHY_RECT(w, h) (nk_rect(0, 0, 200 + w, 450 + h))
#define BVR_INSPECTOR_RECT(w, h) (nk_rect(__editor->book->window.framebuffer.width - 350 + w, 0, 350 + w, 400 + h))

static int bvri_create_editor_render_buffers(uint32* array_buffer, uint32* vertex_buffer, uint64 vertex_size);
static void bvri_bind_editor_buffers(uint32 array_buffer, uint32 vertex_buffer);
static void bvri_set_editor_buffers(float* vertices, uint32 vertices_count, uint8 stride);
static void bvri_draw_editor_buffer(int drawmode, uint32 element_offset, uint32 element_count); 
static void bvri_destroy_editor_render_buffers(uint32* array_buffer, uint32* vertex_buffer);

static bvr_editor_t* __editor = NULL;

static void bvri_draw_editor_vec3(const char* text, vec3 value){
    nk_layout_row_dynamic(__editor->gui.context, 15, 4);
    nk_label_wrap(__editor->gui.context, text);
    nk_label_wrap(__editor->gui.context, BVR_FORMAT("x%f ", value[0]));
    nk_label_wrap(__editor->gui.context, BVR_FORMAT("y%f ", value[1]));
    nk_label_wrap(__editor->gui.context, BVR_FORMAT("z%f ", value[2]));
}

static void bvri_draw_editor_transform(bvr_transform_t* transform){

    //if(nk_group_begin(__editor->gui.context, "transform", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE));
    if(true)
    {
        nk_layout_row_dynamic(__editor->gui.context, 15, 1);
        nk_label(__editor->gui.context, "TRANSFORM", NK_TEXT_ALIGN_CENTERED);
        
        nk_layout_row_dynamic(__editor->gui.context, 15, 3);
     
        nk_property_float(__editor->gui.context, "x", -100000.0f, &transform->position[0], 100000.0f, 0.1f, 0.1f);
        nk_property_float(__editor->gui.context, "y", -100000.0f, &transform->position[1], 100000.0f, 0.1f, 0.1f);
        nk_property_float(__editor->gui.context, "z", -100000.0f, &transform->position[2], 100000.0f, 0.1f, 0.1f);
        
        nk_property_float(__editor->gui.context, "r", -100000.0f, &transform->rotation[0], 100000.0f, 0.1f, 0.1f);
        nk_property_float(__editor->gui.context, "p", -100000.0f, &transform->rotation[1], 100000.0f, 0.1f, 0.1f);
        nk_property_float(__editor->gui.context, "y", -100000.0f, &transform->rotation[2], 100000.0f, 0.1f, 0.1f);
        
        nk_layout_row_dynamic(__editor->gui.context, 15, 1);
        float scale = transform->scale[0];
        nk_property_float(__editor->gui.context, "size", 0.0f, &scale, 100000.0f, 0.1f, 0.1f);
        BVR_SCALE_VEC3(transform->scale, scale);

        //nk_group_end(__editor->gui.context);
    }
}

static void bvri_draw_editor_body(struct bvr_body_s* body){
    nk_layout_row_dynamic(__editor->gui.context, 15, 1);

    nk_label(__editor->gui.context, BVR_FORMAT("acceleration %f", body->acceleration), NK_TEXT_ALIGN_LEFT);
    bvri_draw_editor_vec3("direction", body->direction);
}

static void bvri_draw_editor_mesh(bvr_mesh_t* mesh){
    if(!mesh){
        return;
    }

    nk_layout_row_dynamic(__editor->gui.context, 15, 1);

    nk_label(__editor->gui.context, "Mesh", NK_TEXT_ALIGN_CENTERED);
    nk_layout_row_dynamic(__editor->gui.context, 40, 1);
    if (nk_group_begin(__editor->gui.context, BVR_FORMAT("framebuffer%x", &mesh), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic(__editor->gui.context, 15, 2);

        nk_label(__editor->gui.context, BVR_FORMAT("vertex count %i", mesh->vertex_count), NK_TEXT_ALIGN_LEFT);
        nk_label(__editor->gui.context, BVR_FORMAT("element count %i", mesh->element_count), NK_TEXT_ALIGN_LEFT);

        nk_label(__editor->gui.context, BVR_FORMAT("vertex buffer '0x%x'", mesh->vertex_buffer), NK_TEXT_ALIGN_LEFT);
        nk_label(__editor->gui.context, BVR_FORMAT("element buffer '0x%x'", mesh->element_buffer), NK_TEXT_ALIGN_LEFT);

        nk_group_end(__editor->gui.context);
    }
}

static void bvri_draw_editor_image(bvr_image_t* image){
    nk_layout_row_dynamic(__editor->gui.context, 15, 1);

    nk_label(__editor->gui.context, "Image", NK_TEXT_ALIGN_CENTERED);
    nk_label_wrap(__editor->gui.context, image->asset.pointer.asset_id);
}

static void bvri_draw_editor_layer(bvr_layer_t* layer){

    nk_layout_row_dynamic(__editor->gui.context, 90, 1);
    if(nk_group_begin_titled(__editor->gui.context, layer->name.string, layer->name.string, NK_WINDOW_BORDER | NK_WINDOW_TITLE)){
        int flag = 0;
        nk_layout_row_dynamic(__editor->gui.context, 15, 2);
        nk_checkbox_label(__editor->gui.context, "is visible", (nk_bool*)&layer->opacity);

        flag = BVR_HAS_FLAG(layer->flags, BVR_LAYER_Y_SORTED);
        if(nk_checkbox_label(__editor->gui.context, "y sorted", &flag)){
            layer->flags ^= BVR_LAYER_Y_SORTED;
        }

        nk_property_int(__editor->gui.context, "#x", -100000, &layer->anchor_x, 100000, 1, 1);
        nk_property_int(__editor->gui.context, "#y", -100000, &layer->anchor_y, 100000, 1, 1);

        nk_group_end(__editor->gui.context);
    }
    
}

static void bvri_draw_editor_shader(bvr_shader_t* shader){
    if(!shader){
        return;
    }

    nk_layout_row_dynamic(__editor->gui.context, 15, 1);

    nk_label(__editor->gui.context, "Shader", NK_TEXT_ALIGN_CENTERED);
    nk_label(__editor->gui.context, shader->asset.pointer.asset_id, NK_TEXT_ALIGN_LEFT);
    
    nk_layout_row_dynamic(__editor->gui.context, 15, 2);
    
    char type_name[16];
    for (size_t i = 0; i < shader->uniform_count; i++)
    {
        bvr_nameof(shader->uniforms[i].type, type_name);

        nk_label_wrap(__editor->gui.context, BVR_FORMAT("%s", shader->uniforms[i].name.string));  
        
        nk_label_wrap(__editor->gui.context, BVR_FORMAT("%s", type_name));     
    }
}

static void bvri_draw_hierarchy_button(const char* name, uint64 type, void* object){
    if(nk_button_label(__editor->gui.context, name)){

        bvr_destroy_string(&__editor->inspector_cmd.name);
        bvr_create_string(&__editor->inspector_cmd.name, name);

        __editor->inspector_cmd.type = type;
        __editor->inspector_cmd.pointer = object;
    }
}

void bvr_create_editor(bvr_editor_t* editor, bvr_book_t* book){
    BVR_ASSERT(editor);
    BVR_ASSERT(book);
    
    if(__editor){
        BVR_PRINT("warning, override previous binded editor!");
        bvr_destroy_editor(__editor);
    }
    
    __editor = editor;

    editor->book = book;
    editor->state = BVR_EDITOR_STATE_HANDLE;
    editor->inspector_cmd.pointer = NULL;
    editor->inspector_cmd.type = 0;
    editor->draw_cmd.drawmode = 0;
    editor->draw_cmd.element_offset = 0;
    editor->draw_cmd.element_count = 0;

    {
        const char* vertex_shader = 
            "#version 400\n"
            "layout(location=0) in vec3 in_position;\n"
            "uniform mat4 bvr_transform;\n"
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
        
        bvri_create_shader_vert_frag(&editor->device.shader, vertex_shader, fragment_shader);
        //BVR_ASSERT(bvr_shader_register_uniform(&editor->device.shader, BVR_MAT4, 1, "bvr_transform"));
        BVR_ASSERT(bvr_shader_register_uniform(&editor->device.shader, BVR_VEC3, 1, "bvr_color"));
        BVR_ASSERT(bvr_shader_register_block(&editor->device.shader, BVR_UNIFORM_CAMERA_NAME, BVR_MAT4, 2, BVR_UNIFORM_BLOCK_CAMERA));

        vec3 color = {0.0f, 1.0f, 0.0f};
        BVR_IDENTITY_MAT4(editor->device.transform);
        
        bvr_shader_set_uniformi(&editor->device.shader.uniforms[1], &color);
        
    }

    if(bvri_create_editor_render_buffers(
        &editor->device.array_buffer, 
        &editor->device.vertex_buffer, 
        BVR_EDITOR_VERTEX_BUFFER_SIZE
    )){
        BVR_ASSERT(0 || "failed to create editor buffers");
    }
    
    bvr_create_string(&editor->inspector_cmd.name, NULL);
    bvr_create_nuklear(&editor->gui, &book->window);
}

void bvr_editor_handle(){
    BVR_ASSERT(__editor);

    bvr_nuklear_handle(&__editor->gui);
    
    if(bvr_key_down(BVR_EDITOR_HIDDEN_INPUT)){
        __editor->state = BVR_EDITOR_STATE_HIDDEN;
    }
    if(bvr_key_down(BVR_EDITOR_SHOW_INPUT)){
        __editor->state = BVR_EDITOR_STATE_HANDLE;
    }

    if(__editor->state != BVR_EDITOR_STATE_HIDDEN){
        __editor->state = BVR_EDITOR_STATE_HANDLE;
    }

    __editor->device.is_gui_hovered = 0;
}

void bvr_editor_draw_page_hierarchy(){
    if(__editor->state == BVR_EDITOR_STATE_HIDDEN) {
        return;
    }

    BVR_ASSERT(__editor->state == BVR_EDITOR_STATE_HANDLE);
    __editor->state = BVR_EDITOR_STATE_DRAWING;

    if(nk_begin(__editor->gui.context, BVR_FORMAT("scene '%s'", __editor->book->page.name.string), BVR_HIERARCHY_RECT(0, 0), 
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE)){

        {
            nk_menubar_begin(__editor->gui.context);
            {
                nk_layout_row_begin(__editor->gui.context, NK_STATIC, 25, 2); 
                nk_layout_row_push(__editor->gui.context, 45);

                if(nk_menu_begin_label(__editor->gui.context, "file", NK_TEXT_ALIGN_LEFT, nk_vec2(100, 100))){
                    nk_layout_row_dynamic(__editor->gui.context, 15, 1);

                    if(nk_menu_item_label(__editor->gui.context, "save", NK_TEXT_ALIGN_LEFT)){
                        bvr_write_book(BVR_FORMAT("%s.bin", __editor->book->page.name.string), __editor->book);
                    }

                    if(nk_menu_item_label(__editor->gui.context, "open", NK_TEXT_ALIGN_LEFT)){
                        bvr_open_book(BVR_FORMAT("%s.bin", __editor->book->page.name.string), __editor->book);
                    }

                    if(nk_menu_item_label(__editor->gui.context, "clear cache", NK_TEXT_ALIGN_LEFT)){
                        remove(BVR_FORMAT("%s.bin", __editor->book->page.name.string));
                    }

                    if(nk_menu_item_label(__editor->gui.context, "exit", NK_TEXT_ALIGN_LEFT)){
                        __editor->book->window.awake = 0;
                    }

                    nk_menu_end(__editor->gui.context);
                }

                if(nk_menu_begin_label(__editor->gui.context, "tools", NK_TEXT_ALIGN_LEFT, nk_vec2(100, 100))){
                    nk_layout_row_dynamic(__editor->gui.context, 15, 1);

                    if(nk_menu_item_label(__editor->gui.context, "triangulate", NK_TEXT_ALIGN_LEFT)){
                        
                    }

                    if(nk_menu_item_label(__editor->gui.context, "save garbage", NK_TEXT_ALIGN_LEFT)){
                        if(__editor->book->garbage_stream.data){
                            remove("garbagedump.bin");
                            FILE* file = fopen("garbagedump.bin", "wb");
                            fwrite(__editor->book->garbage_stream.data, sizeof(char), __editor->book->garbage_stream.size, file);
                            fclose(file);
                        }
                    }

                    if(nk_menu_item_label(__editor->gui.context, "save assets", NK_TEXT_ALIGN_LEFT)){
                        if(__editor->book->asset_stream.data){
                            remove("assetsdump.bin");
                            FILE* file = fopen("assetsdump.bin", "wb");
                            fwrite(__editor->book->asset_stream.data, sizeof(char), __editor->book->asset_stream.size, file);
                            fclose(file);
                        }
                    }

                    nk_menu_end(__editor->gui.context);
                }
            }
            nk_menubar_end(__editor->gui.context);
        }
        
        {
            vec2 screen, world;
            bvr_mouse_position(&screen[0], &screen[1]);
            bvr_screen_to_world_coords(__editor->book, screen, world);

            nk_layout_row_dynamic(__editor->gui.context, 15, 1);
            nk_label_wrap(__editor->gui.context, BVR_FORMAT("mx%f my%f", world[0], world[1]));
        }

        // scene components
        nk_layout_row_dynamic(__editor->gui.context, 150, 1);
        if(nk_group_begin_titled(__editor->gui.context, BVR_MACRO_STR(__LINE__), "global infos", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(__editor->gui.context, 15, 1);

            if(__editor->book->asset_stream.data){
                bvri_draw_hierarchy_button("assets", BVR_EDITOR_ASSETS, &__editor->book->asset_stream);
            }

            bvri_draw_hierarchy_button("camera", BVR_EDITOR_CAMERA, &__editor->book->page.camera);
            bvri_draw_hierarchy_button("graphic pipeline", BVR_EDITOR_PIPELINE, &__editor->book->pipeline);

            nk_group_end(__editor->gui.context);
        }

        // scene actors
        nk_layout_row_dynamic(__editor->gui.context, 50 + __editor->book->page.actors.count * 20, 1);
        if(nk_group_begin_titled(__editor->gui.context, BVR_MACRO_STR(__LINE__), "actors", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(__editor->gui.context, 15, 1);
            
            struct bvr_actor_s* actor = NULL;
            BVR_POOL_FOR_EACH(actor, __editor->book->page.actors){
                if(!actor){
                    break;
                }

                switch (actor->type)
                {
                case BVR_LANDSCAPE_ACTOR:
                    bvri_draw_hierarchy_button(actor->name.string, BVR_EDITOR_LANDSCAPE, actor);
                    break;
                
                default:
                    bvri_draw_hierarchy_button(actor->name.string, BVR_EDITOR_ACTOR, actor);
                    break;
                }
            }

            nk_group_end(__editor->gui.context);
        }

        nk_layout_row_dynamic(__editor->gui.context, 70 + __editor->book->page.colliders.count * 20, 1);
        if(nk_group_begin_titled(__editor->gui.context, BVR_MACRO_STR(__LINE__), "colliders", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(__editor->gui.context, 15, 1);
            
            struct bvr_collider_s* collider = NULL;
            BVR_POOL_FOR_EACH(collider, __editor->book->page.colliders){
                if(!collider){
                    break;
                }

                bvri_draw_hierarchy_button(
                    BVR_FORMAT("collider%x", (uint64)blockcollider - (uint64)__editor->book->page.colliders.data),
                    BVR_EDITOR_COLLIDER, collider
                );
            }

            if(nk_button_label(__editor->gui.context, "Add")){

            }

            nk_group_end(__editor->gui.context);
        }
        
        __editor->device.is_gui_hovered |= nk_window_is_hovered(__editor->gui.context);
        nk_end(__editor->gui.context);
    }
}

static void bvri_editor_import_asset(bvr_string_t* string){
    if(string && string->string){
        if(bvr_register_asset(string->string, BVR_OPEN_READ)){
            BVR_PRINTF("sucessfully imported %s", string->string);
        }
        else {
            BVR_PRINTF("failed to import %s", string->string);
        }
    }
}

void bvr_editor_draw_inspector(){
    if(__editor->state == BVR_EDITOR_STATE_HIDDEN) {
        return;
    }

    BVR_ASSERT(__editor->state == BVR_EDITOR_STATE_DRAWING);

    if(nk_begin(__editor->gui.context, BVR_FORMAT("inspector '%s'", __editor->inspector_cmd.name.string), BVR_INSPECTOR_RECT(0, 0), 
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE)){
    
        if(!__editor->inspector_cmd.pointer){
            nk_end(__editor->gui.context);
            return;
        }

        __editor->draw_cmd.drawmode = 0;
        __editor->draw_cmd.element_offset = 0;
        __editor->draw_cmd.element_count = 0;

        nk_layout_row_dynamic(__editor->gui.context, 15, 1);
        switch (__editor->inspector_cmd.type)
        {
        case BVR_EDITOR_CAMERA:
            {
                bvr_camera_t* camera = (bvr_camera_t*)__editor->inspector_cmd.pointer;
                
                nk_layout_row_dynamic(__editor->gui.context, 40, 1);
                if(nk_group_begin(__editor->gui.context, BVR_FORMAT("framebuffer%x", &camera->framebuffer), NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR))
                {
                    nk_layout_row_dynamic(__editor->gui.context, 15, 2);
                    
                    nk_label(__editor->gui.context, BVR_FORMAT("width %i", camera->framebuffer->width), NK_TEXT_ALIGN_LEFT);
                    nk_label(__editor->gui.context, BVR_FORMAT("height %i", camera->framebuffer->height), NK_TEXT_ALIGN_LEFT);
                    
                    nk_label(__editor->gui.context, BVR_FORMAT("far %f", camera->far), NK_TEXT_ALIGN_LEFT);
                    nk_label(__editor->gui.context, BVR_FORMAT("near %f", camera->near), NK_TEXT_ALIGN_LEFT);

                    nk_group_end(__editor->gui.context);
                }

                nk_property_float(__editor->gui.context, "scale", 0.01f, &camera->field_of_view.scale, 20.0f, 0.1f, 0.1f);
                
                bvri_draw_editor_transform(&camera->transform);
            }
            break;

        case BVR_EDITOR_PIPELINE:
            {
                bvr_pipeline_t* pipeline = (bvr_pipeline_t*)__editor->inspector_cmd.pointer;
                
                nk_layout_row_dynamic(__editor->gui.context, 15, 1);

                nk_label(__editor->gui.context, BVR_FORMAT("render time %f ms", __editor->book->timer.delta_time), NK_TEXT_ALIGN_LEFT);
                nk_label(__editor->gui.context, BVR_FORMAT("fps %i", __editor->book->timer.average_render_time), NK_TEXT_ALIGN_LEFT);

                nk_checkbox_label(__editor->gui.context, "is blending", (int*)&pipeline->rendering_pass.blending);
                nk_checkbox_label(__editor->gui.context, "is depth testing", (int*)&pipeline->rendering_pass.depth);

                int blending = pipeline->rendering_pass.blending;
                int depth = pipeline->rendering_pass.depth;

                nk_layout_row_dynamic(__editor->gui.context, 20, 1);
                if(nk_combo_begin_label(__editor->gui.context, "blending", nk_vec2(200, 150))){
                    nk_layout_row_dynamic(__editor->gui.context, 15, 1);

                    blending = BVR_HAS_FLAG(pipeline->rendering_pass.blending, BVR_BLEND_FUNC_ALPHA_ONE_MINUS);
                    if(nk_checkbox_label(__editor->gui.context, "alpha one minus", &blending)){
                        pipeline->rendering_pass.blending ^= BVR_BLEND_FUNC_ALPHA_ONE_MINUS;
                    }

                    blending = BVR_HAS_FLAG(pipeline->rendering_pass.blending, BVR_BLEND_FUNC_ALPHA_ADD);
                    if(nk_checkbox_label(__editor->gui.context, "alpha add", &blending)){
                        pipeline->rendering_pass.blending ^= BVR_BLEND_FUNC_ALPHA_ADD;
                    }

                    blending = BVR_HAS_FLAG(pipeline->rendering_pass.blending, BVR_BLEND_FUNC_ALPHA_MULT);
                    if(nk_checkbox_label(__editor->gui.context, "alpha mult", &blending)){
                        pipeline->rendering_pass.blending ^= BVR_BLEND_FUNC_ALPHA_MULT;
                    }

                    nk_combo_end(__editor->gui.context);
                }

                if(nk_combo_begin_label(__editor->gui.context, "depth", nk_vec2(200, 150))){
                    nk_layout_row_dynamic(__editor->gui.context, 15, 1);

                    depth = BVR_HAS_FLAG(pipeline->rendering_pass.depth, BVR_DEPTH_FUNC_NEVER);
                    if(nk_checkbox_label(__editor->gui.context, "never", &blending)){
                        pipeline->rendering_pass.depth ^= BVR_DEPTH_FUNC_NEVER;
                    }

                    depth = BVR_HAS_FLAG(pipeline->rendering_pass.depth, BVR_DEPTH_FUNC_ALWAYS);
                    if(nk_checkbox_label(__editor->gui.context, "always", &blending)){
                        pipeline->rendering_pass.depth ^= BVR_DEPTH_FUNC_ALWAYS;
                    }

                    depth = BVR_HAS_FLAG(pipeline->rendering_pass.depth, BVR_DEPTH_FUNC_LESS);
                    if(nk_checkbox_label(__editor->gui.context, "less", &blending)){
                        pipeline->rendering_pass.depth ^= BVR_DEPTH_FUNC_LESS;
                    }

                    depth = BVR_HAS_FLAG(pipeline->rendering_pass.depth, BVR_DEPTH_FUNC_GREATER);
                    if(nk_checkbox_label(__editor->gui.context, "greater", &blending)){
                        pipeline->rendering_pass.depth ^= BVR_DEPTH_FUNC_GREATER;
                    }

                    depth = BVR_HAS_FLAG(pipeline->rendering_pass.depth, BVR_DEPTH_FUNC_LEQUAL);
                    if(nk_checkbox_label(__editor->gui.context, "less or equal", &blending)){
                        pipeline->rendering_pass.depth ^= BVR_DEPTH_FUNC_LEQUAL;
                    }

                    depth = BVR_HAS_FLAG(pipeline->rendering_pass.depth, BVR_DEPTH_FUNC_GEQUAL);
                    if(nk_checkbox_label(__editor->gui.context, "greater or equal", &blending)){
                        pipeline->rendering_pass.depth ^= BVR_DEPTH_FUNC_GEQUAL;
                    }

                    depth = BVR_HAS_FLAG(pipeline->rendering_pass.depth, BVR_DEPTH_FUNC_NOTEQUAL);
                    if(nk_checkbox_label(__editor->gui.context, "not equal", &blending)){
                        pipeline->rendering_pass.depth ^= BVR_DEPTH_FUNC_NOTEQUAL;
                    }

                    depth = BVR_HAS_FLAG(pipeline->rendering_pass.depth, BVR_DEPTH_FUNC_EQUAL);
                    if(nk_checkbox_label(__editor->gui.context, "equal", &blending)){
                        pipeline->rendering_pass.depth ^= BVR_DEPTH_FUNC_EQUAL;
                    }

                    nk_combo_end(__editor->gui.context);
                }
                
            }
            break;

        case BVR_EDITOR_ASSETS:
            {
                bvr_memstream_t* stream = (bvr_memstream_t*)__editor->inspector_cmd.pointer;
                bvr_asset_t asset;

                bvr_memstream_seek(stream, 0, SEEK_SET);
                while (!bvr_memstream_eof(stream))
                {
                    bvr_memstream_read(stream, &asset.id, sizeof(bvr_uuid_t));
                    if(asset.id[0] + asset.id[1] + asset.id[2] == 0){
                        break;
                    }

                    bvr_memstream_read(stream, &asset.path.length, sizeof(uint16));

                    asset.path.string = stream->cursor;
                    bvr_memstream_seek(stream, asset.path.length, SEEK_CUR);
                    bvr_memstream_read(stream, &asset.open_mode, sizeof(uint8));

                    nk_layout_row_dynamic(__editor->gui.context, 45, 1);
                    if(nk_group_begin(__editor->gui.context, BVR_MACRO_STR(__LINE__), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)){
                        nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                        nk_label(__editor->gui.context, asset.id, NK_TEXT_ALIGN_LEFT);

                        nk_layout_row_dynamic(__editor->gui.context, 15, 2);
                        nk_label(__editor->gui.context, asset.path.string, NK_TEXT_ALIGN_CENTERED);
                        
                        if(asset.open_mode == BVR_OPEN_READ){
                            nk_label(__editor->gui.context, "READ ONLY", NK_TEXT_ALIGN_RIGHT);
                        }
                        else{
                            nk_label(__editor->gui.context, "WRITE ONLY", NK_TEXT_ALIGN_RIGHT);
                        }

                        nk_group_end(__editor->gui.context);
                    }
                }

                nk_layout_row_dynamic(__editor->gui.context, 25, 2);
                if(nk_button_label(__editor->gui.context, "Import New")){
                    bvr_open_file_dialog(bvri_editor_import_asset);
                }

                if(nk_button_label(__editor->gui.context, "Clear")){
                    bvr_memstream_clear(&__editor->book->asset_stream);
                }
            }
            break;
        
        case BVR_EDITOR_ACTOR: 
            {
                struct bvr_actor_s* actor = (struct bvr_actor_s*)__editor->inspector_cmd.pointer;
                
                nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                nk_label(__editor->gui.context, BVR_FORMAT("id %s", actor->id), NK_TEXT_ALIGN_LEFT);
                nk_label(__editor->gui.context, BVR_FORMAT("flags %x", actor->flags), NK_TEXT_ALIGN_LEFT);

                nk_checkbox_label(__editor->gui.context, "is active", (nk_bool*)&actor->active);
                nk_property_int(__editor->gui.context, "order in layer", 0, (int*)&actor->order_in_layer, 0x7fff, 1, 1.0f);

                bvri_draw_editor_transform(&actor->transform);

                nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                switch (actor->type)
                {
                case BVR_EMPTY_ACTOR:
                    nk_label(__editor->gui.context, "EMPTY ACTOR", NK_TEXT_ALIGN_CENTERED);
                    break;
                case BVR_LAYER_ACTOR:
                    {
                        nk_label(__editor->gui.context, "LAYER ACTOR", NK_TEXT_ALIGN_CENTERED);
                        bvr_layer_t* layers = (bvr_layer_t*)(((bvr_layer_actor_t*)actor)->texture.image.layers.data);
                        nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                        bvri_draw_editor_shader(&((bvr_layer_actor_t*)actor)->shader);
                        bvri_draw_editor_image(&((bvr_layer_actor_t*)actor)->texture.image);

                        nk_label(__editor->gui.context, "LAYERS", NK_TEXT_ALIGN_CENTERED);
                        for (uint64 layer = 0; layer < BVR_BUFFER_COUNT(((bvr_layer_actor_t*)actor)->texture.image.layers); layer++)
                        {
                            bvri_draw_editor_layer(&layers[layer]);
                        }
                        
                    }
                    break;
                case BVR_BITMAP_ACTOR:
                    {
                        nk_label(__editor->gui.context, "BITMAP ACTOR", NK_TEXT_ALIGN_CENTERED);
                    }
                    break;
                case BVR_STATIC_ACTOR:
                    {
                        nk_label(__editor->gui.context, "STATIC ACTOR", NK_TEXT_ALIGN_CENTERED);
                    }
                    break;
                case BVR_DYNAMIC_ACTOR:
                    {
                        nk_label(__editor->gui.context, "DYNAMIC ACTOR", NK_TEXT_ALIGN_CENTERED);
                        bvri_draw_editor_mesh(&((bvr_dynamic_actor_t*)actor)->mesh);
                        bvri_draw_editor_shader(&((bvr_dynamic_actor_t*)actor)->shader);
                        bvri_draw_editor_body(&((bvr_dynamic_actor_t*)actor)->collider.body);
                        nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                        bvri_draw_hierarchy_button("go to collider", BVR_EDITOR_COLLIDER, &((bvr_dynamic_actor_t*)actor)->collider);
                    }
                    break;
                default:
                    break;
                }
                
            }
            break;

        case BVR_EDITOR_COLLIDER:
            {
                bvr_collider_t* collider = (bvr_collider_t*)__editor->inspector_cmd.pointer;

                //nk_layout_row_dynamic(__editor->gui.context, 180, 1);
                //if(nk_group_begin_titled(__editor->gui.context, BVR_FORMAT("collider%i", collider), "geometry", NK_WINDOW_BORDER | NK_WINDOW_TITLE)){
                if(true){    

                    nk_layout_row_dynamic(__editor->gui.context, 15, 3);
                    nk_checkbox_label(__editor->gui.context, "is inverted", (nk_bool*)&collider->is_inverted);

                    // bouding box typed collider gui
                    if(collider->shape == BVR_COLLIDER_BOX){
                        struct bvr_bounds_s* bounds = (struct bvr_bounds_s*)collider->geometry.data;
                    
                        {
                            float vertices[15] = {
                                bounds->coords[0] + bounds->width * -0.5f, bounds->coords[1] + bounds->height * +0.5f, 0.1f,
                                bounds->coords[0] + bounds->width * +0.5f, bounds->coords[1] + bounds->height * +0.5f, 0.1f,
                                bounds->coords[0] + bounds->width * +0.5f, bounds->coords[1] + bounds->height * -0.5f, 0.1f,
                                bounds->coords[0] + bounds->width * -0.5f, bounds->coords[1] + bounds->height * -0.5f, 0.1f,
                                bounds->coords[0] + bounds->width * -0.5f, bounds->coords[1] + bounds->height * +0.5f, 0.1f,
                            };
                        
                            bvri_bind_editor_buffers(__editor->device.array_buffer, __editor->device.vertex_buffer);
                            bvri_set_editor_buffers(vertices, 5, 3);
                            bvri_bind_editor_buffers(0, 0);
                        
                            __editor->draw_cmd.drawmode = BVR_DRAWMODE_LINE_STRIPE;
                            __editor->draw_cmd.element_offset = 0;
                            __editor->draw_cmd.element_count = 5;
                        }
                        
                        nk_layout_row_dynamic(__editor->gui.context, 15, 3);
                        
                        nk_label_wrap(__editor->gui.context, "coords");
                        nk_label_wrap(__editor->gui.context, BVR_FORMAT("x %f ", bounds->coords[0]));
                        nk_label_wrap(__editor->gui.context, BVR_FORMAT("y %f ", bounds->coords[1]));
                    
                        nk_label_wrap(__editor->gui.context, "size");
                        nk_label_wrap(__editor->gui.context, BVR_FORMAT("width %i ", bounds->width));
                        nk_label_wrap(__editor->gui.context, BVR_FORMAT("height %i ", bounds->height));
                    }

                    // triangle typed collider gui
                    else if (collider->shape == BVR_COLLIDER_TRIARRAY)
                    {
                        vec2* tri = (vec2*)collider->geometry.data;

                        {
                            bvri_bind_editor_buffers(__editor->device.array_buffer, __editor->device.vertex_buffer);
                            bvri_set_editor_buffers(collider->geometry.data, collider->geometry.size / sizeof(vec2), 2);
                            bvri_bind_editor_buffers(0, 0);
                        
                            __editor->draw_cmd.drawmode = BVR_DRAWMODE_LINE_STRIPE;
                            __editor->draw_cmd.element_offset = 0;
                            __editor->draw_cmd.element_count = collider->geometry.size / sizeof(vec2);
                        }

                        nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                        nk_label(__editor->gui.context, BVR_FORMAT("%i triangles", collider->geometry.size / sizeof(vec2) / 3), NK_TEXT_ALIGN_LEFT);
                        
                        nk_layout_row_dynamic(__editor->gui.context, 15, 6);

                        for (size_t triid = 0; triid < collider->geometry.size / sizeof(vec2) / 3; triid++)
                        {
                            if(nk_tree_push(__editor->gui.context, NK_TREE_TAB, BVR_FORMAT("triangle %i", triid), NK_MINIMIZED)){
                                nk_layout_row_dynamic(__editor->gui.context, 15, 2);

                                for (size_t i = 0; i < 3; i++)
                                {
                                    nk_label_wrap(__editor->gui.context, BVR_FORMAT("x%f", (tri[triid + i])[0]));
                                    nk_label_wrap(__editor->gui.context, BVR_FORMAT("y%f", (tri[triid + i])[1]));
                                }

                                nk_tree_pop(__editor->gui.context);
                            }
                        }
                    }
                    //nk_group_end(__editor->gui.context);
                } 
            }
            break;

        case BVR_EDITOR_LANDSCAPE:
            {
                bvr_landscape_actor_t* actor = (bvr_landscape_actor_t*)__editor->inspector_cmd.pointer;
                
                nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                nk_label(__editor->gui.context, BVR_FORMAT("id %s", actor->object.id), NK_TEXT_ALIGN_LEFT);
                nk_label(__editor->gui.context, BVR_FORMAT("flags %x", actor->object.flags), NK_TEXT_ALIGN_LEFT);

                nk_checkbox_label(__editor->gui.context, "is active", (nk_bool*)&actor->object.active);

                bvri_draw_editor_transform(&actor->object.transform);

                nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                nk_label(__editor->gui.context, "LANDSCAPE", NK_TEXT_ALIGN_CENTERED);

                nk_layout_row_dynamic(__editor->gui.context, 15, 2);
                nk_property_int(__editor->gui.context, "tile x", 0, &__editor->inspector_cmd.user_data, actor->dimension[0] - 1, 1, .5f);
                nk_property_int(__editor->gui.context, "tile y", 0, &__editor->inspector_cmd.user_data2, actor->dimension[1] - 1, 1, .5f);

                // apply y axis
                const int vertices_per_row = actor->dimension[0] * 2 + 3;

                int target_tile = __editor->inspector_cmd.user_data2 * vertices_per_row;
                // apply x axis
                target_tile += (int)clamp(__editor->inspector_cmd.user_data * 2.0f, 0.0f, vertices_per_row - 2.0f);
                // start offset 
                target_tile += 3;

                BVR_ASSERT(target_tile < actor->mesh.vertex_count);

                nk_layout_row_dynamic(__editor->gui.context, 15, 1);

                {
                    nk_label(__editor->gui.context, "TILE INFORMATIONS", NK_TEXT_ALIGN_LEFT);
                    
                    glBindBuffer(GL_ARRAY_BUFFER, actor->mesh.vertex_buffer);
                    int* vertices_map = glMapBufferRange(GL_ARRAY_BUFFER, 0, actor->mesh.vertex_count * sizeof(int), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

                    if(vertices_map){
                        // extract each values through bitwise operations
                        int texture_id = (0xff00 & vertices_map[target_tile]) >> 8;
                        int altitude[2] = {0, 0};

                        altitude[0] = 0xff & vertices_map[target_tile + 0];
                        altitude[1] = 0xff & vertices_map[target_tile + 1];

                        //TODO: better texture id selection
                        nk_property_int(__editor->gui.context, "texture id", 0, &texture_id, actor->atlas.tile_count_x * actor->atlas.tile_count_y, 1, .5f);
                        
                        nk_layout_row_dynamic(__editor->gui.context, 15, 2);
                        nk_property_int(__editor->gui.context, "Altutide 0", 0, &altitude[0], 255, 1, 1.0f);
                        nk_property_int(__editor->gui.context, "Altutide 1", 0, &altitude[1], 255, 1, 1.0f);

                        nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                        
                        // if values are differents
                        if(texture_id != ((0xff00 & vertices_map[target_tile + 0]) >> 8)){
                            // write texture id on the two opposites vertices
                            // bitmask: remove all previous texture id and overwrite with the new texid value shifted 8 bits 
                            vertices_map[target_tile + 0] = (vertices_map[target_tile + 0] & ~0xff00) | (texture_id) << 8;
                            vertices_map[target_tile + 1] = (vertices_map[target_tile + 1] & ~0xff00) | (texture_id) << 8;
                        }

                        // check for altitude on the first vertex
                        if(altitude[0] != (0xff & vertices_map[target_tile + 0])){
                            // try to overwrite the vertex on the other side of the edge (on top of it)
                            int prev_row_vertex = (int)clamp(target_tile - vertices_per_row + 1, 0, actor->mesh.vertex_count);
                            vertices_map[target_tile + 0] = (vertices_map[target_tile + 0] & ~0xff) | altitude[0];
                            vertices_map[prev_row_vertex] = (vertices_map[prev_row_vertex] & ~0xff) | altitude[0];
                        }

                        if(altitude[1] != (0xff & vertices_map[target_tile + 1])){
                            // try to overwrite the vertex on the other side of the edge (bottom)
                            int next_row_vertex = (int)clamp(target_tile + vertices_per_row, 0, actor->mesh.vertex_count);
                            
                            vertices_map[target_tile + 1] = (vertices_map[target_tile + 1] & ~0xff) | altitude[1];
                            vertices_map[next_row_vertex] = (vertices_map[next_row_vertex] & ~0xff) | altitude[1];
                        }

                        glUnmapBuffer(GL_ARRAY_BUFFER);
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                    }
                }

                // drawing tile selector gizmo
                {
                    vec2 tile_position, half_size;
                    tile_position[0] = __editor->inspector_cmd.user_data;
                    tile_position[1] = __editor->inspector_cmd.user_data2;

                    half_size[0] = actor->dimension[2] * 0.5f;
                    half_size[1] = actor->dimension[3] * 0.5f - actor->dimension[3];

                    vec2_add(tile_position, actor->object.transform.position, tile_position);
                    tile_position[0] *= actor->dimension[2];
                    tile_position[1] *= -actor->dimension[3];

                    vec2_add(tile_position, half_size, tile_position);

                    float vertices[] = {
                        tile_position[0] + actor->dimension[2] * -0.5f, tile_position[1] + actor->dimension[3] * +0.5f,0.1f,
                        tile_position[0] + actor->dimension[2] * +0.5f, tile_position[1] + actor->dimension[3] * +0.5f,0.1f,
                        tile_position[0] + actor->dimension[2] * +0.5f, tile_position[1] + actor->dimension[3] * -0.5f,0.1f,
                        tile_position[0] + actor->dimension[2] * -0.5f, tile_position[1] + actor->dimension[3] * -0.5f,0.1f,
                        tile_position[0] + actor->dimension[2] * -0.5f, tile_position[1] + actor->dimension[3] * +0.5f,0.1f,
                    };

                    vec3 draw_color = {1.0, 0.0, 0.0};
                    bvr_shader_set_uniform(&__editor->device.shader, "bvr_color", &draw_color);

                    bvri_bind_editor_buffers(__editor->device.array_buffer, __editor->device.vertex_buffer);
                    bvri_set_editor_buffers(vertices, sizeof(vertices) / sizeof(vec3), 3);
                    bvri_bind_editor_buffers(0, 0);
                    
                    __editor->draw_cmd.drawmode = BVR_DRAWMODE_LINE_STRIPE;
                    __editor->draw_cmd.element_offset = 0;
                    __editor->draw_cmd.element_count = sizeof(vertices) / sizeof(vec3);
                }
                

                bvri_draw_editor_shader(&actor->shader);
                bvri_draw_editor_image(&actor->atlas.image);
                bvri_draw_editor_mesh(&actor->mesh);
            }
            break;
        default:
            break;
        }

        __editor->device.is_gui_hovered |= nk_window_is_hovered(__editor->gui.context);
        nk_end(__editor->gui.context);
    }
}

void bvr_editor_render(){
    BVR_ASSERT(__editor);
    if(__editor->state == BVR_EDITOR_STATE_HIDDEN) {
        return;
    }

    BVR_ASSERT(__editor->state == BVR_EDITOR_STATE_DRAWING);
    __editor->state = BVR_EDITOR_STATE_RENDERING;

    if(__editor->draw_cmd.drawmode){
        bvr_shader_enable(&__editor->device.shader);

        bvri_bind_editor_buffers(__editor->device.array_buffer, __editor->device.vertex_buffer);
        bvri_draw_editor_buffer(__editor->draw_cmd.drawmode, __editor->draw_cmd.element_offset, __editor->draw_cmd.element_count);
        bvri_bind_editor_buffers(0, 0);

        bvr_shader_disable();

    }

    bvr_nuklear_render(&__editor->gui);
}

void bvr_destroy_editor(bvr_editor_t* editor){
    bvr_destroy_string(&editor->inspector_cmd.name);
    bvr_destroy_nuklear(&editor->gui);
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
    glDisableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return BVR_OK;
}

void bvri_bind_editor_buffers(uint32 array_buffer, uint32 vertex_buffer){
    glBindVertexArray(array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
}

void bvri_set_editor_buffers(float* vertices, uint32 vertices_count, uint8 stride){
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_count * stride * sizeof(float), vertices);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, stride, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    glDisableVertexAttribArray(0);
    
}

void bvri_draw_editor_buffer(int drawmode, uint32 element_offset, uint32 element_count){
    glEnableVertexAttribArray(0);
    glDrawArrays(drawmode, element_offset, element_count);
    glDisableVertexAttribArray(0);
}

void bvri_destroy_editor_render_buffers(uint32* array_buffer, uint32* vertex_buffer){
    glDeleteVertexArrays(1, array_buffer);
    glDeleteBuffers(1, vertex_buffer);
}