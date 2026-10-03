#pragma once

#include <bvr/common.h>
#include <bvr/config.h>

#include <bvr/collections/string.h>

#include <bergson/config.h>
#include <bergson/viewport.h>
#include <bergson/menu.h>

struct bgs_window_s;

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
        struct bgs_menu_s menu;
    } components;

    struct {
        void (*on_create)(struct bgs_window_s* self);
        void (*on_tick)(struct bgs_window_s* self, float delta);
        void (*on_destroy)(struct bgs_window_s* self);
    } events;
} bgs_window_t;

int bgs_create_window(bgs_window_t* window, const char* name, const uint16 width, const uint16 height, int flags);
int bgs_destroy_window(bgs_window_t* window);