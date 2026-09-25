#include <bvr/serialize.h>
#include <bvr/actors.h>
#include <bvr/io.h>

#include <json-c/json.h>

#define BVRI_MESH_PATH_TOKEN "path"
#define BVRI_MESH_ATTRIBS_TOKEN "path"
#define BVRI_MESH_VERTICES_TOKEN "vertices"
#define BVRI_MESH_ELEMENTS_TOKEN "elements"

#define BVRI_FHANDLE_PATH_TOKEN "path"

#define BVRI_TRANSFORM_POSITION_TOKEN "position"
#define BVRI_TRANSFORM_SCALE_TOKEN "scale"
#define BVRI_TRANSFORM_EULER_TOKEN "euler"
#define BVRI_TRANSFORM_ROTATION_TOKEN "rotation"

#define BVRI_CAMERA_MODE_TOKEN "mode"
#define BVRI_CAMERA_TRANSFORM_TOKEN "transform"
#define BVRI_CAMERA_WIDTH_TOKEN "width"
#define BVRI_CAMERA_HEIGHT_TOKEN "height"
#define BVRI_CAMERA_NEAR_TOKEN "near"
#define BVRI_CAMERA_FAR_TOKEN "far"
#define BVRI_CAMERA_FOV_TOKEN "fov"

#define BVRI_PAGE_SELF_TOKEN "self"
#define BVRI_PAGE_SELF_NAME_TOKEN "name"
#define BVRI_PAGE_SELF_ACTOR_LIST_TOKEN "actors"
#define BVRI_PAGE_CAMERA_TOKEN "camera"

#define BVRI_ACTOR_SELF_TRANSFORM_TOKEN "transform"
#define BVRI_ACTOR_SELF_TYPE_TOKEN "type"
#define BVRI_ACTOR_SELF_NAME_TOKEN "name"
#define BVRI_ACTOR_SELF_PARENT_TOKEN "parent"
#define BVRI_ACTOR_SELF_CHILDS_TOKEN "childs"
#define BVRI_ACTOR_SELF_FLAGS_TOKEN "flags"
#define BVRI_ACTOR_SELF_ORDER_IN_LAYER_TOKEN "order_in_layer"
#define BVRI_ACTOR_SELF_ACTIVE_TOKEN "active"

int bvr_deserialize_int32(bvr_fhandle_t token){
    if(json_object_is_type(token.token, json_type_int)){
        return json_object_get_int(token.token);
    }
    else if(json_object_is_type(token.token, json_type_double)){
        // fallback
        return (int32)json_object_get_double(token.token);
    }

    return 0;
}

long bvr_deserialize_int64(bvr_fhandle_t token){
    if(json_object_is_type(token.token, json_type_int)){
        return json_object_get_int64(token.token);
    } 
    else if(json_object_is_type(token.token, json_type_double)){
        // fallback
        return (int64)json_object_get_double(token.token);
    }

    return 0;
}

uint32 bvr_deserialize_uint32(bvr_fhandle_t token){
    if(json_object_is_type(token.token, json_type_int)){
        return (uint32)json_object_get_uint64(token.token);
    }
    else if(json_object_is_type(token.token, json_type_double)){
        // fallback
        return (uint32)json_object_get_double(token.token);
    }

    return 0;
}

uint64 bvr_deserialize_uint64(bvr_fhandle_t token){
    if(json_object_is_type(token.token, json_type_int)){
        return json_object_get_uint64(token.token);
    }
    else if(json_object_is_type(token.token, json_type_double)){
        // fallback
        return (uint64)json_object_get_double(token.token);
    }
    
    return 0;
}

float bvr_deserialize_float(bvr_fhandle_t token){
    if(json_object_is_type(token.token, json_type_double)){
        return (float)json_object_get_double(token.token);
    }
    else if(json_object_is_type(token.token, json_type_int)){
        // fallback
        return (float)json_object_get_int(token.token);
    }
    
    return 0.0f;
}

int bvr_deserialize_farray(bvr_fhandle_t token, float* array, uint32 length){
    if(array == NULL){
        return BVR_FALSE;
    }

    if(!json_object_is_type(token.token, json_type_array)){
        BVR_PRINT("corrupted float array");
        return BVR_FALSE;
    }
    
    if(json_object_array_length(token.token) >= length){
        for (size_t i = 0; i < length; i++)
        {
            array[i] = bvr_deserialize_float(
                BVR_TOKENIZE_JSON(json_object_array_get_idx(token.token, i))
            );
        }

        return BVR_TRUE;
    }

    return BVR_FALSE;
}

bool bvr_deserialize_bool(bvr_fhandle_t token){
    if(json_object_is_type(token.token, json_type_boolean)){
        return json_object_get_boolean(token.token);
    }
    return false;
}

void bvr_deserialize_string(bvr_fhandle_t token, bvr_string_t* string){
    BVR_ASSERT(string);

    if(json_object_is_type(token.token, json_type_string)){
        bvr_create_string(string, json_object_get_string(token.token));
        return;
    }

    bvr_create_string(string, NULL);
}

void bvr_deserialize_fhandle(bvr_fhandle_t token, bvr_fhandle_t* handle){
    BVR_ASSERT(handle);

    if(!json_object_is_type(token.token, json_type_object)){
        BVR_PRINT("corrupted fhandle");
        return;
    }

    json_object* fhandle_json_path = NULL;

    fhandle_json_path = json_object_object_get(token.token, BVRI_FHANDLE_PATH_TOKEN);
    if(!json_object_is_type(fhandle_json_path, json_type_string)){
        BVR_PRINT("corrupted fhandle");
        return;
    }

    *handle = bvr_create_fhandle(
        json_object_get_string(fhandle_json_path)
    );
}

int bvr_deserialize_transform(bvr_fhandle_t token, bvr_transform_t* transform){
    BVR_ASSERT(transform);

    if(!json_object_is_type(token.token, json_type_object)){
        BVR_PRINT("corrupted transform");
        return BVR_FALSE;
    }

    json_object* json_position = NULL;
    json_object* json_scale = NULL;
    json_object* json_rotation = NULL;
    json_object* json_euler = NULL;

    json_position = json_object_object_get(token.token, BVRI_TRANSFORM_POSITION_TOKEN);
    json_scale = json_object_object_get(token.token, BVRI_TRANSFORM_SCALE_TOKEN);
    json_rotation = json_object_object_get(token.token, BVRI_TRANSFORM_ROTATION_TOKEN);
    json_euler = json_object_object_get(token.token, BVRI_TRANSFORM_EULER_TOKEN);

    BVR_SET_VEC3(transform->position, 0.0f);
    BVR_SET_VEC3(transform->scale, 0.0f);
    BVR_SET_VEC4(transform->rotation, 0.0f);
    BVR_IDENTITY_MAT4(transform->local);
    BVR_IDENTITY_MAT4(transform->world);

    bvr_deserialize_vec3(BVR_TOKENIZE_JSON(json_position), transform->position);
    bvr_deserialize_vec3(BVR_TOKENIZE_JSON(json_scale), transform->scale);
    bvr_deserialize_vec4(BVR_TOKENIZE_JSON(json_rotation), transform->rotation);

    // check for euler angles
    if(json_object_is_type(json_euler, json_type_array)){
        vec3 euler;
        bvr_deserialize_vec3(BVR_TOKENIZE_JSON(json_euler), euler);

        // overwrite rotation with euler angles
        quat_euler_deg(transform->rotation,  euler[0], euler[1], euler[2]);
    }

    return json_position && json_scale && json_rotation;
}

int bvr_deserialize_mesh(bvr_fhandle_t token, bvr_mesh_t* mesh){
    BVR_ASSERT(mesh);
    
    if(!json_object_is_type(token.token, json_type_object)){
        BVR_PRINT("corrupted mesh");
        return BVR_FALSE;
    }

    json_object* json_mesh_path = NULL;
    json_object* json_mesh_vertices = NULL;
    json_object* json_mesh_elements = NULL;
    json_object* json_mesh_attributes = NULL;

    json_mesh_attributes = json_object_object_get(token.token, BVRI_MESH_ATTRIBS_TOKEN);
    if(!json_object_is_type(json_mesh_attributes, json_type_int)){
        BVR_PRINT("corrupted mesh");
        return BVR_FALSE;
    }
    
    json_mesh_path = json_object_object_get(token.token, BVRI_MESH_PATH_TOKEN);
    if(json_mesh_path){
        // loading through path
        if(bvr_create_mesh(mesh,
            json_object_get_string(json_mesh_path),
            json_object_get_int64(json_mesh_attributes))){
            
            // path loading succeed
            return BVR_TRUE;
        }

        // if we couldn't load through the path
        // we try to fallback on raw loading
    }

    // load through raw data
    bvr_mesh_buffer_t vertices;
    bvr_mesh_buffer_t elements;
    
    json_mesh_vertices = json_object_object_get(token.token, BVRI_MESH_VERTICES_TOKEN);
    json_mesh_elements = json_object_object_get(token.token, BVRI_MESH_ELEMENTS_TOKEN);
    if(!json_object_is_type(json_mesh_vertices, json_type_array) || 
        json_object_is_type(json_mesh_path, json_type_array)){
        
        BVR_PRINT("corrupted mesh");
        return BVR_FALSE;
    }

    vertices.count = 0;
    elements.count = 0;
    vertices.data = NULL;
    elements.data = NULL;

    if(json_object_array_length(json_mesh_vertices)){
        // get vertices
        vertices.count = json_object_array_length(json_mesh_vertices);

        // in every case, the max size is sizeof(float)
        vertices.data = malloc(vertices.count * sizeof(float));
        
        // check for int or float
        if(json_object_is_type(json_object_array_get_idx(json_mesh_vertices, 0), json_type_int)){
            // vertices are int
            vertices.type = BVR_INT32;
        }
        else if(json_object_is_type(json_object_array_get_idx(json_mesh_vertices, 0), json_type_double)){
            vertices.type = BVR_FLOAT;
        }
        else {
            BVR_PRINT("invalid mesh vertex type");
            
            free(vertices.data);
            return BVR_FALSE;
        }
    }

    if(json_object_array_length(json_mesh_elements)){
        // get indices
        elements.count = json_object_array_length(json_mesh_elements);

        // in every case, the max size is sizeof(float)
        elements.data = malloc(vertices.count * sizeof(float));
        
        // check for int or float
        if(json_object_is_type(json_object_array_get_idx(json_mesh_elements, 0), json_type_int)){
            // vertices are int
            elements.type = BVR_INT32;
        }
        else if(json_object_is_type(json_object_array_get_idx(json_mesh_elements, 0), json_type_double)){
            elements.type = BVR_FLOAT;
        }
        else {
            BVR_PRINT("invalid mesh element type");
            
            free(elements.data);
            return BVR_FALSE;
        }
    }

    int status = bvr_create_meshv(mesh, 
        &vertices, &elements, 
        json_object_get_int64(json_mesh_attributes)
    );

    free(vertices.data);
    free(elements.data);
}

int bvr_deserialize_shader(bvr_fhandle_t token, bvr_shader_t* shader){
    BVR_ASSERT(shader);

    if(!json_object_is_type(token.token, json_type_object)){
        BVR_PRINT("corrupted shader");
        return BVR_FALSE;
    }

    json_object* json_shader_path = NULL;
    json_object* json_shader_flags = NULL;
    json_object* json_shader_uniforms = NULL;
    json_object* json_shader_textures = NULL;
}

int bvr_deserialize_camera(bvr_fhandle_t token, bvr_camera_t* camera){
    BVR_ASSERT(camera);

    if(!json_object_is_type(token.token, json_type_object)){
        BVR_PRINT("corrupted camera");
        return BVR_FALSE;
    }

    json_object* json_camera_mode = NULL;
    json_object* json_camera_transform = NULL;
    json_object* json_camera_width = NULL;
    json_object* json_camera_height = NULL;
    json_object* json_camera_near = NULL;
    json_object* json_camera_far = NULL;
    json_object* json_camera_fov = NULL;

    json_camera_mode = json_object_object_get(token.token, BVRI_CAMERA_MODE_TOKEN);
    json_camera_transform = json_object_object_get(token.token, BVRI_CAMERA_TRANSFORM_TOKEN);
    json_camera_width = json_object_object_get(token.token, BVRI_CAMERA_WIDTH_TOKEN);
    json_camera_height = json_object_object_get(token.token, BVRI_CAMERA_HEIGHT_TOKEN);
    json_camera_near = json_object_object_get(token.token, BVRI_CAMERA_NEAR_TOKEN);
    json_camera_far = json_object_object_get(token.token, BVRI_CAMERA_FAR_TOKEN);
    json_camera_fov = json_object_object_get(token.token, BVRI_CAMERA_FOV_TOKEN);    

    int mode = bvr_deserialize_float(BVR_TOKENIZE_JSON(json_camera_mode));
    if(mode == BVR_CAMERA_ORTHOGRAPHIC){
        bvr_create_ortho_camera(
            camera,
            bvr_deserialize_float(BVR_TOKENIZE_JSON(json_camera_width)),
            bvr_deserialize_float(BVR_TOKENIZE_JSON(json_camera_height)),
            bvr_deserialize_float(BVR_TOKENIZE_JSON(json_camera_near)),
            bvr_deserialize_float(BVR_TOKENIZE_JSON(json_camera_far)),
            bvr_deserialize_float(BVR_TOKENIZE_JSON(json_camera_fov))
        );
    }
    else {
        bvr_create_persp_camera(
            camera,
            bvr_deserialize_float(BVR_TOKENIZE_JSON(json_camera_width)),
            bvr_deserialize_float(BVR_TOKENIZE_JSON(json_camera_height)),
            bvr_deserialize_float(BVR_TOKENIZE_JSON(json_camera_near)),
            bvr_deserialize_float(BVR_TOKENIZE_JSON(json_camera_far)),
            bvr_deserialize_float(BVR_TOKENIZE_JSON(json_camera_fov))
        );
    }

    bvr_deserialize_transform(
        BVR_TOKENIZE_JSON(json_camera_transform), 
        &camera->ortho.transform
    );

    return BVR_TRUE;
}

int bvr_deserialize_page(bvr_fhandle_t token, bvr_page_t* page){
    BVR_ASSERT(page);

    if(!json_object_is_type(token.token, json_type_object)){
        BVR_PRINT("corrupted page");
        return BVR_FALSE;
    }

    json_object* json_self = NULL;
    json_object* json_camera = NULL;
    json_object* json_actor = NULL;
    json_object* json_actor_type = NULL;
    json_object* json_actor_name = NULL;
    json_object* json_actor_transform = NULL;
    json_object* json_actor_parent = NULL;
    json_object* json_actor_childs = NULL;
    json_object* json_actor_flags = NULL;
    json_object* json_actor_order_in_layer = NULL;
    json_object* json_actor_active = NULL;
    json_object* json_actor_field = NULL;
    json_object* json_actor_list = NULL;

    json_self = json_object_object_get(token.token, BVRI_PAGE_SELF_TOKEN);
    json_camera = json_object_object_get(token.token, BVRI_PAGE_CAMERA_TOKEN);
    json_actor_list = json_object_object_get(token.token, BVRI_PAGE_SELF_ACTOR_LIST_TOKEN);

    // do the self object
    if(json_object_is_type(json_self, json_type_object)){
        // bvr_create_string(&page->name, name);
        bvr_deserialize_string(
            BVR_TOKENIZE_JSON(json_object_object_get(json_self, BVRI_PAGE_SELF_NAME_TOKEN)),
            &page->name
        );

    }

    // do the camera
    bvr_deserialize_camera(BVR_TOKENIZE_JSON(json_camera), &page->camera);

    if(json_object_is_type(json_actor_list, json_type_array)){
        bvr_create_table(
            &page->actors, 
            sizeof(struct bvr_actor_s*), 
            MAX(1, json_object_array_length(json_actor_list)) * BVR_GROWTH_FACTOR
        );

        for (size_t i = 0; i < json_object_array_length(json_actor_list); i++)
        {
            json_actor = json_object_array_get_idx(json_actor_list, i);
            
            if(!json_object_is_type(json_actor, json_type_object)){
                // invalid actor, go to the next one
                BVR_PRINTF("invalid actor at index %i", i);
                continue;
            }

            json_actor_type = json_object_object_get(json_actor, BVRI_ACTOR_SELF_TYPE_TOKEN);
            json_actor_name = json_object_object_get(json_actor, BVRI_ACTOR_SELF_NAME_TOKEN);
            json_actor_transform = json_object_object_get(json_actor, BVRI_ACTOR_SELF_TRANSFORM_TOKEN);
            json_actor_parent = json_object_object_get(json_actor, BVRI_ACTOR_SELF_PARENT_TOKEN);
            json_actor_childs = json_object_object_get(json_actor, BVRI_ACTOR_SELF_CHILDS_TOKEN);
            json_actor_flags = json_object_object_get(json_actor, BVRI_ACTOR_SELF_FLAGS_TOKEN);
            json_actor_order_in_layer = json_object_object_get(json_actor, BVRI_ACTOR_SELF_ORDER_IN_LAYER_TOKEN);
            json_actor_active = json_object_object_get(json_actor, BVRI_ACTOR_SELF_ACTIVE_TOKEN);
        
            if(!json_object_is_type(json_actor_type, json_type_string)){
                // invalid type
                BVR_PRINTF("invalid actor type at index %i", i);
                continue;
            }

            if(!json_object_is_type(json_actor_name, json_type_string)){
                // invalid type
                BVR_PRINTF("invalid actor name at index %i", i);
                continue;
            }

            const struct bvr_actor_vtable_s* vtable = bvr_actor_get_vtable(
                json_object_get_string(json_actor_type)
            );

            if(vtable == NULL){
                // invalid type
                BVR_PRINTF("invalid actor type at index %i", i);
                continue;
            }

            // allocate a new actor
            struct bvr_actor_s* actor = bvr_alloc_actor(page, 
                json_object_get_string(json_actor_name),
                vtable->class_size
            );
            BVR_ASSERT(actor);

            // deserialize common actor fields
            bvr_deserialize_transform(BVR_TOKENIZE_JSON(json_actor_transform), &actor->transform);
            actor->flags = bvr_deserialize_uint32(BVR_TOKENIZE_JSON(json_actor_flags));
            actor->order_in_layer = bvr_deserialize_uint32(BVR_TOKENIZE_JSON(json_actor_order_in_layer));
            actor->active = (uint16) bvr_deserialize_bool(BVR_TOKENIZE_JSON(json_actor_active));
            actor->vtable = (struct bvr_actor_vtable_s*) vtable;
            
            for (size_t f = 0; f < vtable->field_count; f++)
            {
                if(vtable->ftable[f].offset < sizeof(struct bvr_actor_s)){
                    // no op for common actor fields
                    continue;
                }

                json_actor_field = json_object_object_get(json_actor, vtable->ftable[f].name);
                if(json_object_is_type(json_actor_field, json_type_null)){
                    BVR_PRINTF("invalid actor field '%s'", vtable->ftable[f].name);
                    continue;
                }

                BVR_PRINTF("field %s %i", vtable->ftable[f].name, vtable->ftable[f].offset);
            }
            
        }
    }
    else {
        // cannot read actor list
        BVR_PRINT("missing actor list!");
    }
}