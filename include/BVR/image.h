#pragma once

#include <BVR/buffer.h>
#include <BVR/common.h>
#include <BVR/assets.h>

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
#define BVR_TEXTURE_UNIT0   0x84C0
#define BVR_TEXTURE_UNIT1   0x84C1
#define BVR_TEXTURE_UNIT2   0x84C2
#define BVR_TEXTURE_UNIT3   0x84C3
#define BVR_TEXTURE_UNIT4   0x84C4
#define BVR_TEXTURE_UNIT5   0x84C5
#define BVR_TEXTURE_UNIT6   0x84C6
#define BVR_TEXTURE_UNIT7   0x84C7
#define BVR_TEXTURE_UNIT8   0x84C8
#define BVR_TEXTURE_UNIT9   0x84C9
#define BVR_TEXTURE_UNIT10  0x84CA
#define BVR_TEXTURE_UNIT11  0x84CB
#define BVR_TEXTURE_UNIT12  0x84CC
#define BVR_TEXTURE_UNIT13  0x84CD
#define BVR_TEXTURE_UNIT14  0x84CE
#define BVR_TEXTURE_UNIT15  0x84CF
#define BVR_TEXTURE_UNIT16  0x84D0
#define BVR_TEXTURE_UNIT17  0x84D1
#define BVR_TEXTURE_UNIT18  0x84D2
#define BVR_TEXTURE_UNIT19  0x84D3
#define BVR_TEXTURE_UNIT20  0x84D4
#define BVR_TEXTURE_UNIT21  0x84D5
#define BVR_TEXTURE_UNIT22  0x84D6
#define BVR_TEXTURE_UNIT23  0x84D7
#define BVR_TEXTURE_UNIT24  0x84D8
#define BVR_TEXTURE_UNIT25  0x84D9
#define BVR_TEXTURE_UNIT26  0x84DA
#define BVR_TEXTURE_UNIT27  0x84DB
#define BVR_TEXTURE_UNIT28  0x84DC
#define BVR_TEXTURE_UNIT29  0x84DD
#define BVR_TEXTURE_UNIT30  0x84DE
#define BVR_TEXTURE_UNIT31  0x84DF

// texture filters
#define BVR_TEXTURE_FILTER_NEAREST 0x2600
#define BVR_TEXTURE_FILTER_LINEAR 0x2601

// texture wrap modes
#define BVR_TEXTURE_WRAP_REPEAT 0x2901
#define BVR_TEXTURE_WRAP_CLAMP_TO_EDGE 0x812F

// layers tags
#define BVR_LAYER_CLIPPED   0x01
#define BVR_LAYER_Y_SORTED  0x02

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
    Layer informations that is sent to the shader
*/
struct bvr_layer_info_s {
    int32 layer;
    int32 blend_mode;
    int32 opacity;
} __attribute__((packed));

/*
    Contains an image informations and data
*/
typedef struct bvr_image_s {
    int width, height, depth;
    int format, sformat;
    uint8 channels;
    uint8* pixels;

    struct bvr_buffer_s layers;
    struct bvr_asset_reference_s asset;
} bvr_image_t;

/*
    2D texture.
*/
typedef struct bvr_texture_s {
    bvr_image_t image;

    uint32 id, target;
    uint8 unit;

    int filter, wrap;
} bvr_texture_t;

typedef struct bvr_composite_s {
    bvr_image_t* image;
    uint32 framebuffer, tex;
} bvr_composite_t;

/*
    Represent an array of 2D textures
*/
typedef struct bvr_texture_atlas_s
{
    struct bvr_texture_s texture;
    struct {
        uint16 width, height;
        uint16 count;
    } tiles;
} bvr_texture_atlas_t;

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

/*
    Create a raw OpenGL texture from another texture.
    This function returns the new texture's id.
*/
int bvr_create_view_texture(bvr_texture_t* origin, bvr_texture_t* dest, int width, int height, int layer);

void bvr_destroy_image(bvr_image_t* image);

/* 2D TEXTURE */
int bvr_create_texture_from_image(bvr_texture_t* texture, bvr_image_t* image, int filter, int wrap);
int bvr_create_texturef(bvr_texture_t* texture, FILE* file, int filter, int wrap);
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
    Unbind textures
*/
void bvr_texture_disable(bvr_texture_t* texture);
void bvr_destroy_texture(bvr_texture_t* texture);

/* ATLAS TEXTURE */
int bvr_create_texture_atlasf(bvr_texture_atlas_t* atlas, FILE* file, uint32 tile_width, uint32 tile_height, int filter, int wrap);
BVR_H_FUNC int bvr_create_texture_atlas(bvr_texture_atlas_t* atlas, const char* path, uint32 tile_width, uint32 tile_height, int filter, int wrap){
    BVR_FILE_EXISTS(path);
    
    bvr_uuid_t* id = bvr_register_asset(path, BVR_OPEN_READ);
    if(id){
        atlas->texture.image.asset.origin = BVR_ASSET_ORIGIN_PATH;
        bvr_copy_uuid(*id, atlas->texture.image.asset.pointer.asset_id);
    }

    FILE* file = fopen(path, "rb");
    int success = bvr_create_texture_atlasf(atlas, file, tile_width, tile_height, filter, wrap);
    fclose(file);
    return success;
}

/* LAYERED TEXTURE */
int bvr_create_layered_texturef(bvr_texture_t* texture, FILE* file, int filter, int wrap);
BVR_H_FUNC int bvr_create_layered_texture(bvr_texture_t* texture, const char* path, int filter, int wrap){
    BVR_FILE_EXISTS(path);
    
    bvr_uuid_t* id = bvr_register_asset(path, BVR_OPEN_READ);
    if(id){
        texture->image.asset.origin = BVR_ASSET_ORIGIN_PATH;
        bvr_copy_uuid(*id, texture->image.asset.pointer.asset_id);
    }

    FILE* file = fopen(path, "rb");
    int success = bvr_create_layered_texturef(texture, file, filter, wrap);
    fclose(file);
    return success;
}

/*
    Create a new composite. A composite is a buffer that will store an image's result before rendering it to the screen.
*/
int bvr_create_composite(bvr_composite_t* composite, bvr_image_t* target);

/*
    Set this composite as the target framebuffer 
*/
void bvr_composite_enable(bvr_composite_t* composite);

/*
    Bind this composite as a texture
*/
void bvr_composite_prepare(bvr_composite_t* composite);

void bvr_composite_disable(bvr_composite_t* composite);

void bvr_destroy_composite(bvr_composite_t* composite);