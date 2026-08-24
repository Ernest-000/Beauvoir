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

static void bvri_abstract_destroy(struct bvr_actor_s* actor){
    BVR_ASSERT(actor);

    // clear the string
    bvr_destroy_string(&actor->name);

    // clear the children array
    free(actor->childs);
    actor->childs = NULL;
    actor->child_slots = 0;
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
    struct bvr_actor_s* parent = actor;
    while (parent->parent)
    {
        // previous actor in hierachy
        parent = parent->parent;
        mat4_mul(actor->transform.world, actor->transform.world, parent->transform.world);
    }
    
}

static void bvri_clear_parent(struct bvr_actor_s* actor, struct bvr_actor_s* parent){
    BVR_ASSERT(actor);
    BVR_ASSERT(parent);

    // nothing to do
    if(actor->parent == NULL){
        return;
    }

    // nothing to do
    if(parent->children_count == 0){
        return;
    }

    actor->parent = NULL;
    
    // find 'actor' in the children list
    for (size_t i = 0; i < parent->child_slots; i++)
    {
        if(parent->childs[i] == actor){
            // found
            parent->childs[i] = NULL;
            break;
        }
    }
    
    parent->children_count--;
}

void bvr_actor_set_parent(struct bvr_actor_s* actor, struct bvr_actor_s* parent){
    BVR_ASSERT(actor);

    if(parent == NULL){
        return;
    }

    if(actor->parent){
        bvri_clear_parent(actor, actor->parent);
    }

    struct bvr_actor_s** pp_actor = NULL;
    
    if(parent->child_slots != parent->children_count){
        // try to find an available chilren slot
        for (size_t i = 0; i < parent->child_slots; i++)
        {
            if(parent->childs[i] == NULL){
                // found available slot
                pp_actor = &parent->childs[i];
            }
        }
    }
    
    if(pp_actor == NULL){
        // when no slot, alloc
        parent->child_slots++;
        parent->childs = realloc(parent->childs, parent->child_slots * sizeof(struct bvr_actor_s*));
        BVR_ASSERT(parent->childs);

        pp_actor = &parent->childs[parent->child_slots - 1];
    }
    

    // set children
    *pp_actor = actor;
    parent->children_count++;

    // set parent
    actor->parent = parent;
    
    return;
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
    bvr_destroy_texture(&sm->texture);
    
    bvri_abstract_destroy(self);
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
    bvr_destroy_texture(&dm->texture);

    bvri_abstract_destroy(self);
}