#pragma once

#include <bvr/config.h>
#include <bvr/common.h>

#include <bvr/collections/string.h>
#include <bvr/collections/table.h>

#include <pthread.h>

#ifndef BVR_MAX_ENTRIES
    #define BVR_MAX_ENTRIES 64
#endif

struct bvr_assembly_attributes_s {
    const char* source_file;
    const char* include_path;
    const char* entry_func;
    const char* draw_func;
    const char* name;
};

typedef void (*bvr_entry_clbk)(void);

typedef struct bvr_assembly_s {
    // tcc state reference
    void* handle;

    // the assembly name
    bvr_string_t name;

    pthread_t thread;

    // the number of source files
    uint32 srcf_count;

    // the name of the entry point function
    bvr_table_t entries;
} bvr_assembly_t;


int bvr_create_assembly(bvr_assembly_t* assembly, struct bvr_assembly_attributes_s* attributes);
int bvr_assembly_reload(bvr_assembly_t* assembly, struct bvr_assembly_attributes_s* attributes);

int bvr_assembly_run(bvr_assembly_t* assembly, const char* function);

void bvr_destroy_assembly(bvr_assembly_t* assembly);