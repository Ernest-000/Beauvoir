#pragma once

#include <BVR/buffer.h>
#include <BVR/math.h>

#ifndef BVR_COLLIDER_COLLECTION_SIZE
    #define BVR_COLLIDER_COLLECTION_SIZE 128
#endif

#define BVR_COLLISION_DISABLE   0x00
#define BVR_COLLISION_ENABLE    0x01
#define BVR_COLLISION_AGRESSIVE 0x02
#define BVR_COLLISION_PASSIVE   0x04

/*
    Contains an array of collider pointers.
*/
typedef struct bvr_pool_s bvr_collider_collection_t;

typedef enum bvr_collider_shape_e {
    BVR_COLLIDER_EMPTY,
    BVR_COLLIDER_BOX,
    BVR_COLLIDER_TRIARRAY
} bvr_collider_shape_t;

struct bvr_body_s {
    float acceleration;
    vec3 direction;

    /* is the body passive or aggressive? */
    char mode;
};

typedef struct bvr_collider_s {
    struct bvr_body_s body;

    struct bvr_buffer_s geometry;
    bvr_collider_shape_t shape;
    bool is_enabled;
    bool is_inverted;

    struct bvr_transform_s* transform;
} bvr_collider_t;

struct bvr_collision_result_s {
    int8 collide;

    float distance;
    vec3 direction;

    bvr_collider_t* other;
} __attribute__((packed));

/*
    Add force to move a body
*/
void bvr_body_add_force(struct bvr_body_s* body, float x, float y, float z);

/*
    Translate the body with applied forces
*/
void bvr_body_apply_motion(struct bvr_body_s* body, struct bvr_transform_s* transform);

void bvr_create_collider(bvr_collider_t* collider, float* vertices, uint64 count);

/*
    Check how two colliders interact.
*/
void bvr_compare_colliders(bvr_collider_t* a, bvr_collider_t* b, struct bvr_collision_result_s* result);

void bvr_destroy_collider(bvr_collider_t* collider);

/*
    https://stackoverflow.com/a/9755252
*/
BVR_H_FUNC int bvr_is_point_inside_triangle(vec2 s, vec2 a, vec2 b, vec2 c){
    vec2 as;
    vec2_sub(as, s, a);

    int s_ab = (b[0] - a[0]) * as[1] - (b[1] - a[1]) * as[0] > 0;
    return !(
        ((c[0] - a[0]) * as[1] - (c[1] - a[1]) * as[0] > 0 == s_ab ) ||
        ((c[0] - b[0]) * (s[1] - b[1]) - (c[1] - b[1])*(s[0] - b[0]) > 0 != s_ab)
    );
}

BVR_H_FUNC void bvr_invert_direction(struct bvr_body_s* body){
    vec3_scale(body->direction, body->direction, -1.0f);
}