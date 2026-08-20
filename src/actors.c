#include <bvr/actors.h>

#include <bvr/book.h>
#include <bvr/graphics.h>

#include <bvr/landscape.h>
#include <bvr/gl.h>

#include <stdlib.h>
#include <math.h>
#include <memory.h>

static int bvri_abstract_draw(struct bvr_actor_s* actor, int drawmode, bvr_mesh_t* mesh, bvr_shader_t* shader){
    BVR_ASSERT(actor);
    BVR_ASSERT(mesh);
    BVR_ASSERT(shader);

    // update actor's tranform
    bvr_shader_set_uniform_raw(&shader->uniforms[0], actor->transform.world);

    struct bvr_draw_command_s cmd;
    cmd.order = actor->order_in_layer;
    cmd.array_buffer = mesh->array_buffer;
    cmd.vertex_buffer = mesh->vertex_buffer;
    cmd.element_buffer = mesh->element_buffer;
    cmd.attrib_count = mesh->attrib_count;
    cmd.element_type = mesh->element_type;

    cmd.shader = shader;
    cmd.draw_mode = drawmode;

    bvr_vertex_group_t* group;
    BVR_POOL_FOR_EACH(mesh->vertex_groups, group){
        cmd.vertex_group = *group;

        bvr_pipeline_add_draw_cmd(&cmd);
    }
}

static void bvri_abstract_calc_transform(struct bvr_actor_s* actor){
    BVR_ASSERT(actor);

    mat4x4 rotation;

    BVR_IDENTITY_MAT4(rotation);
    BVR_IDENTITY_MAT4(actor->transform.local);
    BVR_IDENTITY_MAT4(actor->transform.world);

    mat4_rotate(rotation, actor->transform.rotation);

    // copy translation
    actor->transform.local[3][0] = actor->transform.position[0];
    actor->transform.local[3][1] = actor->transform.position[1];
    actor->transform.local[3][2] = actor->transform.position[2];

    // scale matrix
    actor->transform.local[0][0] = actor->transform.scale[0];
    actor->transform.local[1][1] = actor->transform.scale[0];
    actor->transform.local[2][2] = actor->transform.scale[0];

    // final local matrix
    mat4_mul(actor->transform.local, actor->transform.local, rotation);
    
    // copy value to the world matrix
    mat4_copy(actor->transform.world, actor->transform.local);

    // here, we got to each parent and mult there own world matrices
    // TODO
}

void bvr_static_mesh_draw(struct bvr_actor_s* self, int drawmode){
    bvr_static_mesh_t* sm = (bvr_static_mesh_t*)self;
    BVR_ASSERT(sm);
    
    bvri_abstract_calc_transform(self);
    bvri_abstract_draw(self, drawmode, &sm->mesh, &sm->shader);
}

void bvr_static_mesh_update(struct bvr_actor_s* self){

}

void bvr_static_mesh_destroy(struct bvr_actor_s* self){
    bvr_static_mesh_t* sm = (bvr_static_mesh_t*)self;
    BVR_ASSERT(sm);

    bvr_destroy_mesh(&sm->mesh);
    bvr_destroy_shader(&sm->shader);
    bvr_destroy_image(&sm->image);
}

void bvr_dynamic_mesh_draw(struct bvr_actor_s* self, int drawmode){
    bvr_dynamic_mesh_t* dm = (bvr_dynamic_mesh_t*)self;
    BVR_ASSERT(dm);
    
    bvri_abstract_calc_transform(self);
    bvri_abstract_draw(self, drawmode, &dm->mesh, &dm->shader);
}

void bvr_dynamic_mesh_update(struct bvr_actor_s* self){

}

void bvr_dynamic_mesh_destroy(struct bvr_actor_s* self){
    bvr_dynamic_mesh_t* dm = (bvr_dynamic_mesh_t*)self;
    BVR_ASSERT(dm);

    bvr_destroy_mesh(&dm->mesh);
    bvr_destroy_shader(&dm->shader);
    bvr_destroy_image(&dm->image);
}