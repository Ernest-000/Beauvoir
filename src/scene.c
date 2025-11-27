#include <BVR/scene.h>
#include <BVR/math.h>

#include <BVR/lights.h>
#include <BVR/assets.book.h>

#include <string.h>
#include <memory.h>

#include <malloc.h>

#define BVR_SCENE_PADDING (sizeof(bvr_layer_actor_t) + sizeof(int))

static bvr_book_t *__book_instance = NULL;

static size_t bvri_actor_size(bvr_actor_type_t type);

int bvr_create_book(bvr_book_t *book)
{
    BVR_ASSERT(book);
    BVR_ASSERT(!__book_instance);

    __book_instance = book;

    memset(&book->audio, 0, sizeof(bvr_audio_stream_t));
    memset(&book->window, 0, sizeof(bvr_window_t));
    memset(&book->page, 0, sizeof(bvr_page_t));

    book->timer.frames = 0;
    book->timer.frame_timer = 0.0f;
    book->timer.delta_time = 0.0f;
    book->timer.prev_time = 0.0f;
    book->timer.current_time = 0.0f;
    book->timer.average_render_time = 0.0f;

    book->pipeline.rendering_pass.blending = BVR_BLEND_FUNC_ALPHA_ONE_MINUS;
    book->pipeline.rendering_pass.depth = BVR_DEPTH_FUNC_LESS;
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
    bvr_create_memstream(
        &book->garbage_stream,
        (BVR_MAX_SCENE_ACTOR_COUNT + BVR_MAX_SCENE_LIGHT_COUNT) * BVR_SCENE_PADDING
    );

    return BVR_OK;
}

bvr_book_t *bvr_get_book_instance()
{
    return __book_instance;
}

void bvr_new_frame(bvr_book_t *book)
{
    bvr_window_poll_events();

    book->timer.current_time = bvr_frames();
    book->timer.delta_time = (book->timer.current_time - book->timer.prev_time) / 1000.0f;

    // reset opengl states
    bvr_framebuffer_enable(&book->window.framebuffer);
    bvr_framebuffer_clear(&book->window.framebuffer, book->pipeline.clear_color);

    bvr_pipeline_state_enable(&book->pipeline.rendering_pass);

    // reset pipeline
    book->pipeline.command_count = 0;

    /* calculate camera matrices */
    bvr_update_camera(&book->page.camera);

    if (book->page.global_illumination.buffer)
    {
        bvr_enable_uniform_buffer(book->page.global_illumination.buffer);
        bvr_uniform_buffer_set(0, sizeof(vec4), &book->page.global_illumination.light.position[0]);
        bvr_uniform_buffer_set(sizeof(vec4), sizeof(vec4), &book->page.global_illumination.light.direction[0]);
        bvr_uniform_buffer_set(sizeof(vec4) * 2, sizeof(vec3), &book->page.global_illumination.light.color[0]);
        bvr_uniform_buffer_set(sizeof(vec4) * 3 - sizeof(float), sizeof(float), &book->page.global_illumination.light.intensity);
    }
}

void bvr_update(bvr_book_t *book)
{
    bvr_collider_t *collider = NULL;
    bvr_collider_t *other = NULL;

    if (!bvr_is_active(book))
    {
        return;
    }

    BVR_POOL_FOR_EACH(collider, book->page.colliders)
    {

        if (!collider)
        {
            break;
        }

        // update collider infos
        if (collider->shape == BVR_COLLIDER_BOX)
        {
            vec2_copy(((struct bvr_bounds_s *)collider->geometry.data)->coords, collider->transform->position);
        }

        // collision are disabled
        if (!BVR_HAS_FLAG(collider->body.mode, BVR_COLLISION_ENABLE))
        {

            continue;
        }

        // if this actor is aggressive
        if (BVR_HAS_FLAG(collider->body.mode, BVR_COLLISION_AGRESSIVE))
        {

            struct bvr_collision_result_s result;

            BVR_POOL_FOR_EACH(other, book->page.colliders)
            {
                if (!other)
                {
                    break;
                }

                bvr_compare_colliders(collider, other, &result);

                if (result.collide == 1)
                {
                    bvr_invert_direction(&collider->body);
                    BVR_IDENTITY_VEC3(collider->body.direction);

                    break;
                }
            }
        }

        bvr_body_apply_motion(&collider->body, collider->transform);
    }
}

void bvr_flush(bvr_book_t *book)
{
    // draw each element of the draw command array
    qsort(
        book->pipeline.commands,
        book->pipeline.command_count,
        sizeof(struct bvr_draw_command_s),
        bvr_pipeline_compare_commands);

    for (uint64 i = 0; i < book->pipeline.command_count; i++)
    {
        bvr_pipeline_draw_cmd(&book->pipeline.commands[i]);
    }

    book->pipeline.command_count = 0;
}

void bvr_render(bvr_book_t *book)
{

    // if there is still draw commands, flush
    if (book->pipeline.command_count)
    {
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
    double delay = BVR_TARGET_FRAMERATE * 0.25 - (bvr_frames() - book->timer.current_time);
    if (delay > 0)
    {
        bvr_delay(delay);
    }
#endif

    book->timer.frames++;
    book->timer.frame_timer += book->timer.delta_time;

    // book->average_render_time = (int)(flerp((float)book->average_render_time, (float)(book->frames / book->frame_timer), 0.5f));
    book->timer.prev_time = book->timer.current_time;

    if (book->timer.frames > BVR_TARGET_FRAMERATE)
    {
        book->timer.average_render_time = book->timer.frames / book->timer.frame_timer;
        book->timer.frames = 0;
        book->timer.frame_timer = book->timer.delta_time;
    }

    bvr_poll_errors();
}

void bvr_destroy_book(bvr_book_t *book)
{
    if (book->window.context)
    {
        bvr_destroy_window(&book->window);
    }

    if (book->audio.stream)
    {
        bvr_destroy_audio_stream(&book->audio);
    }

    bvr_destroy_page(&book->page);

    bvr_destroy_memstream(&book->asset_stream);
    bvr_destroy_memstream(&book->garbage_stream);
}

int bvr_create_page(bvr_page_t *page, const char *name)
{
    BVR_ASSERT(page);

    page->camera.mode = 0;
    page->camera.buffer = 0;
    page->camera.far = 0.0f;
    page->camera.near = 0.0f;
    page->camera.framebuffer = NULL;
    page->camera.field_of_view.scale = 0;

    page->global_illumination.buffer = 0;
    page->global_illumination.light.intensity = 255.0f;
    page->global_illumination.light.type = BVR_LIGHT_GLOBAL_ILLUMINATION;
    page->global_illumination.light.position[3] = 51;
    BVR_SCALE_VEC3(page->global_illumination.light.color, 1.0f);
    BVR_SCALE_VEC3(page->global_illumination.light.direction, 0);
    BVR_SCALE_VEC3(page->global_illumination.light.position, 0);

    bvr_create_string(&page->name, name);

    bvr_create_pool(&page->actors, sizeof(struct bvr_actor_s *), BVR_MAX_SCENE_ACTOR_COUNT);
    bvr_create_pool(&page->colliders, sizeof(bvr_collider_t *), BVR_COLLIDER_COLLECTION_SIZE);
    bvr_create_pool(&page->lights, sizeof(struct bvr_light_s *), BVR_MAX_SCENE_LIGHT_COUNT);

    // create global lighting
    bvr_global_illumination_t **gl = (bvr_global_illumination_t **)bvr_pool_alloc(&page->lights);
    *gl = &page->global_illumination;

    return BVR_OK;
}

void bvr_enable_page(bvr_page_t *page)
{
    if (page == &__book_instance->page)
    {
        return;
    }

    if (bvr_is_active(__book_instance))
    {
        bvr_disable_page(&__book_instance->page);
    }

    memcpy(&__book_instance->page, page, sizeof(bvr_page_t));

#ifdef BVR_AUTO_SAVE
    // load page's datas
    bvr_asset_t asset;
    if (bvr_find_asset(BVR_FORMAT("%s.bin", __book_instance->page.name.string), &asset))
    {
        bvr_open_book(BVR_FORMAT("%s.bin", __book_instance->page.name.string), bvr_get_book_instance());
    }
#endif
}

void bvr_disable_page(bvr_page_t *page)
{
#ifdef BVR_AUTO_SAVE
    // sage page's data
    if (access(BVR_FORMAT("%s.bin", __book_instance->page.name.string), F_OK))
    {
        bvr_write_book(BVR_FORMAT("%s.bin", __book_instance->page.name.string), bvr_get_book_instance());
    }
#endif

    bvr_memstream_clear(&__book_instance->garbage_stream);
    bvr_memstream_clear(&__book_instance->asset_stream);

    bvr_destroy_page(&__book_instance->page);
}

struct bvr_actor_s *bvr_alloc_actor(bvr_page_t *page, bvr_actor_type_t type)
{
    BVR_ASSERT(page);

    struct bvr_actor_s** pp_actor;
    const size_t actor_byte_size = bvri_actor_size(type);

    // check if this actor can be added
    if(actor_byte_size <= 0 || actor_byte_size >= BVR_SCENE_PADDING){
        BVR_PRINT("failed to allocate a new actor :<");
        return NULL;
    }

    // get actor's pool pointer
    pp_actor = (struct bvr_actor_s **)bvr_pool_alloc(&page->actors);

    // define actor's pointer as current memory stream cursor
    *pp_actor = (struct bvr_actor_s *)__book_instance->garbage_stream.cursor;

    // everything heaped is padded to a certain constant value
    memset(*pp_actor, 0, BVR_SCENE_PADDING);

    bvr_memstream_write(
        &__book_instance->garbage_stream,
        NULL,
        BVR_SCENE_PADDING
    );

    BVR_PRINTF("alloacted a new actor (%x) remains %i bytes", 
        *pp_actor, __book_instance->garbage_stream.size - (size_t)(__book_instance->garbage_stream.cursor - (char*)__book_instance->garbage_stream.data)
    );

    // types that have colliders
    if(type == BVR_DYNAMIC_ACTOR || type == BVR_TEXTURE_ACTOR){
        bvr_register_collider(page, &((bvr_dynamic_actor_t*)*pp_actor)->collider);
    }   

    return *pp_actor;
}

void bvr_free_actor(bvr_page_t* page, struct bvr_actor_s* actor){
    BVR_ASSERT(page);
    
    if(actor){
        bvr_destroy_actor(actor);
        bvr_pool_free(&page->actors, actor);

        __book_instance->garbage_stream.cursor = (char*)actor;
    }
}

bvr_collider_t *bvr_register_collider(bvr_page_t *page, bvr_collider_t *collider)
{
    BVR_ASSERT(page);

    if (collider)
    {
        bvr_collider_t **cptr = (bvr_collider_t **)bvr_pool_alloc(&page->colliders);
        *cptr = collider;

        BVR_PRINTF("linked collider %x to the page!", *cptr);
        return *cptr;
    }

    return NULL;
}

struct bvr_actor_s *bvr_find_actor(bvr_book_t *book, const char *name)
{
    BVR_ASSERT(book);
    BVR_ASSERT(name);

    struct bvr_actor_s *actor;
    BVR_POOL_FOR_EACH(actor, book->page.actors)
    {
        if (actor == NULL)
        {
            break;
        }

        if (strncmp(actor->name.string, name, actor->name.length) == 0)
        {
            return actor;
        }
    }

    return NULL;
}

struct bvr_actor_s *bvr_find_actor_uuid(bvr_book_t *book, bvr_uuid_t uuid)
{
    BVR_ASSERT(book);
    BVR_ASSERT(uuid);

    struct bvr_actor_s *actor;
    BVR_POOL_FOR_EACH(actor, book->page.actors)
    {
        if (actor == NULL)
        {
            break;
        }

        if (bvr_uuid_equals(actor->id, uuid))
        {
            return actor;
        }
    }

    return NULL;
}

void bvr_destroy_page(bvr_page_t *page)
{
    BVR_ASSERT(page);

    int i = 0;
    struct bvr_actor_s *actor = NULL;
    BVR_POOL_FOR_EACH(actor, page->actors)
    {
        if (!actor)
            break;

        // destroy actor
        bvr_destroy_actor(actor);
    }

    bvr_collider_t *collider = NULL;
    BVR_POOL_FOR_EACH(collider, page->colliders)
    {
        if (!collider)
            break;

        // remove collider
        collider = NULL;
    }

    struct bvr_light_s *light = NULL;
    BVR_POOL_FOR_EACH(light, page->lights)
    {
        if (!light)
            break;

        // destroy light
    }

    bvr_destroy_uniform_buffer(&page->camera.buffer);
    bvr_destroy_uniform_buffer(&page->global_illumination.buffer);

    bvr_destroy_string(&page->name);

    bvr_destroy_pool(&page->actors);
    bvr_destroy_pool(&page->colliders);
    bvr_destroy_pool(&page->lights);
}

static size_t bvri_actor_size(bvr_actor_type_t type)
{
    switch (type)
    {
    case BVR_NULL_ACTOR:
        return 0;
    case BVR_EMPTY_ACTOR:
        return sizeof(bvr_empty_actor_t);
    case BVR_LAYER_ACTOR:
        return sizeof(bvr_layer_actor_t);
    case BVR_TEXTURE_ACTOR:
        return sizeof(bvr_texture_actor_t);
    case BVR_STATIC_ACTOR:
        return sizeof(bvr_static_actor_t);
    case BVR_DYNAMIC_ACTOR:
        return sizeof(bvr_dynamic_actor_t);
    case BVR_LANDSCAPE_ACTOR:
        return sizeof(bvr_landscape_actor_t);
    default:
        return 0;
    }

    return 0;
}