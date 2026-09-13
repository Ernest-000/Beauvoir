#pragma once

#include <bvr/common.h>
#include <bvr/config.h>

#include <bvr/collections/string.h>

#include <bergon/config.h>
#include <bergon/viewport.h>

typedef struct bgs_window_s {
    void* handle;
    void* window;
    void* gl;

    bvr_string_t name;
    
    uint16 width;
    uint16 height;
    int flags; 

    struct {
        struct bgs_viewport_s viewport;
    } components;
} bgs_window_t;

int bgs_create_window(bgs_window_t* window, const char* name, const uint16 width, const uint16 height, int flags);
int bgs_destroy_window(bgs_window_t* window);