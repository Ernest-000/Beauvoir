#ifndef BVR_ACTOR_DEF_H
#define BVR_ACTOR_DEF_H

void bvr_static_mesh_draw(struct bvr_actor_s* mesh, int drawmode);
void bvr_static_mesh_update(struct bvr_actor_s* mesh);
void bvr_static_mesh_destroy(struct bvr_actor_s* mesh);
void bvr_static_mesh_serialize(struct bvr_actor_s* mesh, bvr_fhandle_t token);
void bvr_static_mesh_deserialize(struct bvr_actor_s* mesh, bvr_fhandle_t token);

#define BVR_STATIC_MESH(_, FIELD, METHOD)                   \ 
    FIELD(_, bvr_mesh_t, mesh)                              \
    FIELD(_, bvr_shader_t, shader)                          \
    FIELD(_, bvr_texture_t, texture)                        \
    METHOD(_, draw, bvr_static_mesh_draw)                   \
    METHOD(_, update, bvr_static_mesh_update)               \
    METHOD(_, destroy, bvr_static_mesh_destroy)             \
    METHOD(_, serialize, bvr_static_mesh_serialize)         \
    METHOD(_, deserialize, bvr_static_mesh_deserialize)    

BVR_DEFINE_ACTOR(bvr_static_mesh_t, BVR_STATIC_MESH)

#endif