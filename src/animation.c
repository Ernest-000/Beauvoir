#include <bvr/animation.h>

#include <bvr/book.h>

#include <memory.h>
#include <malloc.h>

#define BVR_DEFAULT_FRAME_COUNT 2

#define BVR_ANIMATION_FINISH_TRESHHOLD 0.1f

/**
 * Initialize a celframe with default values
 * If frame is NULL, it will allocate a new one!
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

/**
 * harch transition between two states.
 */
static void bvri_animation_nearest(struct bvr_keyframe_s* start, struct bvr_keyframe_s* end, float _){
    memcpy(start->target->object.ptr, start->buffer, bvr_sizeof(start->target->type));        
}

/**
 * Linear interpolation between two keyframes
 */
static void bvri_animation_lerp(struct bvr_keyframe_s* start, struct bvr_keyframe_s* end, float cursor){
    float weight = invert_flerp(end->time, start->time, cursor);

    switch (start->target->type)
    {
    case BVR_FLOAT:
        {
            float lerp = 0.0f;
            
            // interpolate with the end position
            lerp = BVR_LINEAR_INTERPOLATE(*(float*)start->buffer, *(float*)end->buffer, weight);
            
            // copy the value
            memcpy(start->target->object.ptr, &lerp, sizeof(float));        
        }    
    
        break;

    case BVR_INT32:
        {
            int lerp = 0;
            
            // interpolate with the end position
            lerp = BVR_LINEAR_INTERPOLATE(*(int*)start->buffer, *(int*)end->buffer, weight);
            
            // copy the value
            memcpy(start->target->object.ptr, &lerp, sizeof(int));      
            break;
        }

    case BVR_VEC2:
        {
            vec2 lerp;
            BVR_CREATE_VEC2(lerp, 0.0f, 0.0f);

            // interpolate with the end position
            lerp[0] = BVR_LINEAR_INTERPOLATE(((float*)start->buffer)[0], ((float*)end->buffer)[0], weight);
            lerp[1] = BVR_LINEAR_INTERPOLATE(((float*)start->buffer)[1], ((float*)end->buffer)[1], weight);
            
            // copy the value
            memcpy(start->target->object.ptr, &lerp, sizeof(vec2));        
        }    
    
        break;

    case BVR_VEC3:
        {
            vec3 lerp;
            BVR_CREATE_VEC3(lerp, 0.0f, 0.0f, 0.0f);

            // interpolate with the end position
            lerp[0] = BVR_LINEAR_INTERPOLATE(((float*)start->buffer)[0], ((float*)end->buffer)[0], weight);
            lerp[1] = BVR_LINEAR_INTERPOLATE(((float*)start->buffer)[1], ((float*)end->buffer)[1], weight);
            lerp[2] = BVR_LINEAR_INTERPOLATE(((float*)start->buffer)[2], ((float*)end->buffer)[2], weight);
            
            // copy the value
            memcpy(start->target->object.ptr, &lerp, sizeof(vec3));        
        }    
    
        break;

    case BVR_VEC4:
        {
            vec4 lerp;
            BVR_CREATE_VEC4(lerp, 0.0f, 0.0f, 0.0f, 0.0f);

            // interpolate with the end position
            lerp[0] = BVR_LINEAR_INTERPOLATE(((float*)start->buffer)[0], ((float*)end->buffer)[0], weight);
            lerp[1] = BVR_LINEAR_INTERPOLATE(((float*)start->buffer)[1], ((float*)end->buffer)[1], weight);
            lerp[2] = BVR_LINEAR_INTERPOLATE(((float*)start->buffer)[2], ((float*)end->buffer)[2], weight);
            lerp[3] = BVR_LINEAR_INTERPOLATE(((float*)start->buffer)[3], ((float*)end->buffer)[3], weight);
            
            // copy the value
            memcpy(start->target->object.ptr, &lerp, sizeof(vec4));        
        }    
    
        break;
    
    default:
        // when we cannot interplate between two numbers, we just copy raw data 
        // just like BVR_ANIMATION_LINEAR_NEAREST
        memcpy(start->target->object.ptr, start->buffer, bvr_sizeof(start->target->type));        
        break;
    }
}

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
    anim->celframes.size = sizeof(bvr_celframe_t);
    anim->celframes.data = malloc(anim->celframes.size);
    BVR_ASSERT(anim->celframes.data);

    bvr_create_pool(&anim->tracks, sizeof(struct bvr_animation_handle_s), max_target);

    // create the first and the last celframe of the animation
    // theses will set anim's bounds
    {
        // create default frames
        bvr_celframe_t* start = bvri_create_celframe(anim, (bvr_celframe_t*)anim->celframes.data, anim->start);
        bvr_celframe_t* end = bvri_create_celframe(anim, NULL, anim->end);
        
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
    //uint64 elapsed_time = BVR_INSTANCE()->timer.current_time - BVR_INSTANCE()->timer.prev_time;
    uint64 elapsed_time = BVR_INSTANCE()->window.timer.current_time - BVR_INSTANCE()->window.timer.previous_time;
    //BVR_ASSERT(elapsed_time == 0.0f && "elasped time!!");

    anim->cursor += delta_time;
    anim->current_frame += elapsed_time;

    bvri_do_celframe(anim, anim->next_frame);

    // activate frame
    // if has an available frame after && can go to the next one
    if(anim->next_frame->next && anim->cursor >= anim->next_frame->time){
        anim->next_frame = anim->next_frame->next;
    }

    // when animation is finished
    if(anim->cursor >= MIN(anim->end, anim->duration)){
        anim->cursor = anim->duration - 0.5f;
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
    if(!anim->tracks.next_free){
        BVR_PRINT("cannot add more track...");
        return NULL;
    }

    bvr_animation_handle_t* handle = (bvr_animation_handle_t*) bvr_pool_alloc(&anim->tracks);
    BVR_ASSERT(handle);

    handle->id = anim->tracks.capacity - 1;
    handle->object.ptr = object;
    handle->type = type;
    handle->flags = flags;
    handle->interop_func = NULL;

    // without animation smoothing
    if(BVR_HAS_FLAG(flags, BVR_ANIMATION_LINEAR_NEAREST)){
        handle->interop_func = bvri_animation_nearest;
    }

    // default linear interpolation between two frames
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
        cel->keyframes[handle->id].time = time;
        
        memcpy(cel->keyframes[handle->id].buffer, value, bvr_sizeof(handle->type));
    }
}

void bvr_destroy_animation(bvr_animation_t* anim){
    BVR_ASSERT(anim);

    for (size_t i = 0; i < BVR_BUFFER_COUNT(anim->celframes); i++)
    {
        bvri_destroy_celframe(&((bvr_celframe_t*)anim->celframes.data)[i]);
    }
    
    bvr_animation_handle_t* handle;
    BVR_POOL_FOR_EACH(anim->tracks, handle){
        bvr_destroy_string(&handle->name);
    }

    bvr_destroy_pool(&anim->tracks);

    free(anim->celframes.data);
    anim->celframes.data = NULL;
}

static bvr_celframe_t* bvri_create_celframe(bvr_animation_t* anim, bvr_celframe_t* frame, float time){
    BVR_ASSERT(anim);
    BVR_ASSERT(time >= 0.0f);
    
    if(!frame){
        frame = (bvr_celframe_t*)malloc(sizeof(bvr_celframe_t));
    }
    
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

    bvr_celframe_t* next = head;
    bvr_celframe_t* prev = NULL;

    while (next && next->time <= time)
    {
        prev = next;
        next = next->next;
    }
    
    if(next){
        celframe = bvri_create_celframe(anim, NULL, time);

        BVR_ASSERT(celframe);
        BVR_ASSERT(prev);
        BVR_ASSERT(next);

        // check if theses nodes are linked
        // if they're not it means that we are trying to insert the node
        // in the wrong place :<
        BVR_ASSERT(prev->next == next);

        prev->next = celframe;
        celframe->next = next;
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

        // try to use the blending callback function
        if(frame->keyframes[i].target->interop_func){
            frame->keyframes[i].target->interop_func(
                &frame->keyframes[i],
                &(MAX(frame->next, (bvr_celframe_t*)anim->celframes.data))->keyframes[i],
                anim->cursor
            );  
        }
        else {
            // otherwise, we use nearest as the default one
            bvri_animation_nearest(
                &frame->keyframes[i], 
                &(MAX(frame->next, (bvr_celframe_t*)anim->celframes.data))->keyframes[i],
                0.0f
            );
        }
    }
}

static void bvri_destroy_celframe(bvr_celframe_t* frame){
    BVR_ASSERT(frame);
    
    frame->time = -1.0f;
    frame->keyframes_count = 0;
    
    free(frame->keyframes);
    frame->keyframes = NULL;

    if(frame->next){
        free(frame->next);
        frame->next = NULL;
    }
}

void bvr_create_behaviour_tree(bvr_behaviour_tree_t* machine, const uint16 default_state){
    BVR_ASSERT(machine);
    BVR_ASSERT(default_state < BVR_MAX_STATE_COUNT);

    machine->state_count = 0;
    machine->state = &machine->states[default_state];
    machine->default_state = &machine->states[default_state];

    for (size_t i = 0; i < BVR_MAX_STATE_COUNT; i++)
    {
        machine->states[i].name.length = 0;
        machine->states[i].name.string = NULL;
        
        for (size_t y = 0; y < BVR_MAX_STATE_LINK_COUNT; y++)
        {
            machine->states[i].childs[y] = NULL;
        }
        
        machine->states[i].animation = NULL;
        machine->states[i].child_count = 0;
        machine->states[i].next = 0;
    }
}

void bvr_behaviour_tree_update(bvr_behaviour_tree_t* machine, float delta_time){
    BVR_ASSERT(machine);

    // if animation will be finished
    if(BVR_ANIMATION_TIME_LEFT(*machine->state->animation) <= BVR_ANIMATION_FINISH_TRESHHOLD){
        if(machine->state->flag == BVR_BEHAVIOUR_TREE_RETURNS){
            machine->state->next = BVR_BEHAVIOUR_TREE_STATE_RETURN;
        }
    }

    if(machine->state->next){
        uint8 next = 0;
        switch (machine->state->next)
        {
        // when nothing happens
        case BVR_BEHAVIOUR_TREE_STATE_IDLE:
            /* no-op */
            break;

        // when the next state is to return to the previous one
        case BVR_BEHAVIOUR_TREE_STATE_RETURN:
            machine->state = machine->default_state;
            machine->state->next = BVR_BEHAVIOUR_TREE_STATE_IDLE;
            bvr_animation_restart(machine->state->animation);

            break;
        
        // when the next state is to return to the default state
        case BVR_BEHAVIOUR_TREE_STATE_DEFAULT:
            machine->state = machine->default_state;
            machine->state->next = BVR_BEHAVIOUR_TREE_STATE_IDLE;
            bvr_animation_restart(machine->state->animation);

            break;

        // otherwise, it means that we try to go to a children
        default:
            next = machine->state->next - BVR_BEHAVIOUR_TREE_STATE_GOTO_0;

            // if has children
            if(machine->state->child_count){
                machine->state = machine->state->childs[MIN(next, machine->state->child_count)];
            }

            machine->state->next = BVR_BEHAVIOUR_TREE_STATE_IDLE;
            bvr_animation_restart(machine->state->animation);

            break;
        }
    }

    bvr_animation_update(machine->state->animation, delta_time);
}

struct bvr_behaviour_tree_state_s* bvr_behaviour_tree_add_state_raw(bvr_behaviour_tree_t* machine, 
    struct bvr_behaviour_tree_state_s* const parent, struct bvr_behaviour_tree_state_s* state, 
    const char* name, bvr_animation_t* anim, enum bvr_behaviour_tree_flags_e flag){

    BVR_ASSERT(machine);
    
    // try to seek an existing state with the same name
    if(!state){
        state = bvr_behaviour_tree_get_state(machine, name);
    }
    
    // if there is no existing state we create it!
    if(!state){
        state = &machine->states[machine->state_count++];
        
        state->child_count = 0;
        state->next = 0;

        bvr_overwrite_string(&state->name, name, strlen(name));
    }

    if(parent){
        parent->childs[parent->child_count++] = state;
    }
    
    state->animation = anim;
    state->flag = flag;
}

void bvr_behaviour_tree_change_state_raw(bvr_behaviour_tree_t* tree, 
    struct bvr_behaviour_tree_state_s* state, enum bvr_behaviour_tree_state_e value){

    BVR_ASSERT(tree);

    if(!state){
        BVR_PRINT("invalid behaviour tree :/");
        return;
    }

    state->next = value;
}

void bvr_destroy_behaviour_tree(bvr_behaviour_tree_t* machine){
    BVR_ASSERT(machine);

    machine->default_state = NULL;
    machine->state = NULL;

    for (size_t i = 0; i < machine->state_count; i++)
    {
        for (size_t y = 0; y < BVR_MAX_STATE_LINK_COUNT; y++)
        {
            machine->states[i].childs[y] = NULL;
        }
        
        machine->states[i].animation = NULL;
        machine->states[i].child_count = 0;
        machine->states[i].next = 0;
        
        bvr_destroy_string(&machine->states[i].name);
    }
}