/* include all Beauvoir's headers */
#define BVR_INCLUDE_GEOMETRY
#include <BVR/bvr.h>

#include <BVR/editor/editor.h>

#include <nuklear.h>

/* game's context */
static bvr_book_t book;
static bvr_editor_t editor;

static void _draw_editor(bvr_nuklear_t* gui, bvr_book_t* _);
static void _dialogue_box_callback(bvr_string_t* string);

static bvr_layer_actor_t* _load_image(const char* path);
static void _load_mesh(bvr_static_actor_t* actor, const char* path);

static bool target_image, wait_for_input; 
static bvr_layer_actor_t* p_image;

int main(){
    /* create initial game's context */
    bvr_create_book(&book);
    bvr_create_page(&book.page, "empty");
    
    bvr_create_book_memories(&book, BVR_BUFFER_SIZE, 0);

    /* create the window */
    bvr_create_window(&book.window, 800, 800, "Window", 0);
    
    /* create the camera */
    bvr_create_main_camera(&book, BVR_CAMERA_ORTHOGRAPHIC, 0.0f, 1000.0f, 1.0f);

    bvr_create_editor(&editor, &book);
    bvr_editor_attach_callback(_draw_editor);

    target_image = true;
    wait_for_input = false;
    p_image = _load_image("samples/template.psd");

    /* main loop */
    while (1)
    {
        /* ask Beauvoir to prepare a new frame */
        bvr_new_frame(&book);

        /* quit the main loop if Beauvoir is not running */
        if(!bvr_is_awake(&book)){
            break;
        }

        /* update colliders and physics */
        bvr_update(&book);

        if(target_image){
            bvr_draw_actor(&p_image->self, BVR_DRAWMODE_TRIANGLES);
        }
        else {

        }

        // draw editor
        bvr_editor_handle();
        bvr_editor_draw_page_hierarchy();
        bvr_editor_draw_inspector();
        bvr_editor_render();

        /* push Beauvoir's graphics to the window */
        bvr_render(&book);
    }
    
    /* free */
    bvr_destroy_book(&book);

    return 0;
}

static void _dialogue_box_callback(bvr_string_t* string){
    if(!string->string){
        return;
    }

    if(target_image){
        // destroy previous actor
        bvr_destroy_actor(&p_image->self);
        
        // create another one :3
        p_image = _load_image(string->string);
    }
    else {

    }

    wait_for_input = false;
}

static void _draw_editor(struct bvr_nuklear_s* gui, bvr_book_t* _){
    nk_layout_row_dynamic(gui->context, 40, 1);
    
    if(nk_button_label(gui->context, "load image")){
        target_image = true;
        wait_for_input = true;
        bvr_open_file_dialog(_dialogue_box_callback, NULL, 0);
    }

    if(nk_button_label(gui->context, "load mesh")){
        target_image = false;
        wait_for_input = true;
        bvr_open_file_dialog(_dialogue_box_callback, NULL, 0);
    }

    nk_label_wrap(gui->context, BVR_FORMAT("texture id %i", p_image->texture.id));
}


static bvr_layer_actor_t* _load_image(const char* path){
    BVR_ASSERT(path);

    bvr_layer_actor_t* p_actor = NULL;
    
    // link actor to the scene
    // WARN: actor dynamic components (eg: textures, layers, transform...) that might
    // change overtime MUST link to the new allocated pointer, not the older object!
    p_actor = (bvr_layer_actor_t*) bvr_alloc_actor(&book.page, BVR_LAYER_ACTOR);
    bvr_create_actor(&p_actor->self, "image", BVR_LAYER_ACTOR, BVR_COLLISION_DISABLE);

    // create shader
    bvr_create_shader(&p_actor->shader, "texture_unlit.glsl", BVR_VERTEX_SHADER | BVR_FRAGMENT_SHADER);
    
    // create texture
    bvr_create_layered_texture(&p_actor->texture, path, BVR_TEXTURE_FILTER_LINEAR, BVR_TEXTURE_WRAP_CLAMP_TO_EDGE);
    
    // create mesh
    bvr_create_2d_square_mesh(&p_actor->mesh, p_actor->texture.image.width, p_actor->texture.image.height);

    // link texture & shader
    bvr_shader_register_texture(&p_actor->shader, BVR_TEXTURE_2D_LAYER, &p_actor->texture, "bvr_texture");
    
    // define which uniform defines texture layer index
    bvr_shader_register_uniform(&p_actor->shader, BVR_INT32, BVR_UNIFORM_LAYER_INDEX, 1, "bvr_texture_z");

    return p_actor;
}

static void _load_mesh(bvr_static_actor_t* actor, const char* path){

}