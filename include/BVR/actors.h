#pragma once

#include <BVR/image.h>
#include <BVR/shader.h>
#include <BVR/mesh.h>
#include <BVR/physics.h>
#include <BVR/math.h>

#include <stdio.h>

/*
    Does not destroy the actor when freeing the page
*/
#define BVR_ACTOR_NOT_FREE 0x00001

/*
    This actor can only block object; this means that this actor shall not 
    move.
*/
#define BVR_DYNACTOR_PASSIVE 0x00010

/*
    This actor can move and generate collisions.
*/
#define BVR_DYNACTOR_AGGRESSIVE 0x00020

/*
    Create collider's box shape from an object's bounds
*/
#define BVR_DYNACTOR_CREATE_COLLIDER_FROM_BOUNDS 0x00040

/*
    Create a new triangulated mesh from an object's mesh. 
    This new mesh with be used to 
*/
#define BVR_DYNACTOR_TRIANGULATE_COLLIDER_FROM_VERTICES 0x00080

/*
    Define if this actor should link its y position to its sorting method
*/
#define BVR_DYNACTOR_Y_SORTED   0x00100

/*
    When the bitmap actor is created, it will create a new collider
*/
#define BVR_BITMAP_CREATE_COLLIDER 0x01000

typedef enum bvr_actor_type_e {
    BVR_NULL_ACTOR,
    BVR_EMPTY_ACTOR,
    BVR_LAYER_ACTOR,
    BVR_TEXTURE_ACTOR,
    BVR_STATIC_ACTOR,
    BVR_DYNAMIC_ACTOR,
    BVR_LANDSCAPE_ACTOR
} bvr_actor_type_t;

/*
    Each actor based struct must start by 
    an actor struct.
*/

struct bvr_actor_s {
    bvr_string_t name;
    bvr_actor_type_t type;
    bvr_uuid_t id;

    int flags;

    bool active;
    uint16 order_in_layer;

    struct bvr_transform_s transform;
};

typedef struct bvr_actor_s bvr_empty_actor_t;

typedef struct bvr_layer_actor_s {
    struct bvr_actor_s self;

    bvr_mesh_t mesh;
    bvr_shader_t shader;
    
    bvr_texture_t texture;
    bvr_composite_t composite;
} bvr_layer_actor_t;

typedef struct bvr_static_actor_s {
    struct bvr_actor_s self;

    bvr_mesh_t mesh;
    bvr_shader_t shader;
} bvr_static_actor_t;

typedef struct bvr_dynamic_actor_s {
    struct bvr_actor_s self;

    bvr_mesh_t mesh;
    bvr_shader_t shader;
    bvr_collider_t collider;
} bvr_dynamic_actor_t;

typedef struct bvr_texture_actor_s {
    struct bvr_actor_s self;

    bvr_mesh_t mesh;
    bvr_shader_t shader;
    bvr_collider_t collider;

    bvr_texture_t bitmap;
} bvr_texture_actor_t;

typedef struct bvr_landscape_actor_s {
    struct bvr_actor_s self;

    bvr_mesh_t mesh;
    bvr_shader_t shader;
    
    bvr_texture_atlas_t atlas;

    struct {
        vec2 count;
        vec2 resolution;
        
        uint8 layers;
    } dimension;
} bvr_landscape_actor_t;

/*
    landscape tile's informations
*/
struct bvr_tile_s {
    uint8 altitude;
    uint8 texture;
    uint8 norm_x;
    uint8 norm_y;
};

/*
    Initialize a generic actor.
*/
void bvr_create_actor(struct bvr_actor_s* actor, const char* name, bvr_actor_type_t type, int flags);
void bvr_destroy_actor(struct bvr_actor_s* actor);

void bvr_draw_actor(struct bvr_actor_s* actor, int drawmode);

BVR_H_FUNC int bvr_is_actor_null(struct bvr_actor_s* actor){
    return actor == NULL || actor->type == BVR_NULL_ACTOR;
}