#pragma once

#include <bvr/config.h>
#include <bvr/common.h>

#include <bergson/config.hpp>
#include <bergson/viewport.hpp>

struct bgs_hierarchy_s {
    struct bgs_viewport_s* viewport;
    void* store;
    void* cell;
    void* treeview;

    void* name_column;
    void* visible_column;
};

void* bgs_create_hierarchy(struct bgs_hierarchy_s* hierarchy, struct bgs_viewport_s* viewport);

void bgs_hierarchy_update_items(struct bgs_hierarchy_s* hierarchy);

void bgs_destroy_hierarchy(struct bgs_hierarchy_s* hierarchy);
