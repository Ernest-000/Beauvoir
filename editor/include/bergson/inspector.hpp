#pragma once

#include <bvr/config.h>
#include <bvr/common.h>
#include <bvr/actors.h>

#include <bergson/config.hpp>

struct bgs_inspector_s {
    void* parent;

    void* widget;
    void* store;

    struct bvr_actor_s* actor;
};

struct bgs_inspector_field_s {
    const char* name;
    uint32 type;

    bvr_phandle_t handle;
};

void* bgs_create_inspector(struct bgs_inspector_s* inspector, void* parent);

void bgs_inspector_set_actor(struct bgs_inspector_s* inspector, struct bvr_actor_s* actor);

void bgs_destroy_inspector(struct bgs_inspector_s* inspector);