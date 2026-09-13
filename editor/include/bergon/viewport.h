#pragma once

#include <bvr/config.h>
#include <bvr/common.h>

#include <bvr/window.h>
#include <bvr/book.h>
#include <bvr/graphics.h>

struct bgs_viewport_s {
    void* context;
    void* widget;

    // current graphic pipeline
    bvr_pipeline_t pipeline;

    // page
    bvr_page_t page;
};

void* bgs_create_viewport(struct bgs_viewport_s* viewport);
void bgs_destroy_viewport(struct bgs_viewport_s* viewport);