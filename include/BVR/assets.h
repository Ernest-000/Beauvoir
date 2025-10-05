#pragma once

#include <BVR/config.h>
#include <BVR/utils.h>
#include <BVR/buffer.h>

#define BVR_OPEN_READ  1
#define BVR_OPEN_WRITE 2

enum bvr_asset_reference_origin_e {
    BVR_ASSET_ORIGIN_NONE,
    BVR_ASSET_ORIGIN_RAW,
    BVR_ASSET_ORIGIN_PATH,
};

struct bvr_asset_reference_s {
    enum bvr_asset_reference_origin_e origin;

    union {
        // reference to an asset
        bvr_uuid_t asset_id;

        // raw data 
        struct bvr_buffer_s raw_data;
    } pointer __attribute__ ((packed));
    
} __attribute__ ((packed));

typedef struct bvr_asset_s {
    bvr_uuid_t id;

    bvr_string_t path;
    char open_mode;
} bvr_asset_t;

/*
    Add a new asset to the asset list
*/
bvr_uuid_t* bvr_register_asset(const char* path, char open_mode);

/*
    Retreive asset's informations from the list
*/
bvr_uuid_t* bvr_find_asset(const char* path, bvr_asset_t* asset);

int bvr_find_asset_uuid(bvr_uuid_t uuid, bvr_asset_t* asset);

/*
    Open a file stream to a game asset
*/
BVR_H_FUNC FILE* bvr_open_asset(bvr_asset_t* asset){
    char open_mode[2] = "rb";
    if(asset->open_mode == BVR_OPEN_WRITE) {
        open_mode[0] = 'w';
    }

    BVR_PRINTF("found %s", asset->path.string);
    return fopen(asset->path.string, open_mode);
}