#pragma once

#include <BVR/buffer.h>
#include <BVR/math.h>
#include <BVR/actors.h>
#include <BVR/camera.h>

#include <BVR/window.h>
#include <BVR/audio.h>

#include <BVR/lights.h>

#include <stdint.h>

#ifndef BVR_MAX_SCENE_ACTOR_COUNT
    #define BVR_MAX_SCENE_ACTOR_COUNT 64
#endif

#ifndef BVR_MAX_SCENE_LIGHT_COUNT
    #define BVR_MAX_SCENE_LIGHT_COUNT 16
#endif

#ifndef BVR_NO_SCENE_AUTO_HEAP
    #define BVR_SCENE_AUTO_HEAP
#endif

#ifndef BVR_NO_FPS_CAP
    #ifndef BVR_TARGET_FRAMERATE
        #define BVR_TARGET_FRAMERATE 60
    #endif

    #define BVR_FRAMERATE (1 / BVR_TARGET_FRAMERATE)
#endif


/*
    Contains all world's informations and data
*/
typedef struct bvr_page_s {
    bvr_string_t name;
    
    bvr_camera_t camera;
    bvr_global_illumination_t global_illumination;

    // all world's actors (pointers)
    bvr_pool_t actors;

    // all world lights
    bvr_pool_t lights;

    // all world's colliders (pointers)
    bvr_collider_collection_t colliders;
} bvr_page_t;

/*
    Contains all game's related data
*/
typedef struct bvr_book_s {
    bvr_window_t window;

    bvr_pipeline_t pipeline;

    bvr_audio_stream_t audio;

    // contains all assets informations
    // this might be used to store assets informations to export them as bundle
    bvr_memstream_t asset_stream;

    // for now, this is not used, but it aims to store 
    // all scene-actor heap relative elements
    bvr_memstream_t garbage_stream;

    bvr_page_t page;

    struct {
        float delta_time, frame_timer;
        int average_render_time, frames;
        uint64 prev_time, current_time;
    } timer;
} bvr_book_t;

/*
    Create a new game context
*/
int bvr_create_book(bvr_book_t* book);

bvr_book_t* bvr_get_book_instance();

/*
    Allocate scene's memory streams
*/
BVR_H_FUNC void bvr_create_book_memories(bvr_book_t* book, const uint64 asset_size, const uint64 garbage_size){
    if(!book->asset_stream.data && asset_size){
        bvr_create_memstream(&book->asset_stream, asset_size);        
    }

    if(garbage_size){
        bvr_destroy_memstream(&book->garbage_stream);
        bvr_create_memstream(&book->garbage_stream, garbage_size);
    }
}

/*
    Returns BVR_OK if the game is still running.
*/
BVR_H_FUNC int bvr_is_awake(bvr_book_t* book){
    return book->window.awake;
}

/*
    Returns BVR_OK is a scene is active.
*/
BVR_H_FUNC int bvr_is_active(bvr_book_t* book){
    return book->page.name.string != NULL;
}

/*
    ask Beauvoir to prepare a new frame
*/
void bvr_new_frame(bvr_book_t* book);

void bvr_update(bvr_book_t* book);

/*
    render all draw commands
*/
void bvr_flush(bvr_book_t* book);

/*
    swap framebuffers and push buffers to the screen
*/
void bvr_render(bvr_book_t* book);

void bvr_destroy_book(bvr_book_t* book);

/*
    Create a new scene
*/
int bvr_create_page(bvr_page_t* page, const char* name);

/*
    Set another page as the target one.
    Setting a new page will overwrite the previous page. Previous page will be freed.
*/
void bvr_enable_page(bvr_page_t* page);

/*
    Remove page from current working page
*/
void bvr_disable_page(bvr_page_t* page);

bvr_camera_t* bvr_create_orthographic_camera(bvr_page_t* page, bvr_framebuffer_t* framebuffer, float near, float far, float scale);


/*
    Set the view matrix of the camera.
*/
BVR_H_FUNC void bvr_camera_set_view(bvr_page_t* page, mat4x4 matrix){
    bvr_enable_uniform_buffer(page->camera.buffer);
    bvr_uniform_buffer_set(sizeof(mat4x4), sizeof(mat4x4), &matrix[0][0]);
    bvr_enable_uniform_buffer(0);
}

/*
    Transpose a screen-space coords into a world-space coord.
*/
void bvr_screen_to_world_coords(bvr_book_t* book, vec2 screen, vec3 world);

/*
    Register a new actor inside page's pool. 
    Return NULL if cannot register actor.
*/
struct bvr_actor_s* bvr_link_actor_to_page(bvr_page_t* page, struct bvr_actor_s* actor);

struct bvr_actor_s* bvr_find_actor(bvr_book_t* book, const char* name);

struct bvr_actor_s* bvr_find_actor_uuid(bvr_book_t* book, bvr_uuid_t uuid);

/*
    Register a new non-actor collider inside page's pool.
    Return NULL if cannot register collider.
*/
bvr_collider_t* bvr_link_collider_to_page(bvr_page_t* page, bvr_collider_t* collider);

void bvr_destroy_page(bvr_page_t* page);