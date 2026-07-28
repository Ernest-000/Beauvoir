#pragma once

#include <bvr/config.h>
#include <bvr/common.h>

#include <bvr/collections/string.h>

struct bvr_assembly_attributes_s {
    const char* source_path;
    const char* include_path;
    const char* name;
};

typedef struct bvr_assembly_s {
    // tcc state reference
    void* handle;

    bvr_string_t name;

    void (*entry)(void);
} bvr_assembly_t;

int bvr_create_assembly(bvr_assembly_t* assembly, struct bvr_assembly_attributes_s* attributes);

int bvr_run(bvr_assembly_t* assembly);

void bvr_destroy_assembly(bvr_assembly_t* assembly);