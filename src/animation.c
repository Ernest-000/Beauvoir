#include <BVR/animation.h>

#include <BVR/scene.h>

#include <memory.h>
#include <malloc.h>

#define BVR_DEFAULT_FRAME_COUNT 2

/**
 * Linear interpolation between two keyframes
 */
static void bvri_animation_lerp(void* _start, void* _end, float time){
    
    struct bvr_keyframe_s* start = (struct bvr_keyframe_s*)_start;
    struct bvr_keyframe_s* end = (struct bvr_keyframe_s*)_end;
    
    switch (start->target->type)
    {
    case BVR_FLOAT:
        {
            float lerp = 0.0f;
            // interpolate default position with current pointer's position
            lerp = BVR_LINEAR_INTERPOLATE(*(float*)start->target->object.ptr, *(float*)start->buffer, time) * 2.0f;
            
            // interpolate with the end position
            lerp = BVR_LINEAR_INTERPOLATE(lerp, *(float*)end->buffer, time);
            
            // copy the value
            memcpy(start->target->object.ptr, &lerp, sizeof(float));        
        }    
    
        break;
    
    default:
        memcpy(start->target->object.ptr, start->buffer, bvr_sizeof(start->target->type));        
        break;
    }
}

/**
 * Initialize a celframe with default values
 */
static bvr_celframe_t* bvri_create_celframe(bvr_animation_t* anim, bvr_celframe_t* frame, float time);

/**
 * Insert a new celframe before another
 */
static bvr_celframe_t* bvri_insert_new_celframe(bvr_animation_t* anim, bvr_celframe_t* head, float time);

/**
 * Enable and execute a celframe
 */
static void bvri_do_celframe(bvr_animation_t* anim, bvr_celframe_t* frame);

static void bvri_destroy_celframe(bvr_celframe_t* frame);

int bvr_create_animation(bvr_animation_t* anim, float duration, const uint32 max_target, enum bvr_animation_flags_e flags){
    BVR_ASSERT(anim);
    BVR_ASSERT(duration > 0);

    anim->next_frame = NULL;
    anim->start = 0.0f;
    anim->flags = flags;
    anim->cursor = 0.0f;
    anim->end = duration;
    anim->current_frame = 0;
    anim->duration = duration;
    anim->total_frame = duration / BVR_TARGET_FRAMERATE;

    anim->celframes.elemsize = sizeof(bvr_celframe_t);
    anim->celframes.size = BVR_DEFAULT_FRAME_COUNT * sizeof(bvr_celframe_t);
    anim->celframes.data = malloc(anim->celframes.size);
    BVR_ASSERT(anim->celframes.data);

    bvr_create_pool(&anim->tracks, sizeof(struct bvr_animation_handle_s), max_target);

    // create the first and the last celframe of the animation
    // theses will set anim's bounds
    {
        // create default frames
        bvr_celframe_t* start = bvri_create_celframe(anim, (bvr_celframe_t*)anim->celframes.data, anim->start);
        bvr_celframe_t* end = bvri_create_celframe(anim, &((bvr_celframe_t*)anim->celframes.data)[1], anim->end);
        
        // init default values for the start and end keyframes
        memset(&start->keyframes[0], 0, sizeof(struct bvr_keyframe_s));
        memset(&end->keyframes[0], 0, sizeof(struct bvr_keyframe_s));

        // link frames
        start->next = end;
        anim->next_frame = start;
    }    
}

void bvr_animation_update(bvr_animation_t* anim, float delta_time){
    /**
     * for now time if second based but it's maybe better with a frame based counter ?
     */
    uint64 elapsed_time = bvr_get_instance()->timer.current_time - bvr_get_instance()->timer.prev_time;

    anim->cursor += delta_time;
    anim->current_frame += elapsed_time;

    bvri_do_celframe(anim, anim->next_frame);

    // activate frame
    if(anim->cursor >= anim->next_frame->time){
        anim->next_frame = anim->next_frame->next;
    }

    // when animation is finished
    if(anim->cursor >= MIN(anim->end, anim->duration)){
        anim->cursor = anim->duration;
        anim->current_frame = 0;

        // loop
        if(BVR_HAS_FLAG(anim->flags, BVR_ANIMATION_LOOP)){
            anim->cursor = MAX(0.0f, anim->start);

            // reset to be the first frame
            anim->next_frame = (bvr_celframe_t*)anim->celframes.data;
        }
    }
}

bvr_animation_handle_t* bvr_animation_register_track(bvr_animation_t* anim, const char* name, void* object, const int type, const uint32 flags){
    BVR_ASSERT(anim);
    BVR_ASSERT(BVR_IS_AVAIL_TYPE(type));
    BVR_ASSERT(bvr_sizeof(type) < sizeof_member(struct bvr_keyframe_s, buffer)); // max keyframe buffer size

    if(!object){
        return NULL;
    }

    // avoid overflows
    if(anim->tracks.count >= anim->tracks.capacity){
        BVR_PRINT("cannot add more track...");
        return NULL;
    }

    bvr_animation_handle_t* handle = (bvr_animation_handle_t*) bvr_pool_alloc(&anim->tracks);
    BVR_ASSERT(handle);

    handle->id = anim->tracks.count - 1;
    handle->object.ptr = object;
    handle->type = type;
    handle->flags = flags;
    handle->interop_func = NULL;

    if(BVR_HAS_FLAG(flags, BVR_ANIMATION_LINEAR_INTERPOLATE)){
        handle->interop_func = bvri_animation_lerp;
    }

    bvr_create_string(&handle->name, name);

    return handle;
}

int bvr_animation_add_keyframe_raw(bvr_animation_t* anim, bvr_animation_handle_t* handle, float time, void* value){
    BVR_ASSERT(anim);
    BVR_ASSERT(handle);
    BVR_ASSERT(time >= 0.0f && time <= anim->duration);

    bvr_celframe_t* cel = NULL;
    bvr_celframe_t* avail_cel = (bvr_celframe_t*)anim->celframes.data;

    while (avail_cel)
    {
        if(avail_cel->time == time){
            // we found a frame no need to allocate a new one!
            cel = avail_cel;
            break;
        }
        else if(avail_cel->time > time){
            // we want to insert a new frame before avail_cel
            break;
        }

        avail_cel = avail_cel->next;
    }

    // if we didn't found an existing frame we need to create a new one!
    if(!cel){
        // insert it before
        cel = bvri_insert_new_celframe(anim, (bvr_celframe_t*)anim->celframes.data, time);
        BVR_ASSERT(cel);
    }

    if(handle->id < cel->keyframes_count){
        cel->keyframes[handle->id].target = handle;
        
        memcpy(cel->keyframes[handle->id].buffer, value, bvr_sizeof(handle->type));
    }
}

void bvr_destroy_animation(bvr_animation_t* anim){
    BVR_ASSERT(anim);

    for (size_t i = 0; i < BVR_BUFFER_COUNT(anim->celframes); i++)
    {
        bvri_destroy_celframe(&((bvr_celframe_t*)anim->celframes.data)[i]);
    }
    
    bvr_animation_handle_t handle;
    BVR_POOL_FOR_EACH(handle, anim->tracks){
        bvr_destroy_string(&handle.name);
    }

    bvr_destroy_pool(&anim->tracks);

    free(anim->celframes.data);
    anim->celframes.data = NULL;
}

static bvr_celframe_t* bvri_create_celframe(bvr_animation_t* anim, bvr_celframe_t* frame, float time){
    BVR_ASSERT(frame);
    
    frame->next = NULL;
    frame->time = time;

    // the max keyframe count is clamped to the number of track of the animation
    // this avoid increasing all celframes each time a new keyframe is added
    // plus this keeps keyframe at their relative index depending on the track
    // their affected
    frame->keyframes_count = anim->tracks.capacity;
    frame->keyframes = calloc(frame->keyframes_count, sizeof(struct bvr_keyframe_s));
    BVR_ASSERT(frame->keyframes);
    
    return frame;
}

static bvr_celframe_t* bvri_insert_new_celframe(bvr_animation_t* anim, bvr_celframe_t* head, float time){
    BVR_ASSERT(anim);
    
    // if there is no celframe
    if(!head){
        return NULL;
    }

    bvr_celframe_t* celframe = NULL;

    bvr_celframe_t* next_cel = head;
    bvr_celframe_t* prev_cel = NULL;

    while (next_cel && next_cel->time <= time)
    {
        prev_cel = next_cel;
        next_cel = next_cel->next;
    }
    
    if(next_cel){
        // save element indices --> might use offset more than raw pointer when finding it?
        uint64 prev_index = ((char*)prev_cel - (char*)anim->celframes.data) / anim->celframes.elemsize;
        uint64 next_index = ((char*)next_cel - (char*)anim->celframes.data) / anim->celframes.elemsize;

        BVR_BUFFER_CONST_REALLOC(anim->celframes, anim->celframes.size + anim->celframes.elemsize);
        BVR_ASSERT(anim->celframes.data);

        // update after realloc
        anim->next_frame = (bvr_celframe_t*)anim->celframes.data;

        prev_cel = &((bvr_celframe_t*)anim->celframes.data)[prev_index];
        next_cel = &((bvr_celframe_t*)anim->celframes.data)[next_index];
        celframe = &((bvr_celframe_t*)anim->celframes.data)[BVR_BUFFER_COUNT(anim->celframes) - 1];
        BVR_ASSERT(celframe);

        bvri_create_celframe(anim, celframe, time);
        prev_cel->next = celframe;
        celframe->next = next_cel;
    }

    return celframe;
}

static void bvri_do_celframe(bvr_animation_t* anim, bvr_celframe_t* frame){
    BVR_ASSERT(frame);

    for (size_t i = 0; i < frame->keyframes_count; i++)
    {
        if(!frame->keyframes[i].target){
            continue;
        }

        if(frame->keyframes[i].target->interop_func){
            frame->keyframes[i].target->interop_func(
                &frame->keyframes[i],
                &(MAX(frame->next, (bvr_celframe_t*)anim->celframes.data))->keyframes[i],
                0.5f
            );  
        }
        else {
            memcpy(
                frame->keyframes[i].target->object.ptr, 
                frame->keyframes[i].buffer,
                bvr_sizeof(frame->keyframes[i].target->type)
            );
        }
    }
}

static void bvri_destroy_celframe(bvr_celframe_t* frame){
    BVR_ASSERT(frame);

    free(frame->keyframes);
    frame->keyframes = NULL;
    frame->keyframes_count = 0;
    frame->time = -1.0f;
}