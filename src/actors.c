#include <bvr/actors.h>

#include <bvr/book.h>
#include <bvr/graphics.h>

#include <bvr/landscape.h>
#include <bvr/gl.h>

#include <stdlib.h>
#include <math.h>
#include <memory.h>

// actor classes registry
static struct {
    char name[128];
    bool used;

    struct bvr_actor_vtable_s* table;
} __actor_classes_table[BVR_MAX_ACTOR_CLASSES];

static int bvri_abstract_draw(struct bvr_actor_s* actor, int drawmode, bvr_mesh_t* mesh, bvr_shader_t* shader){
    BVR_ASSERT(actor);
    BVR_ASSERT(mesh);
    BVR_ASSERT(shader);

    // update actor's tranform
    bvr_shader_set_uniform_raw(&shader->uniforms[0], BVR_CREATE_FIELD_PHANDLE(bvr_static_mesh_t, actor, "world"));

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

void bvr_actor_serializable(const char* cname, struct bvr_actor_vtable_s* table){
    BVR_ASSERT(cname);
    BVR_ASSERT(table);

    for (size_t i = 0; i < BVR_MAX_ACTOR_CLASSES; i++)
    {
        if(__actor_classes_table[i].used == 0){
            // available class slot
            BVR_STRCPY(
                __actor_classes_table[i].name, cname,
                MIN(strlen(cname), sizeof(__actor_classes_table[i].name))
            );

            __actor_classes_table[i].table = table;
            __actor_classes_table[i].used = true;
            return;
        }
    }
    
    BVR_ASSERT(0 || "maximum serializable class reached!");
}

struct bvr_actor_fields_s* bvr_actor_get_field(const char* cname, const char* fname){
    BVR_ASSERT(cname);
    BVR_ASSERT(fname);

    for (size_t i = 0; i < BVR_MAX_ACTOR_CLASSES; i++)
    {
        if(__actor_classes_table[i].used
            && BVR_STRCMP(cname, __actor_classes_table[i].name)){
            
            struct bvr_actor_vtable_s* t = __actor_classes_table[i].table;
            for (size_t y = 0; y < __actor_classes_table[i].table->field_count; y++)
            {
                if(BVR_STRCMP(__actor_classes_table[i].table->ftable[y].name, fname)){
                    // field found
                    return &__actor_classes_table[i].table->ftable[y];
                }
            }
            
            // cannot find field if the specified actor
            return NULL;
        }
    }
    
    return NULL;
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

BVR_IMPLEMENT_ACTOR(bvr_static_mesh_t, BVR_STATIC_MESH, true)

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

void bvr_static_mesh_serialize(struct bvr_actor_s* mesh, bvr_fhandle_t token){

}

void bvr_static_mesh_deserialize(struct bvr_actor_s* mesh, bvr_fhandle_t token){

}