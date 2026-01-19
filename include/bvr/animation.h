#pragma once

#include <bvr/buffer.h>
#include <bvr/image.h>

#if !defined(BVR_INTERPOLATE)
    #define BVR_LINEAR_INTERPOLATE(_start, _end, t) (_start + ((_end - _start) * t))
#endif

enum bvr_animation_flags_e {
    BVR_ANIMATION_NONE = 0x0,
    BVR_ANIMATION_LOOP = 0x2,
    BVR_ANIMATION_LINEAR_INTERPOLATE = 0x8
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

    void(*interop_func)(void* start, void* end, float t);
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

/**
 * Add a new keyframe to a track.
 */
int bvr_animation_add_keyframe_raw(bvr_animation_t* anim, bvr_animation_handle_t* handle, float time, void* value);

/**
 * Add a new keyframe to a track.
 */
BVR_H_FUNC int bvr_animation_add_keyframe(bvr_animation_t* anim, const char* track, float time, void* value){
    BVR_ASSERT(anim);
    BVR_ASSERT(track);

    bvr_animation_handle_t* handle;
    BVR_POOL_FOR_EACH(handle, anim->tracks)
    {
        if(!handle->name.length){
            continue;
        }
        
        if (strncmp(handle->name.string, track, handle->name.length) == 0) {
            return bvr_animation_add_keyframe_raw(anim, handle, time, value);
        }
    }
    
    return BVR_FALSE;
}

void bvr_destroy_animation(bvr_animation_t* anim);