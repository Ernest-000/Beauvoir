#pragma once

#include <bvr/config.h>
#include <bvr/common.h>
#include <bvr/math.h>

#include <bvr/mesh.h>

#if !defined(BVR_CREATE_LANDSCAPE)
    #define BVR_CREATE_LANDSCAPE(actor, tile_per_column, tile_per_row, width, height, layer) \
        actor->grid.count[0] = (uint16)tile_per_column; \
        actor->grid.count[1] = (uint16)tile_per_row; \
        actor->grid.size[0] = (float)width; \
        actor->grid.size[1] = (float)height; \
        actor->grid.layers = (uint8)layer; 
#endif

/*
    landscape tile's informations
*/
//struct bvr_tile_s {
//    uint8 altitude;
//    uint8 texture;
//    uint8 norm_x;
//    uint8 norm_y;
//};

struct bvr_tile_s {
    uint16 altitude;
    uint16 texture;
};

struct bvr_landscape_layer_s {
    bvr_string_t name;
};

typedef struct bvr_landscape_s {
    bvr_mesh_t* mesh;

    struct bvr_landscape_grid_s {
        int tile_per_row;
        int tile_per_column;
        int tile_size[2];
    
        uint32 tile_count;

        struct bvr_tile_s* tiles;
        struct bvr_buffer_s layers;
    } grid;
} bvr_landscape_t;

int bvr_create_landscapef(bvr_landscape_t* landscape, FILE* file);
BVR_H_FUNC int bvr_create_landscape(bvr_landscape_t* landscape, const char* path){
    FILE* file = fopen(path, "rb");
    int status = bvr_create_landscapef(landscape, file);
    fclose(file);
    return status; 
}

int bvr_create_landscape_empty(bvr_landscape_t* landscape, 
    uint16 tile_per_row, uint16 tile_per_column, vec2 tile_size, uint8 layer_count);

void bvr_landscape_set_tile(bvr_landscape_t* landscape, uint32 x, uint32 y, uint8 layer, struct bvr_tile_s tile);

BVR_H_FUNC struct bvr_tile_s bvr_landscape_get_tile(bvr_landscape_t* landscape, uint32 x, uint32 y, uint8 layer){
    uint64 tiles_per_layer = landscape->grid.tile_count / BVR_BUFFER_COUNT(landscape->grid.layers);
    uint64 vert_per_row = landscape->grid.tile_per_row * 2 + 3;

    uint32 id = y * vert_per_row + clampi(x * 2, 0, vert_per_row - 2) + 3;
    return landscape->grid.tiles[tiles_per_layer * layer + id];
}

void bvr_destroy_landscape(bvr_landscape_t* landscape);