#pragma once

#include <bvr/buffer.h>
#include <bvr/common.h>
#include <bvr/assets.h>
#include <bvr/math.h>

#include <stdint.h>
#include <stdio.h>

// color modes
#define BVR_RED     0x0
#define BVR_GREEN   0x1
#define BVR_BLUE    0x2
#define BVR_ALPHA   0x3

// color channels
#define BVR_R       0x1903
#define BVR_RG      0x8227
#define BVR_RGB     0x1907
#define BVR_BGR     0x80E0
#define BVR_RGBA    0x1908
#define BVR_BGRA    0x80E1

// sized color channels
#define BVR_RED8    0x8229
#define BVR_RG8     0x822B
#define BVR_RGB8    0x8051
#define BVR_RGBA8   0x8058
#define BVR_RED16   0x822A
#define BVR_RG16    0x822C
#define BVR_RGB16   0x8054
#define BVR_RGBA16  0x805B

// texture units
#define BVR_TEXTURE_UNIT0  0    
#define BVR_TEXTURE_UNIT1  1    
#define BVR_TEXTURE_UNIT2  2    
#define BVR_TEXTURE_UNIT3  3    
#define BVR_TEXTURE_UNIT4  4    
#define BVR_TEXTURE_UNIT5  5    
#define BVR_TEXTURE_UNIT6  6    
#define BVR_TEXTURE_UNIT7  7    
#define BVR_TEXTURE_UNIT8  8    
#define BVR_TEXTURE_UNIT9  9    
#define BVR_TEXTURE_UNIT10 10   
#define BVR_TEXTURE_UNIT11 11   
#define BVR_TEXTURE_UNIT12 12   
#define BVR_TEXTURE_UNIT13 13   
#define BVR_TEXTURE_UNIT14 14   
#define BVR_TEXTURE_UNIT15 15   
#define BVR_TEXTURE_UNIT16 16   
#define BVR_TEXTURE_UNIT17 17   
#define BVR_TEXTURE_UNIT18 18   
#define BVR_TEXTURE_UNIT19 19   
#define BVR_TEXTURE_UNIT20 20   
#define BVR_TEXTURE_UNIT21 21   
#define BVR_TEXTURE_UNIT22 22   
#define BVR_TEXTURE_UNIT23 23   
#define BVR_TEXTURE_UNIT24 24   
#define BVR_TEXTURE_UNIT25 25   
#define BVR_TEXTURE_UNIT26 26   
#define BVR_TEXTURE_UNIT27 27   
#define BVR_TEXTURE_UNIT28 28   
#define BVR_TEXTURE_UNIT29 29   
#define BVR_TEXTURE_UNIT30 30   
#define BVR_TEXTURE_UNIT31 31   

// texture filters
#define BVR_TEXTURE_FILTER_NEAREST 0x2600
#define BVR_TEXTURE_FILTER_LINEAR 0x2601

// texture wrap modes
#define BVR_TEXTURE_WRAP_REPEAT 0x2901
#define BVR_TEXTURE_WRAP_CLAMP_TO_EDGE 0x812F

// layers tags
#define BVR_LAYER_CLIPPED   0x01
#define BVR_LAYER_Y_SORTED  0x02

#if !defined(BVR_CREATE_ATLAS_TILE_SIZE)
    #define BVR_CREATE_ATLAS_TILE_SIZE(desc, width, height) \
        desc.tile_width = (width); \
        desc.tile_height = (height);\
        desc.tile_per_row = 0; \
        desc.tile_per_column = 0;
#endif

#if !defined(BVR_CREATE_ATLAS_TILE_COUNT)
    #define BVR_CREATE_ATLAS_TILE_COUNT(desc, tile_per_row, tile_per_column) \
        desc.tile_width = 0; \
        desc.tile_height = 0;\
        desc.tile_per_row = tile_per_row; \
        desc.tile_per_column = tile_per_column;
#endif

typedef enum bvr_layer_blend_mode_e {
    BVR_LAYER_BLEND_PASSTHROUGH,
    BVR_LAYER_BLEND_NORMAL,
    BVR_LAYER_BLEND_DISSOLVE,
    BVR_LAYER_BLEND_DARKEN,
    BVR_LAYER_BLEND_MULTIPLY,
    BVR_LAYER_BLEND_COLORBURN,
    BVR_LAYER_BLEND_LINEARBURN,
    BVR_LAYER_BLEND_DARKERCOLOR,
    BVR_LAYER_BLEND_LIGHTEN,
    BVR_LAYER_BLEND_SCREEN,
    BVR_LAYER_BLEND_COLORDODGE,
    BVR_LAYER_BLEND_LINEARDODGE,
    BVR_LAYER_BLEND_LIGHTERCOLOR,
    BVR_LAYER_BLEND_OVERLAY,
    BVR_LAYER_BLEND_SOFTLIGHT,
    BVR_LAYER_BLEND_HARDLIGHT,
    BVR_LAYER_BLEND_VIVIDLIGHT,
    BVR_LAYER_BLEND_LINEARLIGHT,
    BVR_LAYER_BLEND_PINLIGHT,
    BVR_LAYER_BLEND_HARDMIX,
    BVR_LAYER_BLEND_DIFFERENCE,
    BVR_LAYER_BLEND_EXCLUSION,
    BVR_LAYER_BLEND_SUBSTRACT,
    BVR_LAYER_BLEND_DIVIDE,
    BVR_LAYER_BLEND_HUE,
    BVR_LAYER_BLEND_SATURATION,
    BVR_LAYER_BLEND_COLOR,
    BVR_LAYER_BLEND_LUMINOSITY
} bvr_layer_blend_mode_t;

/*
    Contains image layer informations
*/
typedef struct bvr_layer_s {
    bvr_string_t name;
    uint16 flags;

    uint16 width, height;
    short anchor_x, anchor_y;

    uint8 opacity;
    bvr_layer_blend_mode_t blend_mode;
} bvr_layer_t;

/*
    Layer informations sent to the shader
*/
struct bvr_layer_info_s {
    uint8 layer;
    uint8 blend_mode;
    uint8 opacity;
    uint8 reserved;
} __attribute__((packed));

/*
    Contains informations and data of an image
*/
typedef struct bvr_image_s {
    int width, height;
    int format, sformat;

    uint8 channels, depth;
    uint8* pixels;

    struct bvr_buffer_s layers;
    struct bvr_asset_reference_s asset;
} bvr_image_t;

/**
 * Contains atlas informations
 */
struct bvr_atlas_s {
    int tile_per_row;
    int tile_per_column;
    
    int width, height;

    /**
     * Currently rendered tile's id
     */
    uint32 brush;

    uint16 padding[4];
};

typedef struct bvr_atlas_desc_s {
    uint32 tile_width;
    uint32 tile_height;

    uint16 padding_bottom;
    uint16 padding_left;
    uint16 padding_right;
    uint16 padding_top;

    uint16 tile_per_row;
    uint16 tile_per_column;
} bvr_atlas_desc_t;

/*
    Global texture object.
*/
typedef struct bvr_texture_s {
    bvr_image_t image;

    uint32 id, target;
    uint8 unit;

    int filter, wrap;

    /**
     * This works as an appentice.
     * This will not be used for non-atlas textures
     * But it stores informations about tiling for atlas textures.
     */
    struct bvr_atlas_s tiles;
} bvr_texture_t;

typedef struct bvr_composite_s {
    bvr_image_t* image;
    uint32 framebuffer, tex;
} bvr_composite_t;

int bvr_create_imagef(bvr_image_t* image, FILE* file);
BVR_H_FUNC int bvr_create_image(bvr_image_t* image, const char* path){
    BVR_FILE_EXISTS(path);
    
    bvr_uuid_t* id = bvr_register_asset(path, BVR_OPEN_READ);
    if(id){
        image->asset.origin = BVR_ASSET_ORIGIN_PATH;
        bvr_copy_uuid(*id, image->asset.pointer.asset_id);
    }

    FILE* file = fopen(path, "rb");
    int success = bvr_create_imagef(image, file);
    fclose(file);
    return success;
}

/*
    Create a new bitmap buffer from a file.
*/
int bvr_create_bitmap(bvr_image_t* image, const char* path, int channel);

/*
    Flip a pixel buffer vertically
*/
void bvr_flip_image_vertically(bvr_image_t* image);

/*
    Copy a specific image channel over another pixel buffer.
    The targeted pixel buffer must be allocated.
*/
int bvr_image_copy_channel(bvr_image_t* image, int channel, uint8* buffer);

// DEPRECIATE
/*
    Create a raw OpenGL texture from another texture.
    This function returns the new texture's id.
*/
//int bvr_create_view_texture(const bvr_composite_t* compose, bvr_texture_t* origin, bvr_texture_t* dest, const uint32 layer);

void bvr_destroy_image(bvr_image_t* image);

/**
 * Create a new 2 dimensional texture from an image.
 * This 2D texture can only handle single-layered rendering. 
 */
int bvr_create_texture_2d(bvr_texture_t* texture, bvr_image_t* image, int filter, int wrap);

/**
 * Create a new 3 dimensional texture from an image.
 * This 3D texture will support layering.
 */
int bvr_create_texture_3d(bvr_texture_t* texture, bvr_image_t* image, int filter, int wrap);

/**
 * Create a new texture from a file. 
 * This will either create 2D or 3D texture depending on layer count.
 */
int bvr_create_texturef(bvr_texture_t* texture, FILE* file, int filter, int wrap);

/**
 * Create a new texture from a file's path. 
 * This will either create 2D or 3D texture depending on layer count.
 */
BVR_H_FUNC int bvr_create_texture(bvr_texture_t* texture, const char* path, int filter, int wrap){
    BVR_FILE_EXISTS(path);

    bvr_uuid_t* id = bvr_register_asset(path, BVR_OPEN_READ);
    if(id){
        texture->image.asset.origin = BVR_ASSET_ORIGIN_PATH;
        bvr_copy_uuid(*id, texture->image.asset.pointer.asset_id);
    }

    FILE* file = fopen(path, "rb");
    int success = bvr_create_texturef(texture, file, filter, wrap);
    fclose(file);
    return success;
}

/*
    Bind a texture. 
*/
void bvr_texture_enable(bvr_texture_t* texture);
    
/*
    Unbind unsed texture.
*/
void bvr_texture_disable(bvr_texture_t* texture);
void bvr_destroy_texture(bvr_texture_t* texture);

/* ATLAS TEXTURE */
int bvr_create_texture_atlasf(bvr_texture_t* texture, FILE* file, bvr_atlas_desc_t* desc, int filter, int wrap);

BVR_H_FUNC int bvr_create_texture_atlas(bvr_texture_t* texture, const char* path, bvr_atlas_desc_t* desc, int filter, int wrap){
    BVR_FILE_EXISTS(path);
    
    bvr_uuid_t* id = bvr_register_asset(path, BVR_OPEN_READ);
    if(id){
        texture->image.asset.origin = BVR_ASSET_ORIGIN_PATH;
        bvr_copy_uuid(*id, texture->image.asset.pointer.asset_id);
    }

    FILE* file = fopen(path, "rb");
    int success = bvr_create_texture_atlasf(texture, file, desc, filter, wrap);
    fclose(file);
    return success;
}

/* LAYERED TEXTURE 
 *int bvr_create_texture_3d(bvr_texture_t* texture, FILE* file, int filter, int wrap);
 *
 *BVR_H_FUNC int bvr_create_layered_texture(bvr_texture_t* texture, const char* path, int filter, int wrap){
 *    BVR_FILE_EXISTS(path);
 *    
 *    bvr_uuid_t* id = bvr_register_asset(path, BVR_OPEN_READ);
 *    if(id){
 *        texture->image.asset.origin = BVR_ASSET_ORIGIN_PATH;
 *        bvr_copy_uuid(*id, texture->image.asset.pointer.asset_id);
 *    }
 *
 *    FILE* file = fopen(path, "rb");
 *    int success = bvr_create_layered_texturef(texture, file, filter, wrap);
 *    fclose(file);
 *    return success;
 *}
 */

/*
    Create a new composite. A composite is a buffer that will store an image's result before rendering it to the screen.
*/
int bvr_create_composite(bvr_composite_t* composite, bvr_image_t* target);

/*
    Set this composite as the target framebuffer 
*/
void bvr_composite_enable(bvr_composite_t* composite, bvr_transform_t* const transform);

/*
    Bind this composite as a texture
*/
void bvr_composite_prepare(bvr_composite_t* composite);

void bvr_composite_disable(bvr_composite_t* composite);

void bvr_destroy_composite(bvr_composite_t* composite);