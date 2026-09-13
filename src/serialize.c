#include <bvr/serialize.h>
#include <bvr/io.h>

#include <json-c/json.h>

#define BVRI_MESH_PATH_TOKEN "path"
#define BVRI_MESH_ATTRIBS_TOKEN "path"
#define BVRI_MESH_VERTICES_TOKEN "vertices"
#define BVRI_MESH_ELEMENTS_TOKEN "elements"

int bvr_deserialize_int32(bvr_fhandle_t token){
    if(json_object_is_type(token.token, json_type_int)){
        return json_object_get_int(token.token);
    }
    return 0;
}

long bvr_deserialize_int64(bvr_fhandle_t token){
    if(json_object_is_type(token.token, json_type_int)){
        return json_object_get_int64(token.token);
    } 
    return 0;
}

uint32 bvr_deserialize_uint32(bvr_fhandle_t token){
    if(json_object_is_type(token.token, json_type_int)){
        return (uint32)json_object_get_uint64(token.token);
    }
    return 0;
}

uint64 bvr_deserialize_uint64(bvr_fhandle_t token){
    if(json_object_is_type(token.token, json_type_int)){
        return json_object_get_uint64(token.token);
    }
    return 0;
}

float bvr_deserialize_float(bvr_fhandle_t token){
    if(json_object_is_type(token.token, json_type_double)){
        return (float)json_object_get_double(token.token);
    }
    return 0.0f;
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

    return bvr_create_meshv(mesh, 
        &vertices, &elements, 
        json_object_get_int64(json_mesh_attributes)
    );
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