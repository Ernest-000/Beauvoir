#pragma once

#include <bvr/common.h>
#include <bvr/buffer.h>

/* Contains of all user's game scripts data */
typedef struct bvr_assembly_s {
    // tcc state
    void* handle;

    bvr_string_t name;
    
    // entry point callback
    void (*main_clbk)(void);
} bvr_assembly_t;

int bvr_create_assembly(bvr_assembly_t* assembly, const char* name);
int bvr_assembly_add(bvr_assembly_t* assembly, const char* path);
int bvr_run(bvr_assembly_t* assembly);
void bvr_destroy_assembly(bvr_assembly_t* assembly);

// internal use
// TODO: clean
void bvr_load_c_symbols(void* _s);