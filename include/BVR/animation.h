#pragma once

#include <BVR/buffer.h>
#include <BVR/image.h>

enum bvr_animation_flags_e {
    BVR_ANIMATION_DO_ONCE = 0x1,
    BVR_ANIMATION_LOOP = 0x2
};

/**
 * This is an animation track. It will target a pointer that 
 * might change over the animation.
 */
typedef struct bvr_animation_handle_s {
    uint32 id;
    bvr_string_t name;

    union {
        void* ptr;
        uint64 id;
    } object;
    
    uint32 type;
    uint32 flags;
} bvr_animation_handle_t;   

struct bvr_keyframe_s {
    bvr_animation_handle_t* target;
    char buffer[20];
};

/**
 * Celluloid frame of an animation
 */
typedef struct bvr_celframe_s {
    float time;

    struct bvr_keyframe_s* keyframes;
    uint32 keyframes_count;

    struct bvr_celframe_s* next;
} bvr_celframe_t;

typedef struct bvr_animation_s {
    struct bvr_pool_s tracks;
    struct bvr_buffer_s celframes;

    enum bvr_animation_flags_e flags;

    // user defined start
    float start;

    // user defined end
    float end;

    float duration;
    float cursor;

    bvr_celframe_t* next_frame;

    // frame passed
    uint32 current_frame;
    uint32 total_frame;
} bvr_animation_t;

/**
 * Create a new animation object.
 */
int bvr_create_animation(bvr_animation_t* anim, float duration, const uint32 max_tracks, enum bvr_animation_flags_e flags);

void bvr_animation_update(bvr_animation_t* anim, float delta_time);

/**
 * Register a new animation track. This track will target a pointer that you can change over time.
 */
bvr_animation_handle_t* bvr_animation_register_track(bvr_animation_t* anim, const char* name, void* object, const int type, const uint32 flags);

int bvr_animation_add_keyframe_raw(bvr_animation_t* anim, bvr_animation_handle_t* handle, float time, void* value);

void bvr_destroy_animation(bvr_animation_t* anim);