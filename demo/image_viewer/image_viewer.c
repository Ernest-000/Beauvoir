/*
    This file contains the fundation of every Beauvoir projects.
*/

/* include all Beauvoir's headers */
#define BVR_GEOMETRY_IMPLEMENTATION
#include <BVR/bvr.h>

#include <BVR/editor/editor.h>

#define BVR_IMAGE_PLANE_NAME "image_plane"

// file functions
void dialog_callback(bvr_string_t* path);
struct bvr_actor_s* init_model(const char* path);

/* game's context */
static bvr_book_t book;
static bvr_editor_t editor;

int main(){
    struct bvr_actor_s* image_actor;
    
    /* create initial game's context */
    bvr_create_book(&book);
    bvr_create_page(&book.page, "scene");

    /* create the window */
    bvr_create_window(&book.window, 800, 800, "Window", 0);
    
    /* create editor context */
    bvr_create_editor(&editor, &book);

    /* create an audio stream */
    bvr_create_audio_stream(&book.audio, BVR_DEFAULT_SAMPLE_RATE, BVR_DEFAULT_AUDIO_BUFFER_SIZE);

    /* create an orthographic camera */
    bvr_create_orthographic_camera(&book.page, &book.window.framebuffer, 0.0f, 100.0f, 1.0f);

    image_actor = init_model("res/scene2.psd");

    /* main loop */
    while (1)
    {
        /* ask Beauvoir to prepare a new frame */
        bvr_new_frame(&book);

        /* quit the main loop if Beauvoir is not running */
        if(!bvr_is_awake() || bvr_key_down(BVR_KEY_ESCAPE)){
            break;
        }

        /* update colliders and physics */
        bvr_update(&book);

        bvr_draw_actor(image_actor, BVR_DRAWMODE_TRIANGLES);

        bvr_flush(&book);

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

/*
    Create or overwrite displayed image.
*/
struct bvr_actor_s* init_model(const char* path){
    bvr_image_t image;
    bvr_layer_actor_t image_plane;
    struct bvr_actor_s* actor;

    if(path == NULL){
        return NULL;
    }

    if((actor = bvr_find_actor(&book, BVR_IMAGE_PLANE_NAME)) != NULL){
        bvr_destroy_actor(actor);
    }

    bvr_create_image(&image, path);

    bvr_create_2d_square_mesh(&image_plane.mesh, image.width, image.height);

    bvr_destroy_image(&image);

    bvr_create_shader(&image_plane.shader, "res/unlit_layer.glsl", BVR_VERTEX_SHADER | BVR_FRAGMENT_SHADER);
    bvr_create_layered_texture(&image_plane.texture, path, BVR_TEXTURE_FILTER_LINEAR, BVR_TEXTURE_WRAP_CLAMP_TO_EDGE);

    bvr_shader_register_texture(&image_plane.shader, BVR_TEXTURE_2D_ARRAY, &image_plane.texture, "bvr_texture");
    bvr_shader_register_uniform(&image_plane.shader, BVR_INT32, 1, "bvr_texture_z");

    bvr_create_actor(&image_plane.object, BVR_IMAGE_PLANE_NAME, BVR_LAYER_ACTOR, BVR_COLLISION_DISABLE);
    actor = bvr_link_actor_to_page(&book.page, &image_plane.object);

    image_plane.object.transform.position[0] = 100.0f;
    image_plane.object.order_in_layer = 0;

    return actor;
}