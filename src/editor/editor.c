#include <BVR/editor/editor.h>
#include <BVR/editor/flags.h>
#include <BVR/editor/landscape.h>

#include <BVR/buffer.h>
#include <BVR/common.h>
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

#define BVR_HIERARCHY_RECT(w, h) (nk_rect(0, 0, (200 + w) / BVR_EDITOR_SCALE, (450 + h) / BVR_EDITOR_SCALE))
#define BVR_INSPECTOR_RECT(w, h) (nk_rect(__editor->book->window.framebuffer.width - (350 + w) / BVR_EDITOR_SCALE, 0, (350 + w) / BVR_EDITOR_SCALE, (400 + h) / BVR_EDITOR_SCALE))

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
        
        nk_property_float(__editor->gui.context, "ro", -100000.0f, &transform->rotation[0], 100000.0f, 0.1f, 0.1f);
        nk_property_float(__editor->gui.context, "pi", -100000.0f, &transform->rotation[1], 100000.0f, 0.1f, 0.1f);
        nk_property_float(__editor->gui.context, "ya", -100000.0f, &transform->rotation[2], 100000.0f, 0.1f, 0.1f);
        
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
    
    nk_layout_row_dynamic(__editor->gui.context, 15, 2);
    
    bvr_vertex_group_t* group;
    for(int i = 0; i < mesh->vertex_groups.count; i++){
        group = bvr_pool_try_get(&mesh->vertex_groups, i);

        nk_label_wrap(__editor->gui.context, 
            BVR_FORMAT("%s: %i-%i", group->name.string, group->element_offset, group->element_offset + group->element_count)
        );

        int flag = BVR_HAS_FLAG(group->flags, BVR_VERTEX_GROUP_FLAG_INVISIBLE);
        if(nk_checkbox_label(__editor->gui.context, "visible", &flag)){
            group->flags ^= BVR_VERTEX_GROUP_FLAG_INVISIBLE;
        }
    }

    nk_layout_row_dynamic(__editor->gui.context, 40, 1);

    if (nk_group_begin(__editor->gui.context, BVR_FORMAT("mesh%x", &mesh), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic(__editor->gui.context, 15, 2);

        nk_label(__editor->gui.context, BVR_FORMAT("vertex count %i", mesh->vertex_count), NK_TEXT_ALIGN_LEFT);
        nk_label(__editor->gui.context, BVR_FORMAT("element count %i", mesh->element_count), NK_TEXT_ALIGN_LEFT);

        nk_label(__editor->gui.context, BVR_FORMAT("vertex buffer '0x%x'", mesh->vertex_buffer), NK_TEXT_ALIGN_LEFT);
        nk_label(__editor->gui.context, BVR_FORMAT("element buffer '0x%x'", mesh->element_buffer), NK_TEXT_ALIGN_LEFT);

        nk_layout_row_dynamic(__editor->gui.context, 15, 1);
        
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

        if(nk_checkbox_label(__editor->gui.context, "is visible", (nk_bool*)&layer->opacity)){        
            layer->opacity *= 255;
        }

        flag = BVR_HAS_FLAG(layer->flags, BVR_LAYER_Y_SORTED);
        if(nk_checkbox_label(__editor->gui.context, "y sorted", &flag)){
            layer->flags ^= BVR_LAYER_Y_SORTED;
        }

        nk_property_int(__editor->gui.context, "#x", -100000, (int*)&layer->anchor_x, 100000, 1, 1);
        nk_property_int(__editor->gui.context, "#y", -100000, (int*)&layer->anchor_y, 100000, 1, 1);

        nk_group_end(__editor->gui.context);
    }
    
}

static void bvri_draw_editor_shader(bvr_shader_t* shader){
    if(!shader){
        return;
    }

    nk_layout_row_dynamic(__editor->gui.context, 15, 1);
    nk_label(__editor->gui.context, "Shader", NK_TEXT_ALIGN_CENTERED);    

    if(shader->asset.origin != BVR_ASSET_ORIGIN_NONE){
        nk_label(__editor->gui.context, shader->asset.pointer.asset_id, NK_TEXT_ALIGN_LEFT);
    }
    
    if(shader->uniform_count){
        nk_layout_row_dynamic(__editor->gui.context, (shader->uniform_count + 3) * 15, 1);
        if(nk_group_begin_titled(__editor->gui.context, "uniformgroupe", "Uniforms", NK_WINDOW_BORDER | NK_WINDOW_TITLE)){  
            nk_layout_row_dynamic(__editor->gui.context, 15, 2);

            char type_name[16];
            for (size_t i = 0; i < shader->uniform_count; i++)
            {
                bvr_nameof(shader->uniforms[i].type, type_name);

                nk_label_wrap(__editor->gui.context, BVR_FORMAT("%s", shader->uniforms[i].name.string));  

                nk_label_wrap(__editor->gui.context, BVR_FORMAT("%s", type_name));     
            }

            nk_group_end(__editor->gui.context);
        }
    }

    if(shader->block_count){
        nk_layout_row_dynamic(__editor->gui.context, (shader->block_count + 3) * 15, 1);

        if(nk_group_begin_titled(__editor->gui.context, "blockgroupe", "Blocks", NK_WINDOW_BORDER | NK_WINDOW_TITLE)){
            nk_layout_row_dynamic(__editor->gui.context, 15, 2);

            char type_name[16];
            for (size_t i = 0; i < shader->block_count; i++)
            {
                bvr_nameof(shader->blocks[i].type, type_name);

                nk_label_wrap(__editor->gui.context, BVR_FORMAT("block%i", shader->blocks[i].location));  
                nk_label_wrap(__editor->gui.context, BVR_FORMAT("%s", type_name));     
            }

            nk_group_end(__editor->gui.context);
        }
    }
    
}

static void bvri_draw_hierarchy_button(const char* name, uint64 type, void* object){
    // if there is no name, or text, the button shall not appear
    if(!name){
        return;
    }

    if(nk_button_label(__editor->gui.context, name)){

        bvr_destroy_string(&__editor->inspector_cmd.name);
        bvr_create_string(&__editor->inspector_cmd.name, name);

        __editor->inspector_cmd.type = type;
        __editor->inspector_cmd.pointer = object;
    }
}

static void bvri_load_landscape(bvr_string_t* path){
    BVR_PRINTF("load map %s", path->string);

    bvr_landscape_actor_t* actor = (bvr_landscape_actor_t*)__editor->inspector_cmd.pointer;
    bvr_landscape_load(path->string, actor);
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
    editor->callback = NULL;
    editor->state = BVR_EDITOR_STATE_HANDLE;
    editor->inspector_cmd.pointer = NULL;
    editor->inspector_cmd.type = 0;
    editor->draw_cmd.drawmode = 0;
    editor->draw_cmd.element_offset = 0;
    editor->draw_cmd.element_count = 0;

    memset(&editor->memory, 0, sizeof(editor->memory));

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
        
        const char* shaders[2] = { vertex_shader, fragment_shader };
        BVR_ASSERT(bvr_create_shader_raw(&editor->device.shader, shaders, BVR_VERTEX_SHADER | BVR_FRAGMENT_SHADER));
        BVR_ASSERT(bvr_shader_register_uniform(&editor->device.shader, BVR_VEC3, 1, 0, "bvr_color"));
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

void bvr_editor_attach_callback(_bvr_editor_callback function){
    BVR_ASSERT(function);
    
    if(__editor){
        __editor->callback = function;
    }
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
                nk_layout_row_begin(__editor->gui.context, NK_STATIC, 25, 3); 
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

                if(nk_menu_begin_label(__editor->gui.context, "import", NK_TEXT_ALIGN_LEFT, nk_vec2(100, 100))){
                    nk_layout_row_dynamic(__editor->gui.context, 15, 1);

                    if(nk_menu_item_label(__editor->gui.context, "tilemap", NK_TEXT_ALIGN_LEFT)){
                        if(__editor->inspector_cmd.type == BVR_EDITOR_LANDSCAPE){
                            bvr_open_file_dialog(bvri_load_landscape, NULL, 0);
                        }
                        else {
                            BVR_PRINT("you're not selecting a landscape actor :/");
                        }
                    }

                    nk_menu_end(__editor->gui.context);
                }
            }
            nk_menubar_end(__editor->gui.context);
        }
        
        {
            nk_layout_row_dynamic(__editor->gui.context, 15, 1);
            nk_label_wrap(__editor->gui.context, BVR_FORMAT("fps %i", __editor->book->timer.average_render_time));
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

            if(__editor->callback){
                bvri_draw_hierarchy_button("user", BVR_EDITOR_USER, __editor->callback);
            }
            
            struct bvr_light_s* light = NULL;
            BVR_POOL_FOR_EACH(light, __editor->book->page.lights){
                if(!light) {
                    break;
                }

                switch (light->type)
                {
                case BVR_LIGHT_NONE: break;
                case BVR_LIGHT_GLOBAL_ILLUMINATION:
                    bvri_draw_hierarchy_button("global illumination", BVR_EDITOR_GLOBAL_ILL, light);
                    break;

                default:
                    break;
                }
            }

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

                nk_label(__editor->gui.context, BVR_FORMAT("render time %f ms", __editor->book->timer.delta_timef), NK_TEXT_ALIGN_LEFT);
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

                if(__editor->book->predefs.is_available){
                    nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                    bvri_draw_editor_shader(&__editor->book->predefs.c_shaders.c_invalid_shader);
        
 
                    nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                    bvri_draw_editor_shader(&__editor->book->predefs.c_shaders.c_framebuffer_shader);
                    
                    nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                    bvri_draw_editor_shader(&__editor->book->predefs.c_shaders.c_composite_shader);
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
                    bvr_open_file_dialog(bvri_editor_import_asset, NULL, 0);
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
                case BVR_TEXTURE_ACTOR:
                    {
                        nk_label(__editor->gui.context, "BITMAP ACTOR", NK_TEXT_ALIGN_CENTERED);
                    }
                    break;
                case BVR_STATIC_ACTOR:
                    {
                        nk_label(__editor->gui.context, "STATIC ACTOR", NK_TEXT_ALIGN_CENTERED);
                        bvri_draw_editor_mesh(&((bvr_static_actor_t*)actor)->mesh);
                        bvri_draw_editor_shader(&((bvr_static_actor_t*)actor)->shader);
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
                int target_tile = 0;
                struct bvr_tile_s tile;
                bvr_landscape_actor_t* landscape = (bvr_landscape_actor_t*)__editor->inspector_cmd.pointer;
                
                nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                nk_label(__editor->gui.context, BVR_FORMAT("id %s", landscape->self.id), NK_TEXT_ALIGN_LEFT);
                nk_label(__editor->gui.context, BVR_FORMAT("flags %x", landscape->self.flags), NK_TEXT_ALIGN_LEFT);

                nk_checkbox_label(__editor->gui.context, "is active", (nk_bool*)&landscape->self.active);

                bvri_draw_editor_transform(&landscape->self.transform);

                    nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                nk_label(__editor->gui.context, "LANDSCAPE", NK_TEXT_ALIGN_CENTERED);

                nk_layout_row_dynamic(__editor->gui.context, 15, 2);
                nk_property_int(__editor->gui.context, "tile x", 0, &__editor->memory.landscape.cursor[0], landscape->dimension.count[0] - 1, 1, .5f);
                nk_property_int(__editor->gui.context, "tile y", 0, &__editor->memory.landscape.cursor[1], landscape->dimension.count[1] - 1, 1, .5f);

                target_tile = bvri_landscape_process_selection(__editor, landscape);
                tile = bvri_landscape_get_tile(landscape, target_tile); 

                nk_layout_row_dynamic(__editor->gui.context, 100, 1);
                if(nk_group_begin(__editor->gui.context, BVR_FORMAT("tile %i", bvri_landscape_id(landscape, target_tile)), NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_SCALABLE)){
                    nk_layout_row_dynamic(__editor->gui.context, 15, 1);

                    int altitude, texture;
                    altitude = tile.altitude;
                    texture = tile.texture;
                    nk_property_int(__editor->gui.context, "altitude", 0, &altitude, 256, 0, 0);
                    nk_property_int(__editor->gui.context, "texture", 0, &texture, 256, 0, 0);

                    nk_group_end(__editor->gui.context);
                }

                nk_layout_row_dynamic(__editor->gui.context, 15, 1);

                // drawing tile selector gizmo
                {
                    vec2 tile_position, half_size;
                    tile_position[0] = __editor->memory.landscape.cursor[0];
                    tile_position[1] = __editor->memory.landscape.cursor[1];

                    half_size[0] = landscape->dimension.resolution[0] * 0.5f;
                    half_size[1] = landscape->dimension.resolution[1] * 0.5f - landscape->dimension.resolution[1];

                    vec2_add(tile_position, landscape->self.transform.position, tile_position);
                    tile_position[0] *= landscape->dimension.resolution[0];
                    tile_position[1] *= -landscape->dimension.resolution[1];

                    vec2_add(tile_position, half_size, tile_position);

                    float vertices[] = {
                        tile_position[0] + landscape->dimension.resolution[0] * -0.5f, tile_position[1] + landscape->dimension.resolution[1] * +0.5f,0.1f,
                        tile_position[0] + landscape->dimension.resolution[0] * +0.5f, tile_position[1] + landscape->dimension.resolution[1] * +0.5f,0.1f,
                        tile_position[0] + landscape->dimension.resolution[0] * +0.5f, tile_position[1] + landscape->dimension.resolution[1] * -0.5f,0.1f,
                        tile_position[0] + landscape->dimension.resolution[0] * -0.5f, tile_position[1] + landscape->dimension.resolution[1] * -0.5f,0.1f,
                        tile_position[0] + landscape->dimension.resolution[0] * -0.5f, tile_position[1] + landscape->dimension.resolution[1] * +0.5f,0.1f,
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
                

                bvri_draw_editor_shader(&landscape->shader);
                bvri_draw_editor_mesh(&landscape->mesh);
            }
            break;
        case BVR_EDITOR_GLOBAL_ILL:
            {
                bvr_global_illumination_t* illumination = (bvr_global_illumination_t*) __editor->inspector_cmd.pointer;

                nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                nk_label(__editor->gui.context, "INTENSITY", NK_TEXT_ALIGN_CENTERED);
                nk_property_float(__editor->gui.context, "light", 0.0, &illumination->light.intensity, 255, 1.0f, 1.0f);
                nk_property_float(__editor->gui.context, "ambiant", 0.0, &illumination->light.position[3], 255, 1.0f, 1.0f);
                
                nk_label(__editor->gui.context, "LIGHT COLOR", NK_TEXT_ALIGN_CENTERED);
                nk_layout_row_dynamic(__editor->gui.context, BVR_INSPECTOR_RECT(0, 0).w / 2, 1);
                nk_color_pick(__editor->gui.context, (struct nk_colorf*)&illumination->light.color, NK_RGB);
                
                nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                nk_label(__editor->gui.context, "LIGHT POSITION", NK_TEXT_ALIGN_CENTERED);
                nk_layout_row_dynamic(__editor->gui.context, 15, 3);
                nk_property_float(__editor->gui.context, "x", -100000.0f, &illumination->light.position[0], 100000.0f, 0.1f, 0.1f);
                nk_property_float(__editor->gui.context, "y", -100000.0f, &illumination->light.position[1], 100000.0f, 0.1f, 0.1f);
                nk_property_float(__editor->gui.context, "z", -100000.0f, &illumination->light.position[2], 100000.0f, 0.1f, 0.1f);
                
                // draw light gizmo
                {
                    float distance = (illumination->light.intensity);
                    vec3 normalized_dir;
                    vec3_norm(normalized_dir, illumination->light.direction);
                    vec3_scale(normalized_dir, normalized_dir, distance);

                    float vertices[6] = {
                        illumination->light.position[0], 
                        illumination->light.position[1], 
                        illumination->light.position[2],
                        illumination->light.position[0], 
                        illumination->light.position[1] - distance, 
                        illumination->light.position[2]
                    };

                    bvri_bind_editor_buffers(__editor->device.array_buffer, __editor->device.vertex_buffer);
                    bvri_set_editor_buffers(vertices, sizeof(vertices) / sizeof(vec3), 3);
                    bvri_bind_editor_buffers(0, 0);
                    
                    __editor->draw_cmd.drawmode = BVR_DRAWMODE_LINE_STRIPE;
                    __editor->draw_cmd.element_offset = 0;
                    __editor->draw_cmd.element_count = sizeof(vertices) / sizeof(vec3);
                }
            }
            break;
        
        case BVR_EDITOR_USER:
            __editor->callback(&__editor->gui, __editor->book);
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

    return BVR_TRUE;
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