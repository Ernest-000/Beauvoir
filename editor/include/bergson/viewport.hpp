#pragma once

#include <bvr/config.h>
#include <bvr/common.h>

#include <bvr/window.h>
#include <bvr/book.h>
#include <bvr/graphics.h>

struct bgs_viewport_s {
    void* parent;
    void* context;
    void* widget;

    // keyboard controller
    void* kcontroller;

    // motion controller
    void* mcontroller;

    // scroll controller
    void* scontroller;

    // game context
    bvr_book_t book;
    bvr_page_t* page;

    struct bvr_actor_s* selected_actor;

    struct {
        vec2 mouse;
        vec2 motion;
        float scroll;
    } inputs;
};

void* bgs_create_viewport(struct bgs_viewport_s* viewport, void* parent);

int bgs_viewport_load_page(struct bgs_viewport_s* viewport, const char* path);

void bgs_viewport_get_size(struct bgs_viewport_s* viewport, uint32* x, uint32* y);

void bgs_viewport_select_actor(struct bgs_viewport_s* viewport, struct bvr_actor_s* actor);

void bgs_destroy_viewport(struct bgs_viewport_s* viewport);