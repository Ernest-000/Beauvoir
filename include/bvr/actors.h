#pragma once

#include <bvr/config.h>
#include <bvr/math.h>

#include <bvr/assets.h>
#include <bvr/mesh.h>
#include <bvr/shader.h>
#include <bvr/image.h>
#include <bvr/physics.h>

#include <bvr/collections/string.h>

#ifndef BVR_MAX_ACTOR_CLASSES
    #define BVR_MAX_ACTOR_CLASSES 128
#endif

#define BVR_ACTOR_FIELD_DECLARE(parent, ctype, cname) ctype cname;
#define BVR_ACTOR_FIELD_REGISTRY(parent, ctype, cname) { #cname, offsetof(parent, cname), sizeof(ctype) },
#define BVR_ACTOR_FIELD_IGNORE(parent, ctype, cname) 

#define BVR_ACTOR_METHOD_IMPLEMENT(parent, fname, fclbk) .fname = fclbk,
#define BVR_ACTOR_METHOD_IGNORE(parent, fname, fclbk)

#define BVR_DEFINE_ACTOR(name, fields)                          \
    struct name {                                               \
        struct bvr_actor_s self;                                \
        fields(name,                                            \
            BVR_ACTOR_FIELD_DECLARE,                            \
            BVR_ACTOR_METHOD_IGNORE)                            \
    };                                                          \
    typedef struct name name;                                   \
    extern const struct bvr_actor_vtable_s _##name##able;              

#define BVR_IMPLEMENT_ACTOR(name, fields, serializable)         \
    static const struct bvr_actor_fields_s _##name##ablef[] = { \
        fields(name,                                            \
            BVR_ACTOR_FIELD_REGISTRY,                           \
            BVR_ACTOR_METHOD_IGNORE                             \
        )                                                       \
    };                                                          \
    const struct bvr_actor_vtable_s _##name##able = {           \
        fields(name,                                            \
            BVR_ACTOR_FIELD_IGNORE,                             \
            BVR_ACTOR_METHOD_IMPLEMENT)                         \
        .ftable = _##name##ablef,                               \
        .field_count = sizeof(_##name##ablef) /                 \
            sizeof(struct bvr_actor_fields_s)                   \
    };                                                          \
    BVR_H_FUNC void __constructor _cst_##name##able(void) {     \
        if(serializable){                                       \
            bvr_actor_serializable(#name, &_##name##able);      \
        }                                                       \
    }

#define BVR_CREATE_ACTOR(actor, type)                   \
    {                                                   \
        (actor)->self.vtable =                          \
            (struct bvr_actor_vtable_s*)&_##type##able; \
    }   

#define BVR_ACTOR_DRAW(actor, drawmode) do {(actor)->self.vtable->draw(&(actor)->self, drawmode); } while(0)
#define BVR_ACTOR_UPDATE(actor) do {(actor)->self.vtable->update(&(actor)->self); } while(0)
#define BVR_ACTOR_DESTROY(actor) do {(actor)->self.vtable->destroy(&(actor)->self); } while(0)

// opaque type for vtable
struct bvr_actor_s;

struct bvr_actor_fields_s {
    const char* name;
    uint16 offset;
    uint16 size;
};

struct bvr_actor_vtable_s {
    void (*update)(struct bvr_actor_s* self);
    void (*draw)(struct bvr_actor_s* self, int drawmode);
    void (*destroy)(struct bvr_actor_s* self);
    void (*serialize)(struct bvr_actor_s* self, bvr_fhandle_t token);
    void (*deserialize)(struct bvr_actor_s* self, bvr_fhandle_t token);

    // depreciate
    void (*user)(struct bvr_actor_s* self);

    // fields
    struct bvr_actor_fields_s* ftable;
    uint16 field_count;
};

struct bvr_actor_s {
    bvr_transform_t transform;
    bvr_string_t name;

    struct bvr_actor_s* parent;
    struct bvr_actor_s** childs;
    uint16 child_slots;
    uint16 children_count;

    // gui equivalent
    uint32 hash;
    uint16 flags;

    uint16 order_in_layer;
    uint16 active;

    struct bvr_actor_vtable_s* vtable;
};

// generic functions
void bvr_actor_serializable(const char* cname, struct bvr_actor_vtable_s* table);
void bvr_actor_set_parent(struct bvr_actor_s* actor, struct bvr_actor_s* parent);

#include "actorsdef.h"