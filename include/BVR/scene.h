#pragma once

#include <BVR/buffer.h>
#include <BVR/math.h>
#include <BVR/actors.h>

#include <BVR/window.h>
#include <BVR/audio.h>

#include <BVR/lights.h>
#include <BVR/camera.h>

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

    // scene's callbacks
    struct {
        void(*construct)(struct bvr_page_s* self);
        void(*load)(struct bvr_page_s* self);
        void(*update)(struct bvr_page_s* self);
        void(*destroy)(struct bvr_page_s* self);
    } events;

    bool is_available;
} bvr_page_t;

/*
    Contains all game's related data
*/
typedef struct bvr_book_s {
    // window object
    bvr_window_t window;

    // graphic pipeline
    bvr_pipeline_t pipeline;

    // audio channels and buffers
    bvr_audio_stream_t audio;

    // contains all assets informations
    // this might be used to store assets informations to export them as bundle
    bvr_memstream_t asset_stream;

    // for now, this is not used, but it aims to store 
    // all scene-actor heap relative elements
    bvr_memstream_t garbage_stream;

    // current page
    bvr_page_t page;

    // constant object predefitions
    // 
    struct bvr_predefs predefs;

    // time informations
    struct bvr_chrono_s {
        float delta_timef, frame_timer;
        int average_render_time, frames;
        uint64 prev_time, current_time;
    } timer;
} bvr_book_t;

typedef void(*bvr_page_event_t)(bvr_page_t* self);

/*
    Create a new game context
*/
int bvr_create_book(bvr_book_t* book);

bvr_book_t* bvr_get_instance();

/**
 * @brief Create and allocate common book's memory blocks (asset stream, garbage and predefs)
 * @param book
 * @param asset_size asset memory stream size (in bytes) 
 * @param garbage_size garbage memory stream size (in bytes)
 * @return (void)
 */
void bvr_create_book_memories(bvr_book_t* book, const uint64 asset_size, const uint64 garbage_size);

/*
    Returns BVR_TRUE if the game is still running.
*/
BVR_H_FUNC int bvr_is_awake(bvr_book_t* book){
    return book->window.awake;
}

BVR_H_FUNC int bvr_is_focus(bvr_book_t* book){
    return book->window.focus;
}

/*
    Returns BVR_TRUE is a scene is active.
*/
BVR_H_FUNC int bvr_is_active(bvr_book_t* book){
    return book->page.is_available;
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

/**
 * @brief create a new camera that target current page.
 * @param book 
 * @param mode ```BVR_CAMERA_ORTHOGRAPHIC``` or ```BVR_CAMERA_PERSPECTIVE```
 * @param near near plane distance
 * @param far far plane distance
 * @param scale fov or camera's scale (depending on camera's mode)
 * @return 
 */
BVR_H_FUNC void bvr_create_main_camera(bvr_book_t* book, int mode, int near, int far, int scale){
    bvr_create_camera(&book->page.camera, &book->window.framebuffer, mode, near, far, scale);
}

/*
    Set another page as the target one.
    Setting a new page will overwrite the previous page. Previous page will be freed.
*/
void bvr_enable_page(bvr_page_t* page);

/*
    Remove page from current working page
*/
void bvr_disable_page(bvr_page_t* page);

/*
    Register a new actor inside page's pool. 
    Return NULL if cannot register actor.
*/
struct bvr_actor_s* bvr_alloc_actor(bvr_page_t* page, bvr_actor_type_t type);
void bvr_free_actor(bvr_page_t* page, struct bvr_actor_s* actor);

struct bvr_actor_s* bvr_find_actor(bvr_book_t* book, const char* name);

struct bvr_actor_s* bvr_find_actor_uuid(bvr_book_t* book, bvr_uuid_t uuid);

/*
    Register a new non-actor collider inside page's pool.
    Return NULL if cannot register collider.
*/
bvr_collider_t* bvr_register_collider(bvr_page_t* page, bvr_collider_t* collider);

void bvr_destroy_page(bvr_page_t* page);