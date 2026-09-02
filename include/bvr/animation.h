#pragma once

#include <bvr/image.h>
#include <bvr/collections/buffer.h>
#include <bvr/collections/pool.h>
#include <bvr/collections/string.h>

#if !defined(BVR_INTERPOLATE)
    #define BVR_LINEAR_INTERPOLATE(_start, _end, t) (_start + ((_end - _start) * t))
#endif

#if !defined(BVR_ANIMATION_TIME_LEFT)
    #define BVR_ANIMATION_TIME_LEFT(_anim) (MIN((_anim).end, (_anim).duration) - (_anim).cursor)
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

enum bvr_behaviour_tree_state_e {
    BVR_BEHAVIOUR_TREE_STATE_IDLE,
    BVR_BEHAVIOUR_TREE_STATE_RETURN,
    BVR_BEHAVIOUR_TREE_STATE_DEFAULT,
    BVR_BEHAVIOUR_TREE_STATE_GOTO_0,
    BVR_BEHAVIOUR_TREE_STATE_GOTO_1,
    BVR_BEHAVIOUR_TREE_STATE_GOTO_2,
    BVR_BEHAVIOUR_TREE_STATE_GOTO_3,
    BVR_BEHAVIOUR_TREE_STATE_GOTO_4,
};  

enum bvr_behaviour_tree_flags_e {
    BVR_BEHAVIOUR_TREE_RETURNS,
    BVR_BEHAVIOUR_TREE_KEEP_ON_STATE
};  

// celframe opaque
struct bvr_celframe_s;

// keyframe opaque
struct bvr_keyframe_s;

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

    void(*interop_func)(struct bvr_keyframe_s* start, struct bvr_keyframe_s* end, float t);
} bvr_animation_handle_t;   

struct bvr_keyframe_s {
    bvr_animation_handle_t* target;
    float time;
    
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

struct bvr_behaviour_tree_state_s {
    bvr_string_t name;
    bvr_animation_t* animation;

    struct bvr_behaviour_tree_state_s* childs[BVR_MAX_STATE_LINK_COUNT];
    uint8 child_count;

    enum bvr_behaviour_tree_state_e next;
    enum bvr_behaviour_tree_flags_e flag;
};

/**
 * Animation state machine system.
 * This state machine is aimed to work more like a tree than like a graph.
 */
typedef struct bvr_behaviour_tree_s {
    struct bvr_behaviour_tree_state_s states[BVR_MAX_STATE_COUNT];

    // current animated state
    struct bvr_behaviour_tree_state_s* state;
    
    // state use as a default state
    struct bvr_behaviour_tree_state_s* default_state;
    
    uint32 state_count;
} bvr_behaviour_tree_t;
 
/**
 * Create a new animation object.
 */
int bvr_create_animation(bvr_animation_t* anim, float duration, const uint32 max_tracks, enum bvr_animation_flags_e flags);

/**
 * 
 */
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
 * This function aim to be generic. But you shall use it with does types: 
 * ```BVR_VEC2```,
 * ```BVR_VEC3```, 
 * ```BVR_VEC4```
 * 
 * @param anim 
 * @param track the name of the bvr_animation_handle_t that will be changed overtime
 * @param time when does this keyframe will be set
 * @param value keyframe's value
 */
BVR_H_FUNC int bvr_animation_add_keyframe(bvr_animation_t* anim, const char* track, float time, void* value){
    BVR_ASSERT(anim);
    BVR_ASSERT(track);

    bvr_animation_handle_t* handle;
    BVR_POOL_FOR_EACH(anim->tracks, handle)
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

/**
 * Restart animation to its first frame.
 */
BVR_H_FUNC void bvr_animation_restart(bvr_animation_t* anim){
    anim->current_frame = 0;
    anim->cursor = 0.0f;
    anim->next_frame = (bvr_celframe_t*)anim->celframes.data;
}

void bvr_destroy_animation(bvr_animation_t* anim);

void bvr_create_behaviour_tree(bvr_behaviour_tree_t* tree, const uint16 default_state);

/**
 * 
 */
void bvr_behaviour_tree_update(bvr_behaviour_tree_t* tree, float delta_time);

/**
 * Returns an existing state machine's name by using its name.
 */
BVR_H_FUNC struct bvr_behaviour_tree_state_s* bvr_behaviour_tree_get_state(bvr_behaviour_tree_t* tree, const char* name){
    
    for (size_t i = 0; i < tree->state_count; i++)
    {
        if(!tree->states[i].name.length){
            continue;
        }

        if(BVR_STRCMP(tree->states[i].name.string, name)){
            return &tree->states[i];
        }
    }
    
    return NULL;
}

/**
 * Add a new state to a state machine.
 * @param machine
 * @param parent new state's parent. If NULL, the new state will not be linked.
 * @param state state's pre-allocated object. You can use that to overwrite a previously created state.
 * If NULL, a new state will be allocated.
 * @param name new state's name.
 * @param anim the animation that will be triggered by the state.
 */
struct bvr_behaviour_tree_state_s* bvr_behaviour_tree_add_state_raw(bvr_behaviour_tree_t* tree, 
    struct bvr_behaviour_tree_state_s* const parent, struct bvr_behaviour_tree_state_s* state,
    const char* name, bvr_animation_t* anim, enum bvr_behaviour_tree_flags_e flag);

/**
 * Create a state machine's default state.
 * @param machine
 * @param name new state's name.
 * @param anim the animation that will be triggered by the state.
 */
BVR_H_FUNC struct bvr_behaviour_tree_state_s* bvr_behaviour_tree_add_default_state(bvr_behaviour_tree_t* tree, 
    const char* name, bvr_animation_t* anim, enum bvr_behaviour_tree_flags_e flag){
    
    return bvr_behaviour_tree_add_state_raw(tree, NULL, NULL, name, anim, flag);
}

/**
 * Create a state machine's default state.
 * @param machine
 * @param parent new state's parent. If NULL, the new state will not be linked.
 * @param name new state's name.
 * @param anim the animation that will be triggered by the state.
 */
BVR_H_FUNC struct bvr_behaviour_tree_state_s* bvr_behaviour_tree_add_state(bvr_behaviour_tree_t* tree, 
    const char* parent, const char* name, bvr_animation_t* anim, enum bvr_behaviour_tree_flags_e flag){
    
    return bvr_behaviour_tree_add_state_raw(
        tree, bvr_behaviour_tree_get_state(tree, parent),
        NULL, name, anim, flag
    );
}

void bvr_behaviour_tree_change_state_raw(bvr_behaviour_tree_t* tree, 
    struct bvr_behaviour_tree_state_s* state, enum bvr_behaviour_tree_state_e value);

BVR_H_FUNC void bvr_behaviour_tree_change_state(bvr_behaviour_tree_t* tree, const char* name, enum bvr_behaviour_tree_state_e value){
    BVR_ASSERT(tree);

    bvr_behaviour_tree_change_state_raw(tree, bvr_behaviour_tree_get_state(tree, name), value);
}

void bvr_destroy_behaviour_tree(bvr_behaviour_tree_t* machine);