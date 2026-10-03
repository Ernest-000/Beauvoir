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

    // game context
    bvr_book_t book;
    bvr_page_t* page;
};

void* bgs_create_viewport(struct bgs_viewport_s* viewport, void* parent);
int bgs_viewport_load_page(struct bgs_viewport_s* viewport, const char* path);
void bgs_destroy_viewport(struct bgs_viewport_s* viewport);