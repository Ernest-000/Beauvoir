#pragma once

#include <bvr/editor/editor.h>
#include <bvr/editor/editor_flags.h>

#include <bvr/common.h>

#ifndef BVR_NO_NUKLEAR

#define BVR_NK_WINDOW_DEFAULT (NK_WINDOW_BORDER | NK_WINDOW_TITLE)
#define BVR_ROW_HEIGHT (15)

#define BVR_NK_SEPARATOR(context) nk_label(context, "~~~~~~~~~~~~~~~~~~~~~", NK_TEXT_CENTERED)

#define BVR_NK_COMBO(value, e, name) int f ## ##e = value == e;\
    if(nk_checkbox_label(&__editor->gui.context, name, &(f ## ##e))){\
        value = e;\
    }

/* Generic components */
static void bvr_nk_draw_actor_component(bvr_canvas_t* context, void* user);
static void bvr_nk_draw_transform_component(bvr_canvas_t* context, void* user);
static void bvr_nk_draw_image_component(bvr_canvas_t* context, void* user);
static void bvr_nk_draw_shader_component(bvr_canvas_t* context, void* user);
static void bvr_nk_draw_mesh_component(bvr_canvas_t* context, void* user);

/* Page components */
static void bvr_nk_draw_asset_component(bvr_canvas_t* context, void* user);
static void bvr_nk_draw_camera_component(bvr_canvas_t* context, void* user);
static void bvr_nk_draw_gpipeline_component(bvr_canvas_t* context, void* user);
static void bvr_nk_draw_global_illumination_component(bvr_canvas_t* context, void* user);
static void bvr_nk_draw_user_component(bvr_canvas_t* context, void* user);

/* Actor components */
static void bvr_nk_draw_layer_actor_component(bvr_canvas_t* context, void* user);
static void bvr_nk_draw_static_actor_component(bvr_canvas_t* context, void* user);
static void bvr_nk_draw_dynamic_actor_component(bvr_canvas_t* context, void* user);
static void bvr_nk_draw_texture_actor_component(bvr_canvas_t* context, void* user);
static void bvr_nk_draw_landscape_actor_component(bvr_canvas_t* context, void* user);

BVR_H_FUNC void bvr_nk_draw_button_actor(bvr_editor_t* editor, struct bvr_actor_s* actor){
    BVR_ASSERT(editor);
    BVR_ASSERT(actor);
    
    if(!actor->name.string){
        return;
    }

    if(nk_button_label(&editor->gui.context, actor->name.string)){
        bvr_overwrite_string(&editor->inspector_cmd.name, actor->name.string, actor->name.length);

        editor->inspector_cmd.type = BVR_EDITOR_ACTOR;
        editor->inspector_cmd.object = actor;
        editor->inspector_cmd.component = NULL;

        switch (actor->type)
        {
        case BVR_LAYER_ACTOR:
            editor->inspector_cmd.component = bvr_nk_draw_layer_actor_component;
            break;
        case BVR_STATIC_ACTOR:
            editor->inspector_cmd.component = bvr_nk_draw_static_actor_component;
            break;
        case BVR_DYNAMIC_ACTOR:
            editor->inspector_cmd.component = bvr_nk_draw_dynamic_actor_component;
            break;
        case BVR_TEXTURE_ACTOR:
            editor->inspector_cmd.component = bvr_nk_draw_texture_actor_component;
            break;
        case BVR_LANDSCAPE_ACTOR:
            editor->inspector_cmd.component = bvr_nk_draw_landscape_actor_component;
            break;
        default:
            break;
        }
    }
}

BVR_H_FUNC void bvr_nk_draw_button(bvr_editor_t* editor, const char* name, uint32 type, void* object){
    BVR_ASSERT(editor);

    if(type == BVR_EDITOR_ACTOR){
        bvr_nk_draw_button_actor(editor, (struct bvr_actor_s*)object);
        return;
    }

    if(nk_button_label(&editor->gui.context, name)){
        bvr_overwrite_string(&editor->inspector_cmd.name, name, strlen(name));

        editor->inspector_cmd.type = BVR_EDITOR_ACTOR;
        editor->inspector_cmd.object = object;
        editor->inspector_cmd.component = NULL;

        switch (type)
        {
        case BVR_EDITOR_ASSETS:
            editor->inspector_cmd.component = bvr_nk_draw_asset_component;
            break;
        
        case BVR_EDITOR_CAMERA:
            editor->inspector_cmd.component = bvr_nk_draw_camera_component;
            break;

        case BVR_EDITOR_PIPELINE:
            editor->inspector_cmd.component = bvr_nk_draw_gpipeline_component;
            break;

        case BVR_EDITOR_USER:
            editor->inspector_cmd.object = editor;
            editor->inspector_cmd.component = bvr_nk_draw_user_component;
            break;
        default:
            break;
        }
    }
}

BVR_H_FUNC void bvr_nk_view_image(bvr_canvas_t* context, bvr_texture_t* texture){
    bvr_editor_t* editor = bvr_get_editor_instance();
    
    if(editor->draw_cmd.view_texture.id || editor->draw_cmd.view_fbo.framebuffer){
        BVR_PRINT("trying to render multiple views in the same frame :<");
        return;
    }
    
    struct nk_rect rect;
    struct nk_color color;
    struct nk_image image;
    
    color.r = 255.0;
    color.g = 255.0;
    color.b = 255.0;
    color.a = 255.0;
    
    rect.w = texture->image.width;
    rect.h = texture->image.height;

    // if texture has tiles
    if(texture->tiles.tile_per_column > 1 || texture->tiles.tile_per_row > 1){
        rect.w = texture->tiles.width;
        rect.h = texture->tiles.height;
    }

    image = nk_image_id(texture->id);

    if(nk_tree_push(&context->context, NK_TREE_NODE, "view", NK_MAXIMIZED)){
       
        if(nk_widget(&rect, &context->context)){
            nk_draw_image(
                &context->context.current->buffer,
                rect, &image,
                color
            );
        }
        
        nk_tree_pop(&context->context);
    }

    nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);
}

static void bvr_nk_draw_transform_component(bvr_canvas_t* context, void* user){
    bvr_transform_t* transform = (bvr_transform_t*)user;
    float scale = transform->scale[0];

    if(nk_tree_push(&context->context, NK_TREE_NODE, "transform", NK_MAXIMIZED)){
        nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 3);
        nk_property_float(&context->context, "#x", -100000.0f, &transform->position[0], 100000.0f, 0.1f, 0.1f);
        nk_property_float(&context->context, "#y", -100000.0f, &transform->position[1], 100000.0f, 0.1f, 0.1f);
        nk_property_float(&context->context, "#z", -100000.0f, &transform->position[2], 100000.0f, 0.1f, 0.1f);
        nk_property_float(&context->context, "#r", -100000.0f, &transform->rotation[0], 100000.0f, 1.0f, 1.0f);
        nk_property_float(&context->context, "#p", -100000.0f, &transform->rotation[1], 100000.0f, 1.0f, 1.0f);
        nk_property_float(&context->context, "#y", -100000.0f, &transform->rotation[2], 100000.0f, 1.0f, 1.0f);

        nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);
        nk_property_float(&context->context, "size", 0.0f, &scale, 100000.0f, 0.1f, 0.1f);
        BVR_SCALE_VEC3(transform->scale, scale);
        nk_tree_pop(&context->context);
    }

    nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);
}

static void bvr_nk_draw_actor_component(bvr_canvas_t* context, void* user){
    struct bvr_actor_s* actor = (struct bvr_actor_s*)user;

    if(nk_tree_push(&context->context, NK_TREE_TAB, "actor", NK_MAXIMIZED)){
        nk_checkbox_label(&context->context, "is active?", (nk_bool*)&actor->active);
        nk_property_short(&context->context, "draw order", 0, &actor->order_in_layer, BVR_INT16_MAX, 1, 1);
        bvr_nk_draw_transform_component(context, &actor->transform);
        nk_tree_pop(&context->context);
    }

    nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);
}

static void bvr_nk_draw_image_component(bvr_canvas_t* context, void* user){
    bvr_texture_t* texture = (bvr_texture_t*)user;
    char target[20];
    char format[20];


    if(nk_tree_push(&context->context, NK_TREE_TAB, "image", NK_MAXIMIZED)){
        nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 2);
                    
        nk_label(&context->context, BVR_FORMAT("width %i", texture->image.width), NK_TEXT_ALIGN_LEFT);
        nk_label(&context->context, BVR_FORMAT("height %i", texture->image.height), NK_TEXT_ALIGN_LEFT);
        nk_label_wrap(&context->context, BVR_FORMAT("depth %i", texture->image.depth));
        nk_label_wrap(&context->context, BVR_FORMAT("channels %i", texture->image.channels));

        bvr_nameof(texture->image.format, format);
        nk_label_wrap(&context->context, BVR_FORMAT("format %s", format));

        switch (texture->target)
        {
        case BVR_TEXTURE_2D:
            nk_label_wrap(&context->context, "target TEXTURE_2D");
            break;
        case BVR_TEXTURE_3D:
            nk_label_wrap(&context->context, "target TEXTURE_3D");
            break;
        case BVR_TEXTURE_2D_ARRAY:
            nk_label_wrap(&context->context, "target TEXTURE_2D_ARRAY");
            break;
        default: break;
        }
    
        // if is a texture atlas
        if(texture->tiles.tile_per_column > 1 || texture->tiles.tile_per_row > 1){
            nk_label(&context->context, BVR_FORMAT("tile width %i", texture->tiles.width), NK_TEXT_ALIGN_LEFT);
            nk_label(&context->context, BVR_FORMAT("tile height %i", texture->tiles.height), NK_TEXT_ALIGN_LEFT);
        }

        nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);
        // bvr_nk_view_image(context, texture);

        nk_tree_pop(&context->context);
    }

    nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);
}

static void bvr_nk_draw_shader_component(bvr_canvas_t* context, void* user){
    bvr_shader_t* shader = (bvr_shader_t*)user;

    if(nk_tree_push(&context->context, NK_TREE_TAB, "shader", NK_MAXIMIZED)){
        nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 2);

        nk_label_wrap(&context->context, BVR_FORMAT("shader id 0x%x", shader->program));
        nk_label_wrap(&context->context, BVR_FORMAT("pass count %i", shader->shader_count));
        nk_label_wrap(&context->context, BVR_FORMAT("uniform count %i", shader->uniform_count));
        nk_label_wrap(&context->context, BVR_FORMAT("block count %i", shader->block_count));

        nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);

        char type[25];
        if(nk_tree_push(&context->context, NK_TREE_NODE, "uniforms", NK_MINIMIZED)){
            nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 2);
            
            for (size_t i = 0; i < shader->shader_count; i++)
            {
                bvr_nameof(shader->shaders[i].type, type);

                nk_label_wrap(&context->context, BVR_FORMAT("0x%x", shader->shaders[i].shader));
                nk_label_wrap(&context->context, BVR_FORMAT("%s", type));
            }
            
            nk_tree_pop(&context->context);
        }

        if(nk_tree_push(&context->context, NK_TREE_NODE, "uniforms", NK_MINIMIZED)){
            nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 3);
            
            for (size_t i = 0; i < shader->uniform_count; i++)
            {
                bvr_nameof(shader->uniforms[i].type, type);

                nk_label_wrap(&context->context, BVR_FORMAT("%i", shader->uniforms[i].location));
                nk_label_wrap(&context->context, BVR_FORMAT("%s", shader->uniforms[i].name.string));
                nk_label_wrap(&context->context, BVR_FORMAT("%s", type));
            }
            
            
            nk_tree_pop(&context->context);
        }


        if(nk_tree_push(&context->context, NK_TREE_NODE, "blocks", NK_MINIMIZED)){
            nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 2);
            
            for (size_t i = 0; i < shader->block_count; i++)
            {
                bvr_nameof(shader->blocks[i].type, type);

                nk_label_wrap(&context->context, BVR_FORMAT("%i", shader->blocks[i].location));
                nk_label_wrap(&context->context, BVR_FORMAT("%s", type));
            }
            
            
            nk_tree_pop(&context->context);
        }

        nk_tree_pop(&context->context);
    }

    nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);
}

static void bvr_nk_draw_mesh_component(bvr_canvas_t* context, void* user){

}

static void bvr_nk_draw_asset_component(bvr_canvas_t* context, void* user){

}

static void bvr_nk_draw_camera_component(bvr_canvas_t* context, void* user){
    BVR_ASSERT(user);

    bvr_camera_t* camera = (bvr_camera_t*)user;

    if(nk_tree_push(&context->context, NK_TREE_TAB, "camera", NK_MAXIMIZED)){

        nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 2);
                    
        nk_label(&context->context, BVR_FORMAT("width %i", camera->framebuffer->width), NK_TEXT_ALIGN_LEFT);
        nk_label(&context->context, BVR_FORMAT("height %i", camera->framebuffer->height), NK_TEXT_ALIGN_LEFT);
        
        nk_label(&context->context, BVR_FORMAT("near %f", camera->near), NK_TEXT_ALIGN_LEFT);
        nk_label(&context->context, BVR_FORMAT("far %f", camera->far), NK_TEXT_ALIGN_LEFT);

        nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);
        nk_property_float(&context->context, "scale", 0.0f, &camera->field_of_view.scale, 100000.0f, 0.1f, 0.1f);

        bvr_nk_draw_transform_component(context, &camera->transform);
        nk_tree_pop(&context->context);
    }
 
    nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);
}

static void bvr_nk_draw_gpipeline_component(bvr_canvas_t* context, void* user){

}

static void bvr_nk_draw_global_illumination_component(bvr_canvas_t* context, void* user){

}

static void bvr_nk_draw_user_component(bvr_canvas_t* context, void* user){
    bvr_editor_t* p_editor = bvr_get_editor_instance();
    _bvr_editor_callback p_callback = (_bvr_editor_callback)p_editor->callback;
    
    BVR_CALL(p_callback, context, p_editor->book);
}

static void bvr_nk_draw_layer_actor_component(bvr_canvas_t* context, void* user){
    BVR_ASSERT(user);

    bvr_layer_actor_t* actor = (bvr_layer_actor_t*)user;

    nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);
    nk_label_wrap(&context->context, BVR_FORMAT("%s (bvr_layer_actor_t)", actor->self.name.string));

    nk_layout_row_dynamic(&context->context, 1, 1);
    bvr_nk_draw_actor_component(context, user);

    BVR_NK_SEPARATOR(&context->context);
    nk_label_wrap(&context->context, BVR_FORMAT("%s", actor->self.id));
}

static void bvr_nk_draw_static_actor_component(bvr_canvas_t* context, void* user){
    BVR_ASSERT(user);

    bvr_static_actor_t* actor = (bvr_static_actor_t*)user;

    nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);
    nk_label_wrap(&context->context, BVR_FORMAT("%s (bvr_static_actor_t)", actor->self.name.string));
    
    nk_layout_row_dynamic(&context->context, 1, 1);
    bvr_nk_draw_actor_component(context, user);

    BVR_NK_SEPARATOR(&context->context);
    nk_label_wrap(&context->context, BVR_FORMAT("%s", actor->self.id));
}

static void bvr_nk_draw_dynamic_actor_component(bvr_canvas_t* context, void* user){
    BVR_ASSERT(user);

    bvr_dynamic_actor_t* actor = (bvr_dynamic_actor_t*)user;

    nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);
    nk_label_wrap(&context->context, BVR_FORMAT("%s (bvr_dynamic_actor_t)", actor->self.name.string));

    nk_layout_row_dynamic(&context->context, 1, 1);
    bvr_nk_draw_actor_component(context, user);

    BVR_NK_SEPARATOR(&context->context);
    nk_label_wrap(&context->context, BVR_FORMAT("%s", actor->self.id));
}

static void bvr_nk_draw_texture_actor_component(bvr_canvas_t* context, void* user){
    BVR_ASSERT(user);

    bvr_texture_actor_t* actor = (bvr_texture_actor_t*)user;

    nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);
    nk_label_wrap(&context->context, BVR_FORMAT("%s (bvr_texture_actor_t)", actor->self.name.string));

    nk_layout_row_dynamic(&context->context, 1, 1);
    bvr_nk_draw_actor_component(context, &actor->self);
    bvr_nk_draw_image_component(context, &actor->texture);
    bvr_nk_draw_shader_component(context, &actor->shader);

    BVR_NK_SEPARATOR(&context->context);
    nk_label_wrap(&context->context, BVR_FORMAT("%s", actor->self.id));
}

static void bvr_nk_draw_landscape_actor_component(bvr_canvas_t* context, void* user){
    BVR_ASSERT(user);

    bvr_landscape_actor_t* actor = (bvr_landscape_actor_t*)user;

    nk_layout_row_dynamic(&context->context, BVR_ROW_HEIGHT, 1);
    nk_label_wrap(&context->context, BVR_FORMAT("%s (bvr_landscape_actor_t)", actor->self.name.string));

    nk_layout_row_dynamic(&context->context, 1, 1);
    bvr_nk_draw_actor_component(context, user);

    BVR_NK_SEPARATOR(&context->context);
    nk_label_wrap(&context->context, BVR_FORMAT("%s", actor->self.id));
}

#endif