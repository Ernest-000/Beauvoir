#pragma once

#include <bvr/config.h>
#include <bvr/common.h>

#include <bvr/collections/string.h>

#include <pthread.h>

struct bvr_assembly_attributes_s {
    const char* source_file;
    const char* include_path;
    const char* entry_name;
    const char* name;
};

typedef void (*bvr_entry_clbk)(void);

typedef struct bvr_assembly_s {
    // tcc state reference
    void* handle;

    // the assembly name
    bvr_string_t name;

    pthread_t thread;
    uint32 srcf_count;

    // the name of the entry point function
    bvr_string_t entry_name;
    bvr_entry_clbk entry;

} bvr_assembly_t;

int bvr_create_assembly(bvr_assembly_t* assembly, struct bvr_assembly_attributes_s* attributes);

int bvr_assembly_run(bvr_assembly_t* assembly);

void bvr_destroy_assembly(bvr_assembly_t* assembly);