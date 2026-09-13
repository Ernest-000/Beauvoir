#pragma once

#include <bvr/config.h>
#include <bvr/common.h>

#ifndef BVR_MAX_ASSET
    #define BVR_MAX_ASSET 32
#endif

#define BVR_PHANDLE_GLOBAL  0
#define BVR_PHANDLE_FIELD   1
#define BVR_PHANDLE_LITERAL 2

#define BVR_FILE_HANDLE(obj, path) \
    do { ((*obj).fhandle) = bvr_create_fhandle(path); } while(0); \

/**
 * fhandle are opaque types that can handle 
 * a file/object reference from different origins.
 */
typedef union bvr_fhandle_u {
    // a reference to a json object
    void* token;

    // the path to a file
    long path;
} bvr_fhandle_t; 

typedef struct bvr_phandle_s {
    uint8 origin;
    
} bvr_phandle_t;

/**
 * @brief create a new file handle.
 */
bvr_fhandle_t bvr_create_fhandle(const char* path);

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

