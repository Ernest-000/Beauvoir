#pragma once

#include <bvr/config.h>
#include <bvr/math.h>

#include <bvr/mesh.h>
#include <bvr/shader.h>
#include <bvr/image.h>
#include <bvr/physics.h>

#include <bvr/collections/string.h>

#define BVR_DECLARE_ACTOR(name, f0, f1, f2, ...)                \
    static const struct bvr_actor_vtable_s _##name##able = {    \
        f0, f1, f2                                              \
    };                                                          \
    struct name {                                               \
        struct bvr_actor_s self;                                \
        __VA_ARGS__                                             \
    };                                                          \
    typedef struct name name;  

#define BVR_ACTOR_FIELD(type, name) type name;
#define BVR_ACTOR_METHOD(function, pointer) .function = pointer

#define BVR_CREATE_ACTOR(actor, type)               \
    {                                               \
        (actor)->self.vtable =                          \
            (struct bvr_actor_vtable_s*)&_##type##able; \
    }   

#define BVR_ACTOR_DRAW(actor, drawmode) do {(actor)->self.vtable->draw(&(actor)->self, drawmode); } while(0)
#define BVR_ACTOR_UPDATE(actor) do {(actor)->self.vtable->update(&(actor)->self); } while(0)
#define BVR_ACTOR_DESTROY(actor) do {(actor)->self.vtable->destroy(&(actor)->self); } while(0)

// opaque type for vtable
struct bvr_actor_s;

struct bvr_actor_vtable_s {
    void (*update)(struct bvr_actor_s* self);
    void (*draw)(struct bvr_actor_s* self, int drawmode);
    void (*destroy)(struct bvr_actor_s* self);
    void (*user)(struct bvr_actor_s* self);
};

struct bvr_actor_s {
    bvr_transform_t transform;
    bvr_string_t name;

    struct bvr_actor_s* parent;
    struct bvr_actor_s** childs;
    uint16 child_slots;
    uint16 children_count;

    uint32 hash;
    uint16 flags;

    uint16 order_in_layer;
    uint16 active;

    struct bvr_actor_vtable_s* vtable;
};

// generic functions
void bvr_actor_set_parent(struct bvr_actor_s* actor, struct bvr_actor_s* parent);

// static mesh functions

void bvr_static_mesh_draw(struct bvr_actor_s* mesh, int drawmode);
void bvr_static_mesh_update(struct bvr_actor_s* mesh);
void bvr_static_mesh_destroy(struct bvr_actor_s* mesh);

// dynamic mesh functions

void bvr_dynamic_mesh_draw(struct bvr_actor_s* mesh, int drawmode);
void bvr_dynamic_mesh_update(struct bvr_actor_s* mesh);
void bvr_dynamic_mesh_destroy(struct bvr_actor_s* mesh);

// static mesh
BVR_DECLARE_ACTOR(
    bvr_static_mesh_t,
    BVR_ACTOR_METHOD(draw, bvr_static_mesh_draw),
    BVR_ACTOR_METHOD(update, bvr_static_mesh_update),
    BVR_ACTOR_METHOD(destroy, bvr_static_mesh_destroy),
    BVR_ACTOR_FIELD(bvr_mesh_t, mesh)
    BVR_ACTOR_FIELD(bvr_shader_t, shader)
    BVR_ACTOR_FIELD(bvr_texture_t, texture)
)

// dynamic mesh
BVR_DECLARE_ACTOR(
    bvr_dynamic_mesh_t,
    BVR_ACTOR_METHOD(draw, bvr_dynamic_mesh_draw),
    BVR_ACTOR_METHOD(update, bvr_dynamic_mesh_update),
    BVR_ACTOR_METHOD(destroy, bvr_dynamic_mesh_destroy),
    BVR_ACTOR_FIELD(bvr_mesh_t, mesh)
    BVR_ACTOR_FIELD(bvr_shader_t, shader)
    BVR_ACTOR_FIELD(bvr_texture_t, texture)
    BVR_ACTOR_FIELD(bvr_collider_t, collider)
)