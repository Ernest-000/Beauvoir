#pragma once

#include <bvr/buffer.h>
#include <bvr/image.h>

#if !defined(BVR_INTERPOLATE)
    #define BVR_LINEAR_INTERPOLATE(_start, _end, t) (_start + ((_end - _start) * t))
#endif

#if !defined(BVR_MAX_STATE_COUNT)
    #define BVR_MAX_STATE_COUNT 10
#endif

#if !defined(BVR_MAX_STATE_LINK_COUNT)
    #define BVR_MAX_STATE_LINK_COUNT 5
#endif

enum bvr_animation_flags_e {
    BVR_ANIMATION_NONE = 0x0,
    BVR_ANIMATION_LOOP = 0x2,
    BVR_ANIMATION_LINEAR_NEAREST = 0x4,
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
    /** 
     * a pool that contains all animation tracks.
     * a track represent a specific buffer to change over time.
    */
    struct bvr_pool_s tracks;

    /**
     * the list that contains all frames of the animations.
     */
    struct bvr_buffer_s celframes;

    enum bvr_animation_flags_e flags;

    // next frame to display
    bvr_celframe_t* next_frame;

    // user defined start
    float start;

    // user defined end
    float end;

    float duration;
    float cursor;

    // frame passed
    uint32 current_frame;
    uint32 total_frame;
} bvr_animation_t; 

typedef struct bvr_state_machine_s {
    struct bvr_state_machine_state_s {
        bvr_string_t name;

        struct bvr_state_machine_state_s* childs[BVR_MAX_STATE_LINK_COUNT];
        bvr_animation_t* animation;
        
        uint8 child_count;
        uint8 next;
    } states[BVR_MAX_STATE_COUNT];

    struct bvr_state_machine_state_s* next_state;
    struct bvr_state_machine_state_s* default_state;
    
    uint32 state_count;
} bvr_state_machine_t;
 
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
 * Add a new keyframe to a track. Copy value's data to keyframe's value.
 * Use this function for theses types : 
 * ```BVR_VEC2```,
 * ```BVR_VEC3```, 
 * ```BVR_VEC4```
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
        
        if (BVR_STRCMP(handle->name.string, track)) {
            return bvr_animation_add_keyframe_raw(anim, handle, time, value);
        }
    }
    
    return BVR_FALSE;
}

BVR_H_FUNC int bvr_animation_add_keyframe_i(bvr_animation_t* anim, const char* track, float time, int value){
    bvr_animation_add_keyframe(anim, track, time, &value);
}

BVR_H_FUNC int bvr_animation_add_keyframe_f(bvr_animation_t* anim, const char* track, float time, float value){
    bvr_animation_add_keyframe(anim, track, time, &value);
}

void bvr_destroy_animation(bvr_animation_t* anim);

void bvr_create_state_machine(bvr_state_machine_t* machine, const uint16 default_state);

BVR_H_FUNC struct bvr_state_machine_state_s* bvr_state_machine_get_state(bvr_state_machine_t* machine, const char* name){
    for (size_t i = 0; i < machine->state_count; i++)
    {
        if(!machine->states[i].name.length){
            continue;
        }

        if(BVR_STRCMP(machine->states[i].name.string, name)){
            return &machine->states[i];
        }
    }
    
    return NULL;
}

struct bvr_state_machine_state_s* bvr_state_machine_add_state_raw(bvr_state_machine_t* machine, 
    struct bvr_state_machine_state_s* const parent, const char* name, bvr_animation_t* anim);

BVR_H_FUNC struct bvr_state_machine_state_s* bvr_state_machine_add_state(bvr_state_machine_t* machine, 
    const char* parent, const char* name, bvr_animation_t* anim){
    
    return bvr_state_machine_add_state_raw(
        machine, bvr_state_machine_get_state(machine, parent),
        name, anim
    );
}

void bvr_destroy_state_machine(bvr_state_machine_t* machine);