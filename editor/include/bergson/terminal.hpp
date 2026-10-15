#pragma once

#include <bvr/config.h>
#include <bvr/common.h>

#include <bergson/config.hpp>

struct bgs_terminal_s {
    void* parent;
    void* container;
    void* scroll;
};


void* bgs_create_terminal(struct bgs_terminal_s* terminal, void* parent);
void bgs_destory_terminal(struct bgs_terminal_s* terminal);