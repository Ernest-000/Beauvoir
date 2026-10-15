#pragma once

#include <bvr/common.h>
#include <bvr/config.h>

#include <bvr/collections/string.h>

#include <bergson/config.hpp>
#include <bergson/viewport.hpp>
#include <bergson/hierarchy.hpp>
#include <bergson/inspector.hpp>
#include <bergson/menu.hpp>
#include <bergson/terminal.hpp>

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
        struct bgs_terminal_s terminal;
        struct bgs_hierarchy_s hierarchy;
        struct bgs_inspector_s inspector;
    } components;
} bgs_window_t;

int bgs_create_window(bgs_window_t* window, const char* name, const uint16 width, const uint16 height, int flags);
int bgs_destroy_window(bgs_window_t* window);