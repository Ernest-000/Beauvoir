#include <bvr/book.h>
#include <bvr/io.h>
#include <bvr/math.h>
#include <bvr/actors.h>

#include <stdlib.h>

#define BVRI_DEFAULT_WINWIDTH 800
#define BVRI_DEFAULT_WINHEIGHT 800

#define BVRI_DEFAULT_STREAM_SIZE (sizeof(struct bvr_actor_s) * 32)

// binded book
static bvr_book_t* __book = NULL;

bvr_book_t* bvr_create_book(bvr_book_t* book){
    // bypass book creating if a context already exists
    if(__book){
        return __book;
    }

    BVR_ASSERT(book);

    return bvr_create_book_attributes(book, NULL);
}

bvr_book_t* bvr_create_book_attributes(bvr_book_t* book, struct bvr_book_attributes_s* _attributes){
    // bypass book creating if a context already exists
    if(__book){
        return __book;
    }

    BVR_ASSERT(book);

    struct bvr_book_attributes_s attributes;
    if(_attributes){
        // check for uninitialized attributes
        attributes.window_width = _attributes->window_width ? _attributes->window_width : BVRI_DEFAULT_WINWIDTH;
        attributes.window_height = _attributes->window_height ? _attributes->window_height : BVRI_DEFAULT_WINHEIGHT;
        attributes.window_flags = _attributes->window_flags ? _attributes->window_flags : BVR_WINDOW_DEFAULT;

        attributes.garbage_stream_size = _attributes->garbage_stream_size ? _attributes->garbage_stream_size : BVRI_DEFAULT_STREAM_SIZE;
        attributes.asset_stream_size = _attributes->asset_stream_size ? _attributes->asset_stream_size : BVRI_DEFAULT_STREAM_SIZE;
        
        attributes.sample_rate = _attributes->sample_rate ? _attributes->sample_rate : BVR_SAMPLE_RATE;
        attributes.channels = _attributes->channels && _attributes->channels < 3 ? _attributes->channels : BVR_AUDIO_STEREO;
        
        attributes.name = _attributes->name != NULL ? _attributes->name : BVR_CLASS_NAME;
    }
    else {
        // create default attributes
        attributes.window_width = BVRI_DEFAULT_WINWIDTH;
        attributes.window_height = BVRI_DEFAULT_WINHEIGHT;
        attributes.window_flags = BVR_WINDOW_DEFAULT;

        attributes.garbage_stream_size = BVRI_DEFAULT_STREAM_SIZE;
        attributes.asset_stream_size = BVRI_DEFAULT_STREAM_SIZE;

        attributes.sample_rate = BVR_SAMPLE_RATE;
        attributes.channels = BVR_AUDIO_STEREO;

        attributes.name = BVR_CLASS_NAME;
    }

    BVR_ASSERT(bvr_create_window(
        &book->window, 
        attributes.window_width, 
        attributes.window_height, 
        attributes.name, 
        attributes.window_flags
    ));

    BVR_ASSERT(bvr_create_audio_mixer(
        &book->mixer,
        attributes.sample_rate,
        attributes.channels
    ));

    bvr_create_predefs(&book->predefs);

    // setting up default pipeline values
    {
        book->graphics.rendering_pass.blending = BVR_BLEND_FUNC_ALPHA_ONE_MINUS;
        book->graphics.rendering_pass.depth = BVR_DEPTH_FUNC_LESS;
        book->graphics.rendering_pass.flags = 0;

        book->graphics.gui_pass.blending = BVR_BLEND_FUNC_ALPHA_ADD;
        book->graphics.gui_pass.depth = BVR_DEPTH_TEST_DISABLE;
        book->graphics.gui_pass.flags = BVR_SCISSORS_ENABLE;

        book->graphics.swap_pass.blending = BVR_BLEND_DISABLE;
        book->graphics.swap_pass.depth = BVR_DEPTH_TEST_DISABLE;
        book->graphics.swap_pass.flags = 0;

        BVR_IDENTITY_VEC3(book->graphics.clear_color);
        book->graphics.state.framebuffer = NULL;

        book->graphics.command_count = 0;
        memset(&book->graphics.commands, 0, sizeof(book->graphics.commands));
    }

    // init scene slots
    for (size_t i = 0; i < BVR_MAX_PAGE; i++)
    {
        book->slots[i].order = i;
        book->slots[i].is_active = false;
        book->slots[i].is_assigned = false;
    }
    book->active_slot = -1;
    
    bvr_create_memstream(&book->garbage, attributes.garbage_stream_size);
    bvr_create_memstream(&book->assets, attributes.asset_stream_size);

    // set instance
    __book = book;

    return book;
}

void bvr_new_frame(void){
    BVR_ASSERT(__book);

    bvr_window_poll_events(&__book->window);

    bvr_framebuffer_enable(&__book->window.framebuffer);
    bvr_framebuffer_clear(&__book->window.framebuffer, __book->graphics.clear_color);

    bvr_pipeline_state_enable(&__book->graphics.rendering_pass);

    __book->graphics.command_count = 0;
    
    // get the current page
    bvr_page_t* page = BVR_PAGE();

    if(page){
        // prepare opengl uniform buffers
        // update camera
        bvr_update_camera(&page->camera);
    }
    
}

void bvr_flush(void){
    BVR_ASSERT(__book);

    // if has enough commands to be sorted
    if(__book->graphics.command_count > 1){
        qsort(
            __book->graphics.commands,
            __book->graphics.command_count,
            sizeof(struct bvr_draw_command_s),
            bvr_pipeline_compare_commands
        );
    }

    for (size_t i = 0; i < __book->graphics.command_count; i++)
    {
        bvr_pipeline_do_draw_cmd(
            &__book->graphics.rendering_pass,
            &__book->graphics.commands[i]
        );
    }
    
    // clear command queue
    __book->graphics.command_count = 0;
}

void bvr_render(void){
    BVR_ASSERT(__book);

    if(__book->graphics.command_count){
        bvr_flush();
    }

    bvr_framebuffer_disable(&__book->window.framebuffer);

    // swap pass
    bvr_pipeline_state_enable(&__book->graphics.swap_pass);
    bvr_framebuffer_clear(NULL, __book->graphics.clear_color);

    bvr_framebuffer_blit(&__book->window.framebuffer);
    bvr_window_push_buffers(&__book->window);
}

void bvr_destroy_book(bvr_book_t* book){
    BVR_ASSERT(book);

    // destroying predefs should be first because it
    // rely on opengl functions 
    bvr_destroy_predefs(&book->predefs);

    // audio functions shall be destroyed before the window system
    bvr_destroy_audio_mixer(&book->mixer);

    bvr_destroy_window(&book->window);

    // free garbage streams
    bvr_destroy_memstream(&book->garbage);
    bvr_destroy_memstream(&book->assets);
}

bvr_page_t* bvr_create_page(bvr_page_t* page, const char* name){
    if(page == NULL){
        // if it's null, we return the current scene
        return &BVR_INSTANCE()->slots[clampi(BVR_INSTANCE()->active_slot, 0, BVR_MAX_PAGE - 1)].page;
    }

    bvr_create_string(&page->name, name);
    bvr_create_table(&page->actors, sizeof(struct bvr_actor_s*), 64);
    return page;
}

bvr_page_t* bvr_enable_page(uint32 index){
    if(index >= BVR_MAX_PAGE){
        return NULL;
    }

    if(index == BVR_INSTANCE()->active_slot){
        // when it's already the active scene
        return bvr_create_page(NULL, 0);
    }

    if(BVR_INSTANCE()->active_slot >= 0){
        // disable the current slot
        struct bvr_page_slot_s* prev = &BVR_INSTANCE()->slots[BVR_INSTANCE()->active_slot];
        prev->is_active = false;
        prev->is_assigned = false;
    }

    struct bvr_page_slot_s* slot = &BVR_INSTANCE()->slots[index];
    slot->is_active = true;
    slot->is_assigned = true;

    // update the current slot
    BVR_INSTANCE()->active_slot = index;

    return &slot->page;
}

struct bvr_actor_s* bvr_alloc_actor(bvr_page_t* page, const char* name, const uint32 size){
    BVR_ASSERT(page);

    if(size == 0){
        // allocation failed
        return NULL;
    }

    struct bvr_actor_s** pp_actor = bvr_table_add(&page->actors, name, 0);
    struct bvr_actor_s* p_actor = NULL;
    BVR_ASSERT(pp_actor);

    if(BVR_INSTANCE()->garbage.cursor == 0){
        // check for memstream errors
        BVR_PRINT("invalid garbage stream");
        return NULL;
    }

    p_actor = (struct bvr_actor_s*)BVR_INSTANCE()->garbage.cursor;
    BVR_ASSERT(p_actor);

    bvr_memstream_write(&BVR_INSTANCE()->garbage, NULL, size);
    
    // clear up memory
    memset(p_actor, 0, size);

    BVR_TRANSFORM(
        p_actor->transform,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    );

    p_actor->parent = NULL;
    p_actor->childs = NULL;
    p_actor->child_count = 0;
    p_actor->flags = 0;
    p_actor->hash = ((struct bvr_table_chunk_s*)(pp_actor - sizeof(struct bvr_table_chunk_s)))->key;

    bvr_create_string(&p_actor->name, name);

    return p_actor;
}

void bvr_destroy_page(bvr_page_t* page){
    BVR_ASSERT(page);
    
    bvr_destroy_string(&page->name);
    bvr_destroy_table(&page->actors);
}