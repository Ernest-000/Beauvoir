#pragma once

#include <bvr/config.h>
#include <bvr/common.h>

struct bgs_menu_s {
    void* parent;

    void* menu_widget;
    void* file_widget;
    void* edit_widget;
};

void* bgs_create_menuitems(struct bgs_menu_s* menu, void* parent);
void bgs_destroy_menuitems(struct bgs_menu_s* menu);