#pragma once

#include <bvr/config.h>
#include <bvr/common.h>

#ifndef BVR_MAX_ASSET
    #define BVR_MAX_ASSET 32
#endif

#define BVR_PHANDLE_NONE    0
#define BVR_PHANDLE_FIELD   1
#define BVR_PHANDLE_LITERAL 2
#define BVR_PHANDLE_RAW     3

#define BVR_FILE_HANDLE(obj, path) \
    do { ((*obj).fhandle) = bvr_create_fhandle(path); } while(0); \

#define BVR_CREATE_LITERAL_PHANDLE(value) bvr_create_literal_phandle(sizeof(value), value);

#define BVR_CREATE_FIELD_PHANDLE(class, _actor, _field)     \
    ((bvr_phandle_t) {                                      \
        .origin = BVR_PHANDLE_FIELD,                        \ 
        .pointer.field.self = (struct bvr_actor_s*)(_actor),\
        .pointer.field.field = bvr_actor_get_field(         \
            #class, (_field)                                \
        )                                                   \
    })

#define BVR_CREATE_RAW_PHANDLE(_ref) \
    ((bvr_phandle_t){.origin = BVR_PHANDLE_RAW, .pointer.raw.pointer = _ref, .pointer.raw.size = sizeof(_ref)})

#define BVR_CREATE_NULL_PHANDLE() \
    ((bvr_phandle_t){.origin = BVR_PHANDLE_NONE, .pointer.raw.pointer = NULL, .pointer.raw.size = 0})

// opaque field struct
struct bvr_actor_fields_s;
struct bvr_actor_s;

/**
 * @brief fhandle are opaque types that can handle 
 * a file/object reference from different origins.
 */
typedef union bvr_fhandle_u {
    // a reference to a json object
    void* token;

    // the path to a file
    long path;
} __struct_align16 bvr_fhandle_t; 

/**
 * @brief phandle are opaque types that can handle
 * a pointer reference from different origin
 * while keeping it serialized throught actors.
 */
typedef struct bvr_phandle_s {
    uint8 origin;
    union {
        struct {
            uint8 size;
            uint8 value[16];
        } literal;

        struct {
            struct bvr_actor_s* self;
            struct bvr_actor_fields_s* field;
        } field;

        struct {
            void* pointer;

            // not the size of the object
            uint32 size; 
        } raw;
    } pointer;
} __struct_align16 bvr_phandle_t;

/**
 * @brief create a new file handle.
 */
bvr_fhandle_t bvr_create_fhandle(const char* path);

/**
 * @brief create a new phandle from a constant value. The value will be copied
 * into the internal phandle buffer.
 * @param size the size of the value (maximum 16 bytes)
 * @param value a pointer to the value
 */
bvr_phandle_t bvr_create_literal_phandle(uint8 size, void* value);

void* bvr_phandle_get(bvr_phandle_t* handle);

/**
 * @brief load a page from a json file
 */
/*int bvr_load_pagef(bvr_page_t* page, FILE* file);

BVR_H_FUNC int bvr_load_page(bvr_page_t* page, const char* path){
    FILE* f = fopen(path, "rb");
    int success = bvr_load_pagef(page, f);
    fclose(f);
    return success;
}*/

