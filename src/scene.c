#include <BVR/scene.h>
#include <BVR/math.h>

#include <BVR/lights.h>
#include <BVR/assets.book.h>

#include <string.h>
#include <memory.h>

#include <malloc.h>

static bvr_book_t* __book_instance = NULL;

int bvr_create_book(bvr_book_t* book){
    BVR_ASSERT(book);
    BVR_ASSERT(!__book_instance);

    __book_instance = book;

    memset(&book->audio, 0, sizeof(bvr_audio_stream_t));
    memset(&book->window, 0, sizeof(bvr_window_t));
    memset(&book->page, 0, sizeof(bvr_page_t));

    book->frames = 0;
    book->frame_timer = 0.0f;
    book->delta_time = 0.0f;
    book->prev_time = 0.0f;
    book->current_time = 0.0f;
    book->average_render_time = 0.0f;

    book->pipeline.rendering_pass.blending = BVR_BLEND_FUNC_ALPHA_ONE_MINUS;
    book->pipeline.rendering_pass.depth = BVR_DEPTH_TEST_ENABLE;
    book->pipeline.rendering_pass.depth |= BVR_DEPTH_FUNC_LESS;
    book->pipeline.rendering_pass.flags = 0;

    book->pipeline.swap_pass.blending = BVR_BLEND_DISABLE;
    book->pipeline.swap_pass.depth = BVR_DEPTH_TEST_DISABLE;
    book->pipeline.swap_pass.flags = 0;

    book->pipeline.clear_color[0] = 0.0f;
    book->pipeline.clear_color[1] = 0.0f;
    book->pipeline.clear_color[2] = 0.0f;

    book->pipeline.command_count = 0;
    memset(&book->pipeline.commands, 0, sizeof(book->pipeline.commands));

    bvr_create_memstream(&book->asset_stream, 0);

#ifdef BVR_SCENE_AUTO_HEAP
    bvr_create_memstream(
        &book->garbage_stream, 
        BVR_MAX_SCENE_ACTOR_COUNT * sizeof(bvr_dynamic_actor_t) +
        BVR_MAX_SCENE_LIGHT_COUNT * sizeof(struct bvr_light_s)
    );
#else
    bvr_create_memstream(&book->garbage_stream, 0);
#endif
    return BVR_OK;
}

bvr_book_t* bvr_get_book_instance(){
    return __book_instance;
}

void bvr_new_frame(bvr_book_t* book){
    bvr_window_poll_events();

    book->current_time = bvr_frames();
    book->delta_time = (book->current_time - book->prev_time) / 1000.0f;

    // reset opengl states
    bvr_framebuffer_enable(&book->window.framebuffer);
    bvr_framebuffer_clear(&book->window.framebuffer, book->pipeline.clear_color);

    bvr_pipeline_state_enable(&book->pipeline.rendering_pass);

    // reset pipeline
    book->pipeline.command_count = 0;

    /* calculate camera matrices */
    mat4x4 projection, view;
    bvr_camera_t* camera = &book->page.camera;
    BVR_IDENTITY_MAT4(projection);
    BVR_IDENTITY_MAT4(view);

    if(camera->mode == BVR_CAMERA_ORTHOGRAPHIC){
        float width = 1.0f / camera->framebuffer->target_width * camera->field_of_view.scale;
        float height = 1.0f / camera->framebuffer->target_height * camera->field_of_view.scale;
        float farnear = 1.0f / (camera->far - camera->near);

        projection[0][0] = width;
        projection[1][1] = height;
        projection[2][2] = farnear;
        projection[3][0] = -width;
        projection[3][1] = -height;
        projection[3][2] = -camera->near * farnear;
        projection[3][3] =  1.0f;
    }

    view[3][0] = camera->transform.position[0];
    view[3][1] = camera->transform.position[1];
    view[3][2] = camera->transform.position[2];

    bvr_enable_uniform_buffer(book->page.camera.buffer);
    bvr_uniform_buffer_set(0, sizeof(mat4x4), &projection[0][0]);
    bvr_uniform_buffer_set(sizeof(mat4x4), sizeof(mat4x4), &view[0][0]);
    bvr_enable_uniform_buffer(0);
}

void bvr_update(bvr_book_t* book){
    bvr_collider_t* collider = NULL;
    bvr_collider_t* other = NULL;

    if(!bvr_is_active()){
        return;
    }

    BVR_POOL_FOR_EACH(collider, book->page.colliders){        

        if(!collider){
            break;
        }

        // update collider infos
        if(collider->shape == BVR_COLLIDER_BOX){
            vec2_copy(((struct bvr_bounds_s*)collider->geometry.data)->coords, collider->transform->position);
        }

        // collision are disabled
        if(!BVR_HAS_FLAG(collider->body.mode, BVR_COLLISION_ENABLE)){
            
            continue;
        }

        // if this actor is aggressive
        if(BVR_HAS_FLAG(collider->body.mode, BVR_COLLISION_AGRESSIVE)){

            struct bvr_collision_result_s result;

            BVR_POOL_FOR_EACH(other, book->page.colliders){
                if(!other){
                    break;
                }

                bvr_compare_colliders(collider, other, &result);

                if(result.collide == 1){
                    //bvr_invert_direction(&collider->body);
                    //BVR_IDENTITY_VEC3(collider->body.direction);

                    break;
                }
            }

        }

        bvr_body_apply_motion(&collider->body, collider->transform);
    }
}

void bvr_flush(bvr_book_t* book){
    // draw each element of the draw command array
    qsort(
        book->pipeline.commands, 
        book->pipeline.command_count, 
        sizeof(struct bvr_draw_command_s), 
        bvr_pipeline_compare_commands
    );
    
    for (uint64 i = 0; i < book->pipeline.command_count; i++)
    {
        bvr_pipeline_draw_cmd(&book->pipeline.commands[i]);
    }

    book->pipeline.command_count = 0;
}

void bvr_render(bvr_book_t* book){
    
    // if there is still draw commands, flush
    if(book->pipeline.command_count){
        bvr_flush(book);
    }

    // disable the rendering framebuffer
    bvr_framebuffer_disable(&book->window.framebuffer);

    // swap pass
    bvr_pipeline_state_enable(&book->pipeline.swap_pass);
    bvr_framebuffer_clear(NULL, book->pipeline.clear_color);

    // push rendered scene to the screen
    bvr_framebuffer_blit(&book->window.framebuffer);
    bvr_window_push_buffers();

#ifndef BVR_NO_FPS_CAP
    // wait for next frame. 
    if(book->prev_time + BVR_FRAMERATE > book->current_time){
        bvr_delay(book->current_time - book->prev_time + BVR_FRAMERATE);
    }
#endif

    book->frames++;
    book->frame_timer += book->delta_time;

    //book->average_render_time = (int)(flerp((float)book->average_render_time, (float)(book->frames / book->frame_timer), 0.5f));
    book->prev_time = book->current_time;

    if(book->frames > BVR_TARGET_FRAMERATE){
        book->average_render_time = book->frames / book->frame_timer;
        book->frames = 0;
        book->frame_timer = book->delta_time;
    }

    bvr_error();
}

void bvr_destroy_book(bvr_book_t* book){
    if(book->window.context){
        bvr_destroy_window(&book->window);
    }

    if(book->audio.stream){
        bvr_destroy_audio_stream(&book->audio);
    }

    bvr_destroy_page(&book->page);

    bvr_destroy_memstream(&book->asset_stream);
    bvr_destroy_memstream(&book->garbage_stream);    
}

int bvr_create_page(bvr_page_t* page, const char* name){
    BVR_ASSERT(page);

    bvr_create_string(&page->name, name);

    bvr_create_pool(&page->actors, sizeof(struct bvr_actor_s*), BVR_MAX_SCENE_ACTOR_COUNT);
    bvr_create_pool(&page->colliders, sizeof(bvr_collider_t*), BVR_COLLIDER_COLLECTION_SIZE);
    bvr_create_pool(&page->lights, sizeof(struct bvr_light_s*), BVR_MAX_SCENE_LIGHT_COUNT);

    return BVR_OK;
}

void bvr_enable_page(){
    
#ifdef BVR_AUTO_SAVE

    bvr_asset_t asset;
    if(bvr_find_asset(BVR_FORMAT("%s.bin", __book_instance->page.name.string), &asset)){
        bvr_open_book(BVR_FORMAT("%s.bin", __book_instance->page.name.string), bvr_get_book_instance());
    }

#endif


}

void bvr_disable_page(){
#ifdef BVR_AUTO_SAVE

    if(access(BVR_FORMAT("%s.bin", __book_instance->page.name.string), F_OK)){
        bvr_write_book(BVR_FORMAT("%s.bin", __book_instance->page.name.string), bvr_get_book_instance());
    }

#endif

    bvr_memstream_clear(&__book_instance->garbage_stream);
    bvr_memstream_clear(&__book_instance->asset_stream);

    bvr_destroy_page(&__book_instance->page);
}

bvr_camera_t* bvr_create_orthographic_camera(bvr_page_t* page, bvr_framebuffer_t* framebuffer, float near, float far, float scale){
    BVR_ASSERT(page);

    page->camera.mode = BVR_CAMERA_ORTHOGRAPHIC;
    page->camera.framebuffer = framebuffer;
    page->camera.near = near;
    page->camera.far = far;
    page->camera.field_of_view.scale = scale;
    
    BVR_IDENTITY_VEC3(page->camera.transform.position);
    BVR_IDENTITY_VEC4(page->camera.transform.rotation);
    BVR_IDENTITY_VEC3(page->camera.transform.scale);
    BVR_IDENTITY_MAT4(page->camera.transform.matrix);

    bvr_create_uniform_buffer(&page->camera.buffer, 2 * sizeof(mat4x4));

    return &page->camera;
}

void bvr_camera_lookat(bvr_page_t* page, vec3 target, vec3 y){
    mat4x4 view;
    BVR_IDENTITY_MAT4(view);
    
    vec3 fwd, side, up;
    vec3_sub(fwd, target, page->camera.transform.position);
    vec3_norm(fwd, fwd);
    
    vec3_mul_cross(side, fwd, y);
    
    vec3_mul_cross(up, side, fwd);
    vec3_norm(up, up);

    view[0][0] = side[0];
    view[1][0] = side[1];
    view[2][0] = side[2];
    view[3][0] = -vec3_dot(side, page->camera.transform.position);
    view[0][1] = up[0];
    view[1][1] = up[1];
    view[2][1] = up[2];
    view[3][1] = -vec3_dot(up, page->camera.transform.position);
    view[0][2] = -fwd[0];
    view[1][2] = -fwd[1];
    view[2][2] = -fwd[2];
    view[3][2] = -vec3_dot(fwd, page->camera.transform.position);
    
    bvr_camera_set_view(page, view);
}

void bvr_screen_to_world_coords(bvr_book_t* book, vec2 screen_coords, vec3 world_coords){
    BVR_ASSERT(book);

    if(!vec2_dot(screen_coords, screen_coords)){
        return;
    }

    vec4 world, screen;
    mat4x4 projection, view, inv;
    
    bvr_enable_uniform_buffer(book->page.camera.buffer);

    mat4x4* buffer_ptr = bvr_uniform_buffer_map(0, 2 * sizeof(mat4x4));
    if(buffer_ptr){
        memcpy(projection, &buffer_ptr[0], sizeof(mat4x4));
        memcpy(view, &buffer_ptr[1], sizeof(mat4x4));

        bvr_uniform_buffer_close();
        bvr_enable_uniform_buffer(0);

        screen[0] = (screen_coords[0] / book->window.framebuffer.width - 0.5f) * 6.0f;
        screen[1] = (screen_coords[1] / book->window.framebuffer.height - 0.5f) * -6.0f;
        screen[2] = 0.0f;
        screen[3] = 1.0f;
        
        mat4_mul(projection, projection, view);
        mat4_inv(inv, projection);
        mat4_mul_vec4(world, inv, screen);

        world[3] = 1.0f / world[3];
        world_coords[0] = world[0] * world[3];
        world_coords[1] = world[1] * world[3];
        world_coords[2] = world[2] * world[3];

        return;
    }

    world_coords[0] = 0.0f;
    world_coords[1] = 0.0f;
    world_coords[2] = 0.0f;
}

struct bvr_actor_s* bvr_link_actor_to_page(bvr_page_t* page, struct bvr_actor_s* actor){
    BVR_ASSERT(page);
    
    if(actor){
        struct bvr_actor_s** aptr = (struct bvr_actor_s**) bvr_pool_alloc(&page->actors);

#ifdef BVR_SCENE_AUTO_HEAP
        *aptr = (struct bvr_actor_s*) bvr_get_book_instance()->garbage_stream.cursor;

        switch (actor->type)
        {
        case BVR_NULL_ACTOR:        
            break;
        case BVR_EMPTY_ACTOR:
            bvr_memstream_write(&bvr_get_book_instance()->garbage_stream, actor, sizeof(bvr_empty_actor_t));
            break;

        case BVR_LAYER_ACTOR:
            {
                bvr_memstream_write(&bvr_get_book_instance()->garbage_stream, actor, sizeof(bvr_layer_actor_t));
            }
            break;

        case BVR_BITMAP_ACTOR:
            {
                bvr_collider_t* collider = bvr_link_collider_to_page(page, &((bvr_bitmap_layer_t*)*aptr)->collider);
                bvr_memstream_write(&bvr_get_book_instance()->garbage_stream, actor, sizeof(bvr_bitmap_layer_t));
                collider->transform = &(*aptr)->transform;
            }
            break;

        case BVR_STATIC_ACTOR:
            bvr_memstream_write(&bvr_get_book_instance()->garbage_stream, actor, sizeof(bvr_static_actor_t));
            break;

        case BVR_DYNAMIC_ACTOR:
            {
                bvr_collider_t* collider = bvr_link_collider_to_page(page, &((bvr_dynamic_actor_t*)*aptr)->collider);
                bvr_memstream_write(&bvr_get_book_instance()->garbage_stream, actor, sizeof(bvr_dynamic_actor_t));
                collider->transform = &(*aptr)->transform;
            }
            break;
        
        default:
            break;
        }
#else
        *aptr = actor;
        switch (actor->type)
        {
        case BVR_BITMAP_ACTOR:
            bvr_link_collider_to_page(page, &((bvr_bitmap_layer_t*)actor)->collider);
        case BVR_DYNAMIC_ACTOR:
            bvr_link_collider_to_page(page, &((bvr_dynamic_actor_t*)actor)->collider);
            break;
        
        default:
            break;
        }
#endif
        BVR_PRINTF("linked %s to the page!", (*aptr)->name.string);
        return *aptr;
    }

    return NULL;
}

bvr_collider_t* bvr_link_collider_to_page(bvr_page_t* page, bvr_collider_t* collider){
    BVR_ASSERT(page);

    if(collider){
        bvr_collider_t** cptr = (bvr_collider_t**) bvr_pool_alloc(&page->colliders);
        *cptr = collider;
        BVR_PRINTF("linked collider %x to the page!", *cptr);
        return *cptr;
    }

    return NULL;
}


struct bvr_actor_s* bvr_find_actor(bvr_book_t* book, const char* name){
    BVR_ASSERT(book);
    BVR_ASSERT(name);

    struct bvr_actor_s* actor;
    BVR_POOL_FOR_EACH(actor, book->page.actors){
        if(actor == NULL){
            break;
        }

        if(strncmp(actor->name.string, name, actor->name.length) == 0){
            return actor;
        }
    }

    return NULL;
}

struct bvr_actor_s* bvr_find_actor_uuid(bvr_book_t* book, bvr_uuid_t uuid){
    BVR_ASSERT(book);
    BVR_ASSERT(uuid);

    struct bvr_actor_s* actor;
    BVR_POOL_FOR_EACH(actor, book->page.actors){
        if(actor == NULL){
            break;
        }

        if(bvr_uuid_equals(actor->id, uuid)){
            return actor;
        }
    }

    return NULL;
}

void bvr_destroy_page(bvr_page_t* page){
    BVR_ASSERT(page);

    int i = 0;
    struct bvr_actor_s* actor = NULL;
    BVR_POOL_FOR_EACH(actor, page->actors){
        if(!actor) break;
        
        // destroy actor
        bvr_destroy_actor(actor);
    }

    bvr_collider_t* collider = NULL;
    BVR_POOL_FOR_EACH(collider, page->colliders){
        if(!collider) break;

        // remove collider
        collider = NULL;
    }

    struct bvr_light_s* light = NULL;
    BVR_POOL_FOR_EACH(light, page->lights){
        if(!light) break;

        // destroy light
    }

    bvr_destroy_uniform_buffer(&page->camera.buffer);

    bvr_destroy_string(&page->name);

    bvr_destroy_pool(&page->actors);
    bvr_destroy_pool(&page->colliders);
    bvr_destroy_pool(&page->lights);
}