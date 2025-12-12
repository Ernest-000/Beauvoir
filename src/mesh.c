#include <BVR/mesh.h>
#include <BVR/math.h>
#include <BVR/buffer.h>
#include <BVR/file.h>
#include <BVR/physics.h>

#include <malloc.h>
#include <string.h>
#include <memory.h>

#include <GLAD/glad.h>

#ifndef BVR_NO_OBJ
    #include <ctype.h>
#endif

#ifndef BVR_NO_GLTF
    #include <json-c/json.h>
#endif

/*
    Generic function tp allocate and create ogl buffers.
    This function is used when creating buffers so that if any anything is needed to be change globally,
    only this function's content need to me changed.
*/
static int bvri_create_mesh_buffers(bvr_mesh_t* mesh, uint64 vertices_size, uint64 element_size, 
    int vertex_type, int element_type, bvr_mesh_array_attrib_t attrib);

#ifndef BVR_NO_OBJ

struct bvri_objobject_s {
    bvr_string_t name;
    bvr_string_t material;

    vec3 vertex[BVR_BUFFER_SIZE];
    vec2 uvs[BVR_BUFFER_SIZE];
    vec3 normals[BVR_BUFFER_SIZE];
    struct {
        uint32 vertex[4];
        uint32 uv[4];
        uint32 normal[4];

        uint8 edges;
    } faces[BVR_BUFFER_SIZE];

    bvr_mesh_buffer_t vertices;
    bvr_mesh_buffer_t elements;

    bvr_vertex_group_t groups[BVR_BUFFER_SIZE / 2];
    bvr_vertex_group_t* group;

    uint16 vertex_count;
    uint16 uv_count;
    uint16 normal_count;
    uint16 face_count;
    uint16 group_count;
};


static int bvri_objreadline(char* buffer, FILE* file);
static char* bvri_objparseint(char* buffer, int* v);
static char* bvri_objparsefloat(char* buffer, float* v);

static int bvri_is_obj(FILE* file){
    fseek(file, 0, SEEK_SET);

    char sig[32];
    uint8 max = 1;

    // check 5 first lines
    for (uint64 i = 0; i < max; i++)
    {
        bvri_objreadline(sig, file);

        if(sig[0] == '#'){
            max++;
            continue;
        }

        if(!strncmp(sig, "mtllib ", 7)){
            return BVR_TRUE;
        }

        if(sig[0] == 'o'){
            return BVR_TRUE;
        }
    }
    
    return BVR_FALSE;
}

static int bvri_load_obj(bvr_mesh_t* mesh, FILE* file){
    BVR_ASSERT(mesh);

    struct bvri_objobject_s object;

    object.vertex_count = 0;
    object.uv_count = 0;
    object.normal_count = 0;
    object.face_count = 0;
    object.group_count = 0;
    object.group = &object.groups[0];

    object.elements.data = NULL;
    object.vertices.data = NULL;
    object.elements.count = 0;
    object.vertices.count = 0;
    object.elements.type = BVR_UNSIGNED_INT32;
    object.vertices.type = BVR_FLOAT;

    // WARN: find a way to find stride
    mesh->attrib = BVR_MESH_ATTRIB_V3UV2N3;
    mesh->stride = mesh->attrib * sizeof(float);
    bvr_create_string(&object.name, NULL);
    bvr_create_string(&object.material, NULL);

    object.group->flags = 0;
    object.group->texture = 0;
    object.group->element_count = 0;
    object.group->element_offset = 0;
    bvr_create_string(&object.group->name, NULL);

    char* cursor;
    char buffer[256];
    while(bvri_objreadline(buffer, file)){
        switch (buffer[0])
        {
        case '#':
            /* no-op */
            break;

        case 's':
            /* no-op */
            break;
        
        case 'm':
            bvr_overwrite_string(&object.material, &buffer[7], 0);
            break;    

        case 'o':
            BVR_ASSERT(object.group_count < BVR_BUFFER_SIZE / 2);
            object.group = &object.groups[++object.group_count - 1];

            object.group->flags = 0;
            object.group->texture = 0;
            object.group->element_count = 0;
            object.group->element_offset = object.elements.count;

            bvr_create_string(&object.group->name, &buffer[2]);
            break;

        case 'v':
            {
                cursor = NULL;

                if(buffer[1] == 'n' && mesh->attrib >= BVR_MESH_ATTRIB_V3UV2N3){
                    BVR_ASSERT(object.normal_count < BVR_BUFFER_SIZE);

                    cursor = bvri_objparsefloat(&buffer[3], &object.normals[object.normal_count][0]);
                    cursor = bvri_objparsefloat(++cursor, &object.normals[object.normal_count][1]);
                    cursor = bvri_objparsefloat(++cursor, &object.normals[object.normal_count][2]);
                    object.normal_count++;
                }
                else if(buffer[1] == 't' && mesh->attrib >= BVR_MESH_ATTRIB_V3UV2){
                    BVR_ASSERT(object.uv_count < BVR_BUFFER_SIZE);
                    
                    cursor = bvri_objparsefloat(&buffer[3], &object.uvs[object.uv_count][0]);
                    cursor = bvri_objparsefloat(++cursor, &object.uvs[object.uv_count][1]);
                    object.uv_count++;
                }
                else if(buffer[1] == ' '){
                    BVR_ASSERT(object.vertex_count < BVR_BUFFER_SIZE);

                    cursor = bvri_objparsefloat(&buffer[2], &object.vertex[object.vertex_count][0]);
                    cursor = bvri_objparsefloat(++cursor, &object.vertex[object.vertex_count][1]);
                    cursor = bvri_objparsefloat(++cursor, &object.vertex[object.vertex_count][2]);
                    object.vertex_count++;
                }
            }   
            break;
        
        case 'f': 
            {
                BVR_ASSERT(object.face_count < BVR_BUFFER_SIZE);

                cursor = &buffer[1];
                object.faces[object.face_count].edges = 0;
                
                while (*cursor != '\0')
                {
                    cursor = bvri_objparseint(++cursor, &object.faces[object.face_count].vertex[object.faces[object.face_count].edges]);    
                    cursor = bvri_objparseint(++cursor, &object.faces[object.face_count].uv[object.faces[object.face_count].edges]);    
                    cursor = bvri_objparseint(++cursor, &object.faces[object.face_count].normal[object.faces[object.face_count].edges]);    

                    object.faces[object.face_count].edges++;
                    object.vertices.count += mesh->stride;
                }
                
                if(object.faces[object.face_count].edges == 4){
                    object.elements.count += 6;
                    object.group->element_count += 6;
                }
                else {
                    object.elements.count += 3;
                    object.group->element_count += 3;
                }

                object.face_count++;
            }
            break;

        default:
            break;
        }
    }

    // if the mesh is empty
    if(!object.elements.count || !object.vertices.count){
        BVR_PRINT("failed to load mesh :(");
        goto bvr_objfailed;
    }

    // create opengl buffers
    if(bvri_create_mesh_buffers(mesh, 
        object.vertices.count * sizeof(float), 
        object.elements.count * sizeof(uint32),
        object.vertices.type, object.elements.type, mesh->attrib) == BVR_FALSE){

        BVR_PRINT("failed to allocate object buffers");
        goto bvr_objfailed;
    }

    // create vertex groups
    // copy created groups
    bvr_create_pool(&mesh->vertex_groups, sizeof(bvr_vertex_group_t), object.group_count);
    for (size_t i = 0; i < object.group_count; i++)
    {
        bvr_vertex_group_t* group = bvr_pool_alloc(&mesh->vertex_groups);
        group->name.length = object.groups[i].name.length;
        group->name.string = object.groups[i].name.string;
        group->element_count = object.groups[i].element_count;
        group->element_offset = object.groups[i].element_offset;
        group->texture = 0;

        BVR_IDENTITY_MAT4(group->matrix);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);

    // copy data to opengl buffers
    uint64 vertex = 0, element = 0;
    for (uint64 face = 0; face < object.face_count; face++)
    {
        BVR_ASSERT(object.faces[face].edges == 3 || object.faces[face].edges == 4);

        if(object.faces[face].edges == 4){
            BVR_ASSERT(0 || "4 edges faces are not supported");
        }
        else {
            BVR_ASSERT(element + 3 <= object.elements.count);

            const uint32 elements_values[3] = {element, element + 1, element + 2};
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, element * sizeof(uint32), sizeof(elements_values), elements_values);
            element += 3;
        }

        for (uint64 i = 0; i < object.faces[face].edges; i++)
        {
            BVR_ASSERT(vertex + BVR_MESH_ATTRIB_V3UV2N3 <= object.vertices.count);

            glBufferSubData(GL_ARRAY_BUFFER, (vertex + 0) * sizeof(float), sizeof(vec3), object.vertex[object.faces[face].vertex[i] - 1]);
            glBufferSubData(GL_ARRAY_BUFFER, (vertex + 3) * sizeof(float), sizeof(vec2), object.uvs[object.faces[face].uv[i] - 1]);
            glBufferSubData(GL_ARRAY_BUFFER, (vertex + 5) * sizeof(float), sizeof(vec3), object.normals[object.faces[face].normal[i] - 1]);
            vertex += BVR_MESH_ATTRIB_V3UV2N3;
        }
        
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    bvr_destroy_string(&object.name);
    bvr_destroy_string(&object.material);

    free(object.vertices.data);
    free(object.elements.data);

    return BVR_TRUE;

// called when obj's loading failed
bvr_objfailed:
    bvr_destroy_string(&object.name);
    bvr_destroy_string(&object.material);

    // free all allocated strings
    for (uint64 i = 0; i < object.group_count; i++)
    {
        bvr_destroy_string(&object.groups[i].name);
    }
    
    free(object.vertices.data);
    free(object.elements.data);

    return BVR_FALSE;
}

static int bvri_objreadline(char* buffer, FILE* file){
    int c;
    while (c = getc(file), c != EOF && c != '\n') {
        *buffer++ = c;
    }

    *buffer = '\0';

    return c != EOF;
}

static char* bvri_objparseint(char* buffer, int* v){
    int sig = + 1;
    int num = 0;

    if(*buffer == '-'){
        sig = -1;
        buffer++;
    }

    while (isdigit(*buffer))
    {
        num = 10 * num + (*buffer++ - '0');
    }
    
    *v = sig * num;

    return buffer;
}

static char* bvri_objparsefloat(char* buffer, float* v){
    double sig = +1, p = +1;
    double num = 0.0;
    double fra = 0.0;
    double div = 1.0;
    uint32 e = 0;

    if(*buffer == '-'){
        sig = -1;
        buffer++;
    }
    else if(*buffer == '+'){
        sig = +1;
        buffer++;
    }

    while (isdigit(*buffer))
    {
        num = 10 * num + (double)(*buffer++ - '0');
    }
    
    if(*buffer == '.'){
        buffer++;
    }

    while (isdigit(*buffer))
    {
        fra = 10.0 * fra + (double)(*buffer++ - '0');
        div *= 10.0;
    }

    num += fra / div;
    
    // powers are not supported :<
    if(*buffer == 'E' || *buffer == 'e'){
        BVR_PRINT("warning powers are not supported!");
    }

    *v = (float)sig * num;

    return buffer;
}


#endif

#ifndef BVR_NO_GLTF 

struct bvri_gltfchunk {
    uint32 length;
    uint32 sig;
    size_t offset;

    void* data;
};

struct bvri_gltfobject {
    struct bvri_gltfchunk* json;
    struct bvri_gltfchunk* binary;
    
    bvr_mesh_buffer_t vertices;
    bvr_mesh_buffer_t elements;

    json_object* json_nodes;
    json_object* json_meshes;
    json_object* json_accessors;
    json_object* json_bufferviews;
    json_object* json_buffers;

    float scale;
};

static int bvri_gltfsizeof(const json_object* component, const json_object* type, int* scalar);
static int bvri_gltfpushbackattribute(struct bvri_gltfobject* object, json_object* target, uint32_t offset, const uint32 stride, const uint32 tbuffer);
static void bvri_gltfhandletransform(struct bvri_gltfobject* object, const json_object* node, bvr_vertex_group_t* group);
static void bvri_gltfhandlescale(struct bvri_gltfobject* object, json_object* target);

static int bvri_is_gltf(FILE* file){
    fseek(file, 0, SEEK_SET);

    // check for magic number
    int magic = bvr_freadu32_be(file);
    return magic == 0x676C5446;
}

static int bvri_load_gltf(bvr_mesh_t* mesh, FILE* file){
    fseek(file, 0, SEEK_SET);

    // get header informations
    int magic = bvr_freadu32_be(file);
    int version = bvr_freadu32_le(file);
    int file_length = bvr_freadu32_le(file);

    struct bvri_gltfchunk json_section, bin_section;
    struct bvri_gltfobject object;

    object.json = &json_section;
    object.binary = &bin_section;

    object.vertices.count = 0;
    object.vertices.data = NULL;
    object.vertices.type = BVR_FLOAT;
    object.elements.count = 0;
    object.elements.data = NULL;
    object.elements.type = BVR_FLOAT;

    object.scale = 1.0f;

    // json section;
    json_section.length = bvr_freadu32_le(file);
    json_section.sig = bvr_freadu32_be(file);
    json_section.data = NULL;
    
    // compare section sig to JSON signature (4A 53 4F 4E)
    if(json_section.sig == 0x4A534F4E){
        json_section.offset = ftell(file);
    }
    else {
        BVR_PRINT("failed to locate json gltf chunk");
        return BVR_FALSE;
    }

    // bin section;
    fseek(file, json_section.length, SEEK_CUR);

    bin_section.length = bvr_freadu32_le(file);
    bin_section.sig = bvr_freadu32_be(file);
    bin_section.data = NULL;

    // compare section sig to BIN signature (42 49 4E 00)
    if(bin_section.sig == 0x42494E00){
        bin_section.offset = ftell(file);
    }
    else {
        BVR_PRINT("failed to locate binary gltf chunk");
        return BVR_FALSE;
    }

    // read json section and create json context
    {
        json_tokener* json_tok = json_tokener_new();

        fseek(file, json_section.offset, SEEK_SET);

        char* json_content = malloc(json_section.length);
        fread(json_content, sizeof(char), json_section.length, file);

        json_section.data = json_tokener_parse_ex(json_tok, json_content, json_section.length);
        free(json_content);
    }

    object.json_nodes = json_object_object_get((json_object*) json_section.data, "nodes");
    object.json_meshes = json_object_object_get((json_object*) json_section.data, "meshes");
    object.json_accessors = json_object_object_get((json_object*) json_section.data, "accessors");
    object.json_bufferviews = json_object_object_get((json_object*) json_section.data, "bufferViews");
    object.json_buffers = json_object_object_get((json_object*) json_section.data, "buffers");

    // check for objects assignments
    if(!(json_section.data && object.json_meshes && object.json_accessors && object.json_bufferviews && object.json_buffers)){
        json_object_put((json_object*) json_section.data);

        BVR_PRINT("corrupted or missing gdb json!");
        return BVR_FALSE;
    }

    json_object* json_node = NULL;
    json_object* json_mesh = NULL;
    json_object* json_pritimive = NULL;
    json_object* json_accessor = NULL;
    json_object* json_attibutes = NULL;

    // create ressources
    for (size_t i = 0; i < json_object_array_length(object.json_meshes); i++)
    {
        json_mesh = json_object_array_get_idx(object.json_meshes, i);
        json_pritimive = json_object_object_get(json_mesh, "primitives");

        for (size_t p = 0; p < json_object_array_length(json_pritimive); p++)
        {
            json_attibutes = json_object_object_get(json_object_array_get_idx(json_pritimive, p), "attributes");

            json_object* json_position = json_object_object_get(json_attibutes, "POSITION");
            json_object* json_normal = json_object_object_get(json_attibutes, "NORMAL");

            //FIXME: make it possible to handle multiple tex coords (TEXCOORD_1, TEXCOORD_2...)
            json_object* json_texcoords = json_object_object_get(json_attibutes, "TEXCOORD_0");

            int component_size, component_number, accessor_count;

            // if there is a position attribute
            if(!json_object_is_type(json_position, json_type_null)){
                json_accessor = json_object_array_get_idx(object.json_accessors, json_object_get_int(json_position));
                accessor_count = json_object_get_int(json_object_object_get(json_accessor, "count"));

                component_size = bvri_gltfsizeof(
                    json_object_object_get(json_accessor, "componentType"), 
                    json_object_object_get(json_accessor, "type"), 
                    &component_number
                );

                // add position count
                object.vertices.count += (component_size / component_number) * accessor_count;
                object.vertices.type = json_object_get_int(json_object_object_get(json_accessor, "componentType"));
            }
            else {
                BVR_ASSERT(0 || "invalid gdb primitive (missing position attribute)!");
            }

            // if there is a position attribute
            if(!json_object_is_type(json_normal, json_type_null)){
                json_accessor = json_object_array_get_idx(object.json_accessors, json_object_get_int(json_normal)); 
                accessor_count = json_object_get_int(json_object_object_get(json_accessor, "count"));

                component_size = bvri_gltfsizeof(
                    json_object_object_get(json_accessor, "componentType"), 
                    json_object_object_get(json_accessor, "type"), 
                    &component_number
                );

                // add normal count
                object.vertices.count += (component_size / component_number) * accessor_count;
            }
            else {
                BVR_ASSERT(0 || "invalid gdb primitive (missing normal attribute)!");
            }
            
            // if there is a position attribute
            if(!json_object_is_type(json_texcoords, json_type_null)){
                json_accessor = json_object_array_get_idx(object.json_accessors, json_object_get_int(json_texcoords));
                accessor_count = json_object_get_int(json_object_object_get(json_accessor, "count"));
                
                component_size = bvri_gltfsizeof(
                    json_object_object_get(json_accessor, "componentType"), 
                    json_object_object_get(json_accessor, "type"), 
                    &component_number
                );

                // add texcoords count
                object.vertices.count += (component_size / component_number) * accessor_count;
            }
            else {
                BVR_ASSERT(0 || "invalid gdb primitive (missing texcoords attribute)!");
            }

            json_accessor = json_object_array_get_idx(
                object.json_accessors, 
                json_object_get_int(json_object_object_get(json_object_array_get_idx(json_pritimive, p), "indices"))
            );

            object.elements.count += json_object_get_int(json_object_object_get(json_accessor, "count"));
            object.elements.type = json_object_get_int(json_object_object_get(json_accessor, "componentType"));
        }        
    }

    // create vertex et element data to create buffers
    object.vertices.data = NULL;
    object.elements.data = NULL;

    bvri_create_mesh_buffers(mesh, 
        object.vertices.count * sizeof(float),
        object.elements.count * sizeof(uint16),
        object.vertices.type,
        object.elements.type,
        BVR_MESH_ATTRIB_V3UV2N3
    );

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);

    // extract binaries
    {
        size_t readed_bytes = 0;
        fseek(file, bin_section.offset, SEEK_SET);

        bin_section.data = malloc(bin_section.length);
        readed_bytes = fread(bin_section.data, sizeof(char), bin_section.length, file);
        BVR_ASSERT(readed_bytes == bin_section.length && bin_section.data);
    }

    object.elements.count = 0;

    bvr_create_pool(&mesh->vertex_groups, sizeof(bvr_vertex_group_t), json_object_array_length(object.json_nodes));

    // read ressources
    for (size_t i = 0; i < json_object_array_length(object.json_nodes); i++)
    {
        bvr_vertex_group_t* group = bvr_pool_alloc(&mesh->vertex_groups);

        object.scale = 1.0f;

        json_node = json_object_array_get_idx(object.json_nodes, i);
        json_mesh = json_object_array_get_idx(object.json_meshes, json_object_get_int(json_object_object_get(json_node, "mesh")));
        json_pritimive = json_object_object_get(json_mesh, "primitives");

        group->texture = 0;
        group->element_count = 0;
        group->element_offset = object.elements.count;

        bvr_create_string(&group->name, json_object_get_string(json_object_object_get(json_node, "name")));

        for (size_t p = 0; p < json_object_array_length(json_pritimive); p++)
        {
            json_attibutes = json_object_object_get(json_object_array_get_idx(json_pritimive, p), "attributes");

            //WARN: vertices will follow BVR_MESH_ATTRIB_V3UV2N3 format!!

            json_object* json_position = json_object_object_get(json_attibutes, "POSITION");
            json_object* json_normal = json_object_object_get(json_attibutes, "NORMAL");

            //FIXME: make it possible to handle multiple tex coords (TEXCOORD_1, TEXCOORD_2...)
            json_object* json_texcoords = json_object_object_get(json_attibutes, "TEXCOORD_0");
        
            // VERTEX
            if(!json_object_is_type(json_position, json_type_null)){
                bvri_gltfpushbackattribute(&object, json_position, 0, 8, GL_ARRAY_BUFFER);
                bvri_gltfhandlescale(&object, json_position);
            }
            
            // TEXTURE COORDS
            if(!json_object_is_type(json_texcoords, json_type_null)){
                bvri_gltfpushbackattribute(&object, json_texcoords, 3, 8, GL_ARRAY_BUFFER);         
            }

            // NORMALS
            if(!json_object_is_type(json_normal, json_type_null)){
                bvri_gltfpushbackattribute(&object, json_normal, 5, 8, GL_ARRAY_BUFFER);
            }
        
            json_object* json_elements = json_object_object_get(
                json_object_array_get_idx(json_pritimive, p), 
                "indices"
            );
            
            group->element_count += bvri_gltfpushbackattribute(&object, json_elements, 0, 1, GL_ELEMENT_ARRAY_BUFFER);
        }
        
        bvri_gltfhandletransform(&object, json_node, group);

        group->element_offset = object.elements.count;
        object.elements.count += group->element_count;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // free
    //TODO: detected a memory leak sometimes when freeing json's root
    json_object_put((json_object*) json_section.data);

    free(bin_section.data);
    
    return BVR_TRUE;
}

/*
    take component and type json objects pointers and return the total size of this object
    if scalar is not nul, it will return how many component
*/
static int bvri_gltfsizeof(const json_object* component, const json_object* type, int* _scalar){
    int size, scalar, hash;
    scalar = 1;
    hash = bvr_hash(json_object_get_string(type));
    size = bvr_sizeof(json_object_get_int(component));
    
    switch (hash)
    {
    // VEC2
    case 0x28233e: scalar = 2; break;
    
    // VEC3
    case 0x28233f: scalar = 3; break;

    // VEC4
    case 0x282340: scalar = 4; break;

    // MAT4
    case 0x23fef4: scalar = 16; break;
    default: break;
    }

    if(_scalar){
        *_scalar = scalar;
    }

    return size * scalar;
}

static int bvri_gltfpushbackattribute(struct bvri_gltfobject* object, json_object* target, uint32_t offset, const uint32 stride, const uint32 tbuffer){
    // query data
    json_object* json_accessor = json_object_array_get_idx(object->json_accessors, json_object_get_int(target));
    json_object* json_bufferview = json_object_array_get_idx(object->json_bufferviews, json_object_get_int(json_object_object_get(json_accessor, "bufferView")));
    BVR_ASSERT(json_bufferview && json_accessor);

    // get element sizes
    int element_component_number;
    const int element_count = json_object_get_int(json_object_object_get(json_accessor, "count"));
    const int element_size = bvri_gltfsizeof(
        json_object_object_get(json_accessor, "componentType"),
        json_object_object_get(json_accessor, "type"),
        &element_component_number
    );

    // get generic variables
    int buffer_id = json_object_get_int(json_object_object_get(json_bufferview, "buffer"));
    int byte_length = json_object_get_int(json_object_object_get(json_bufferview, "byteLength"));
    int byte_offset = json_object_get_int(json_object_object_get(json_bufferview, "byteOffset"));
    int target_buffer = json_object_get_int(json_object_object_get(json_bufferview, "target"));

    //TODO: support multiple buffers
    BVR_ASSERT(buffer_id == 0);
    
    // check for errors
    BVR_ASSERT(object->binary->length >= byte_offset + byte_length);

    // check for invalid buffer
    if(target_buffer != GL_ARRAY_BUFFER && target_buffer != GL_ELEMENT_ARRAY_BUFFER){
        target_buffer = tbuffer;
    }

    // if there is no stride, we just copy all data to the buffer
    if(stride == 1){
        glBufferSubData(
            target_buffer,
            offset,
            byte_length,
            object->binary->data + byte_offset
        );
    }
    // copy data in-between with a stride
    else {
        char* data = (char*)(object->binary->data + byte_offset);
        char* wroted_bytes = data;

        // multiply a generic offset by the component size
        offset *= (element_size / element_component_number);
        
        while (wroted_bytes - data < byte_length)
        {
            glBufferSubData(
                target_buffer,
                offset,
                element_size,
                wroted_bytes
            );

            wroted_bytes += element_size;
            offset += stride * (element_size / element_component_number);
        }
    }    

    return element_count;
}

static void bvri_gltfhandletransform(struct bvri_gltfobject* object, const json_object* node, bvr_vertex_group_t* group){
    BVR_ASSERT(node);
    BVR_ASSERT(group);

    BVR_IDENTITY_MAT4(group->matrix);

    // get each components
    json_object* translation = json_object_object_get(node, "translation");
    json_object* rotation = json_object_object_get(node, "rotation");
    json_object* scale = json_object_object_get(node, "scale");

    // if the node contains translation informations
    if(json_object_is_type(translation, json_type_array)){
        group->matrix[3][0] = json_object_get_double(json_object_array_get_idx(translation, 0));
        group->matrix[3][1] = json_object_get_double(json_object_array_get_idx(translation, 1));
        group->matrix[3][2] = json_object_get_double(json_object_array_get_idx(translation, 2));
    }

    // if the node contains scaling informations
    if(json_object_is_type(scale, json_type_array)){
        group->matrix[0][0] = json_object_get_double(json_object_array_get_idx(scale, 0));
        group->matrix[1][1] = json_object_get_double(json_object_array_get_idx(scale, 1));
        group->matrix[2][2] = json_object_get_double(json_object_array_get_idx(scale, 2));
    }

    group->matrix[0][0] *= object->scale;
    group->matrix[1][1] *= object->scale;
    group->matrix[2][2] *= object->scale;

    // if the node contains rotation informations 
    if(json_object_is_type(rotation, json_type_array)){
        mat4x4 rotation_matrix;
        quat quat_rotation;
        vec3 euler;
        
        quat_rotation[0] = json_object_get_double(json_object_array_get_idx(rotation, 0));
        quat_rotation[1] = json_object_get_double(json_object_array_get_idx(rotation, 1));
        quat_rotation[2] = json_object_get_double(json_object_array_get_idx(rotation, 2));
        quat_rotation[3] = json_object_get_double(json_object_array_get_idx(rotation, 3));
        euler_quat(euler, quat_rotation);

        mat4_rotate(rotation_matrix, euler);
        mat4_mul(group->matrix, group->matrix, rotation_matrix);
    }
}

static void bvri_gltfhandlescale(struct bvri_gltfobject* object, json_object* target){
    json_object* json_accessor = json_object_array_get_idx(object->json_accessors, json_object_get_int(target));
    BVR_ASSERT(json_accessor);

    json_object* json_min = json_object_object_get(json_accessor, "min");
    json_object* json_max = json_object_object_get(json_accessor, "max");

    vec3 min, max, norm;
    BVR_IDENTITY_VEC3(min);
    BVR_IDENTITY_VEC3(max);

    // try to get min values
    if(json_object_get_type(json_min) != json_type_null){
        min[0] = json_object_get_double(json_object_array_get_idx(json_min, 0));
        min[1] = json_object_get_double(json_object_array_get_idx(json_min, 1));
        min[2] = json_object_get_double(json_object_array_get_idx(json_min, 2));
    }

    // try to get max values
    if(json_object_get_type(json_max) != json_type_null){
        max[0] = json_object_get_double(json_object_array_get_idx(json_max, 0));
        max[1] = json_object_get_double(json_object_array_get_idx(json_max, 1));
        max[2] = json_object_get_double(json_object_array_get_idx(json_max, 2));
    }

    vec3_sub(norm, max, min);
    float scale = ((1.0f / vec3_len(norm)) * 500.0f);

    object->scale = MAX(scale, 1.0f);
}

#endif

#ifndef BVR_NO_FBX

#include <zlib.h>

#define BVR_FBX_FOOTER_LENGTH 172

struct bvri_fbxobject {
    uint32 total_bytes;
    uint32 readed_bytes;

    bvr_mesh_buffer_t vertices;
    bvr_mesh_buffer_t normals;
    bvr_mesh_buffer_t uvs;
    bvr_mesh_buffer_t elements;
};

struct bvri_fbxnode {
    uint32 length;
    uint32 offset;

    uint32 child_count;
    uint32 child_count_length;

    uint32 end_offset;
    uint32 properties_count;
    uint32 property_list_length;

    bvr_string_t name;
    struct bvri_fbxnode* parent;
    struct bvri_fbxnode* childs;
};

static int bvri_is_fbx(FILE* file){
    uint8 sig[20];
    uint8 unknow;
    uint8 endian;
    uint32 version;

    fseek(file, 0, SEEK_SET);
    fread(sig, sizeof(uint8), 20, file);
    sig[20] = '\0'; // unsually the signature finish w/ '\0' but we overwrite to avoid memleeks

    bvr_freadu8_le(file); // padding
    unknow = bvr_freadu8_le(file);
    endian = bvr_freadu8_le(file);
    version = bvr_fread32_le(file);

    return strncmp(sig, "Kaydara FBX Binary", 18) == 0 && 
            unknow == 0x1A &&
            (endian == 0x0 || endian == 0x1);
}

static void bvri_copyfbxproperty(FILE* file, char** destination, size_t* length, uint32* dtype){
    if(destination == NULL || length == NULL){
        return;
    }

    uint32 type = bvr_freadu8_le(file);
    
    // binary types
    if(type == 'R' || type == 'S'){
        // if this object was already allocated
        if(*length){
            uint32 new_length = bvr_fread32_le(file);
            *destination = realloc(*destination, *length + new_length);
            BVR_ASSERT(*destination);

            fread(*destination + *length, sizeof(char), new_length, file);
            *length += new_length;
        }
        else {
            *length = bvr_fread32_le(file);
            *destination = malloc(*length);
            BVR_ASSERT(*destination);

            fread(*destination, sizeof(char), *length, file);
        }
    }
    // primitive types
    else if(type < 'Z'){
        switch (type)
        {
        case 'B':
        case 'C':
            *length = 1; 
            *destination = (char*)bvr_freadu8_le(file);
            break;

        case 'Y':
            *length = 2; 
            *destination = (char*)bvr_fread16_le(file);
            break;
        
        case 'I':
            *length = 4;
            *destination = (char*)bvr_fread32_le(file);
            break;

        case 'F':
            *length = 4;
            //*destination = (char*)bvr_freadf(file);
            BVR_ASSERT(0 || "float not supported");
            break;
        
        case 'D':
            *length = 8;
            //((float*)*destination) = bvr_freadf(file);
            BVR_ASSERT(0 || "double not supported");
            break;

        case 'L':
            *length = 8;
            *destination = (char*)((bvr_fread32_le(file) << 32) | bvr_fread32_le(file));
            break;

        default:
            *length = 0;
            break;
        }
    }

    // array
    else {
        uint32 array_length = bvr_fread32_le(file);
        uint32 encoding = bvr_fread32_le(file);
        uint32 encoding_length = bvr_fread32_le(file);
        
        BVR_ASSERT(encoding == 0);

        switch (type)
        {
        case 'b':
        case 'C':
            *dtype = BVR_UNSIGNED_INT8;
            break;
        
        case 'i':
            *dtype = BVR_INT32;
            break;

        case 'f':
            *dtype = BVR_FLOAT;
            break;
        
        case 'd':
            *dtype = BVR_DOUBLE;
            break;

        case 'l':
            *dtype = BVR_INT64;
            break;

        default:
            *dtype = BVR_NULL;
            break;
        }

        // uncompress
        if(encoding){}
    }

}

static int bvri_readfbxproperty(FILE* file, struct bvri_fbxobject* object, struct bvri_fbxnode* parent_node) {
    if(!parent_node->name.length){
        return BVR_FALSE;
    }

    if(strcmp(parent_node->name.string, "Vertices\0") == 0){
        bvri_copyfbxproperty(file, &object->vertices.data, &object->vertices.count, &object->vertices.type);
        return BVR_TRUE;
    }
    
    if(strcmp(parent_node->name.string, "PolygoneVertexIndex\0") == 0){
        bvri_copyfbxproperty(file, &object->elements.data, &object->elements.count, &object->elements.type);
    }

    return BVR_FALSE;
}

static int bvri_readfbxnode(FILE* file, struct bvri_fbxobject* object, struct bvri_fbxnode* node, size_t offset){
    BVR_ASSERT(node);

    fseek(file, offset, SEEK_SET);
    
    // custom properties
    node->offset = offset;
    node->child_count = 0;
    node->child_count_length = 0;
    node->childs = NULL;

    // reading from file
    node->end_offset = bvr_freadu32_le(file);
    node->properties_count = bvr_fread32_le(file);
    node->property_list_length = bvr_fread32_le(file);
    node->name.length = bvr_freadu8_le(file);
    node->name.string = NULL;

    node->length = 13 + node->name.length + node->property_list_length;

    if(node->end_offset > object->total_bytes || node->offset + node->length > object->total_bytes){
        return BVR_FALSE;
    }
    
    // create name if there is one
    if(node->name.length > 0){
        node->name.string = malloc(node->name.length + 1);

        fread(node->name.string, sizeof(char), node->name.length, file);
        node->name.string[node->name.length] = '\0';
    }

    // length = sizeof(end_offset) + sizeof(properties_count) + sizeof(property_list_length) + 
    //          sizeof(name_length) + name_length + property_list_length

    for (size_t i = 0; i < node->properties_count; i++)
    {
        bvri_readfbxproperty(file, object, node);
    }
    
    // seek after properties
    fseek(file, node->offset + node->length, SEEK_SET);

    // gather childs
    while (offset + node->length < node->end_offset)
    {
        struct bvri_fbxnode child;
        child.parent = node;

        bvri_readfbxnode(file, object, &child, offset + node->length);
        
        node->length += child.length;
        node->child_count_length += child.length;
        node->child_count++;
    }
    
    //BVR_PRINTF("%s (property_count=%i, child_count=%i)", node->name.string, node->properties_count, node->child_count);

    fseek(file, node->end_offset, SEEK_SET);

    bvr_destroy_string(&node->name);
    free(node->childs);

    return BVR_TRUE;
}

// https://docs.fileformat.com/3d/fbx/
// https://gist.github.com/iscle/0dbcee58be8582978d15ea3629ce3e8b
// https://github.com/jskorepa/fbx/tree/master
static int bvri_load_fbx(bvr_mesh_t* mesh, FILE* file){
    BVR_ASSERT(mesh);
    BVR_ASSERT(file);

    struct bvri_fbxobject object;
    object.vertices.count = 0;
    object.vertices.type = BVR_FLOAT;
    object.vertices.data = NULL;
    object.elements.count = 0;
    object.elements.type = BVR_UNSIGNED_INT32;
    object.elements.data = NULL;
    object.uvs.count = 0;
    object.uvs.type = BVR_FLOAT;
    object.uvs.data = NULL;
    object.normals.count = 0;
    object.normals.type = BVR_FLOAT;
    object.normals.data = NULL;

    fseek(file, 22, SEEK_SET);

    uint8 endian = bvr_freadu8_le(file);
    uint32 version = bvr_fread32_le(file);;

    BVR_ASSERT(endian == 0x0 || "big endian fbx not supported");
    
    object.readed_bytes = ftell(file);
    object.total_bytes = bvr_get_file_size(file);

    bool eof = 0;
    while (object.readed_bytes < object.total_bytes - BVR_FBX_FOOTER_LENGTH || eof)
    {
        struct bvri_fbxnode node;

        eof = !bvri_readfbxnode(file, &object, &node, object.readed_bytes);
        object.readed_bytes += node.length;
    }
    
    free(object.vertices.data);
    free(object.uvs.data);
    free(object.normals.data);
    free(object.elements.data);

    BVR_BREAK();

    return BVR_TRUE;
}

#endif

int bvr_create_meshf(bvr_mesh_t* mesh, FILE* file, bvr_mesh_array_attrib_t attrib){
    BVR_ASSERT(mesh);
    BVR_ASSERT(file);

    int status = 0;

    mesh->array_buffer = 0;
    mesh->vertex_buffer = 0;
    mesh->element_buffer = 0;
    mesh->vertex_count = 0;
    mesh->element_count = 0;
    mesh->element_type = 0;
    mesh->attrib_count = 0;
    mesh->stride = 0;
    mesh->attrib = attrib;
    
    mesh->vertex_groups.data = NULL;
    mesh->vertex_groups.avail = NULL;
    mesh->vertex_groups.next_block = NULL;
    mesh->vertex_groups.count = 0;
    mesh->vertex_groups.elemsize = sizeof(bvr_vertex_group_t);

#ifndef BVR_NO_GLTF
    if(bvri_is_gltf(file)){
        status = bvri_load_gltf(mesh, file);
    }
#endif

#ifndef BVR_NO_FBX
    if(!status && bvri_is_fbx(file)){
        status = bvri_load_fbx(mesh, file);
    }
#endif

#ifndef BVR_NO_OBJ
    if(!status && bvri_is_obj(file)){
        status = bvri_load_obj(mesh, file);
    }
#endif

    if(!status){
        BVR_PRINT("failed to load model");
    }

    return status;
}

int bvr_create_meshv(bvr_mesh_t* mesh, bvr_mesh_buffer_t* vertices, bvr_mesh_buffer_t* elements, bvr_mesh_array_attrib_t attrib){
    BVR_ASSERT(mesh);
    BVR_ASSERT(vertices);
    BVR_ASSERT(elements);

    int status = 0;

    mesh->array_buffer = 0;
    mesh->vertex_buffer = 0;
    mesh->element_buffer = 0;
    mesh->vertex_count = 0;
    mesh->element_count = 0;
    mesh->element_type = 0;
    mesh->attrib_count = 0;
    mesh->stride = 0;
    mesh->attrib = attrib;

    bvr_create_pool(&mesh->vertex_groups, sizeof(bvr_vertex_group_t), 1);

    status = bvri_create_mesh_buffers(mesh, 
        vertices->count * bvr_sizeof(vertices->type),
        elements->count * bvr_sizeof(elements->type),
        vertices->type, elements->type, attrib
    );

    // if cannot create buffers
    if(!status){
        return BVR_FALSE;
    }

    bvr_vertex_group_t* group = bvr_pool_alloc(&mesh->vertex_groups);
    group->name.length = 0;
    group->name.string = NULL;
    group->element_offset = 0;
    group->element_count = elements->count;
    group->texture = 0;
    BVR_IDENTITY_MAT4(group->matrix);
    
    // copy vertex values over buffers
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);

    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices->count * bvr_sizeof(vertices->type), vertices->data);

    if(elements->count){
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, elements->count * bvr_sizeof(elements->type), elements->data);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return BVR_TRUE;
}

/*
    Generic buffer creation function
*/
static int bvri_create_mesh_buffers(bvr_mesh_t* mesh, uint64 vertices_size, uint64 element_size, 
    int vertex_type, int element_type, bvr_mesh_array_attrib_t attrib){

    BVR_ASSERT(mesh);
    BVR_ASSERT(vertices_size);

    // gather type size
    const int vertex_t = bvr_sizeof(vertex_type);
    const int element_t = bvr_sizeof(element_type);

    // create vertex array 
    glGenVertexArrays(1, &mesh->array_buffer);
    glBindVertexArray(mesh->array_buffer);

    // create vertex and element buffers
    glGenBuffers(1, &mesh->vertex_buffer);
    glGenBuffers(1, &mesh->element_buffer);

    // allocate the whole buffers
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices_size, NULL, GL_STATIC_DRAW);

    if(element_size){
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_size, NULL, GL_STATIC_DRAW);
    }
    
    mesh->attrib = attrib;
    mesh->stride = attrib * vertex_t;
    mesh->vertex_count = vertices_size / vertex_t;
    mesh->element_count = element_size / element_t;
    mesh->element_type = element_type;


    // define each attributes pointers depending on attribute's type
    switch (attrib)
    {
    case BVR_MESH_ATTRIB_V2:
        {
            mesh->attrib_count = 1;

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, vertex_type, GL_FALSE, mesh->stride, (void*)0);
        }
        break;
    
    case BVR_MESH_ATTRIB_V3:
        {
            mesh->attrib_count = 1;

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, vertex_type, GL_FALSE, mesh->stride, (void*)0);
        }
        break;

    case BVR_MESH_ATTRIB_V2UV2:
        {
            mesh->attrib_count = 2;

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, vertex_type, GL_FALSE, mesh->stride, (void*)0);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, vertex_type, GL_FALSE, mesh->stride, (void*)(2 * vertex_t));
        }
        break;
    
    case BVR_MESH_ATTRIB_V3UV2:
        {
            mesh->attrib_count = 2;

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, vertex_type, GL_FALSE, mesh->stride, (void*)0);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, vertex_type, GL_FALSE, mesh->stride, (void*)(3 * vertex_t));
        }
        break;
    
    case BVR_MESH_ATTRIB_V3UV2N3:
        {
            mesh->attrib_count = 3;

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, vertex_type, GL_FALSE, mesh->stride, (void*)0);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, vertex_type, GL_FALSE, mesh->stride, (void*)(3 * vertex_t));

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, vertex_type, GL_FALSE, mesh->stride, (void*)(5 * vertex_t));
        }
        break;

    case BVR_MESH_ATTRIB_SINGLE:
        {
            mesh->attrib_count = 1;

            glEnableVertexAttribArray(0);
            glVertexAttribIPointer(0, 1, vertex_type, mesh->stride, (void*)0);
        }
        break;

    default:
        {
            BVR_PRINT("cannot recognize attribute type!");
            bvr_destroy_mesh(mesh);
        }
        return BVR_FALSE;
    }

    if(!mesh->stride){
        BVR_PRINT("cannot get vertex type size!");
        bvr_destroy_mesh(mesh);
        return BVR_FALSE;
    }

    for (uint64 i = 0; i < mesh->attrib_count; i++){ 
        glDisableVertexAttribArray(i); 
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return BVR_TRUE;
}

// https://github.com/joelibaceta/triangulator/blob/main/triangulator/ear_clipping_method.py
void bvr_triangulate(bvr_mesh_buffer_t* src, bvr_mesh_buffer_t* dest, const uint8 stride){
    BVR_ASSERT(src);
    BVR_ASSERT(dest);
    BVR_ASSERT(src->data && src->count);

    dest->type = BVR_FLOAT;
    dest->count = 0;
    dest->data = NULL;

    struct {
        vec2 a, b, c;
    } triangles[1024];

    // create a copy of the vertices.
    uint32 vertex_count = src->count / stride;
    vec2* raw_poly = (vec2*)src->data;
    vec2* polygone = malloc(src->count * bvr_sizeof(src->type));
    BVR_ASSERT(polygone);

    memcpy(polygone, src->data, src->count * bvr_sizeof(src->type));

    int signed_area = 0;
    {
        vec2 p0, p1;
        for (size_t point = 0; point < vertex_count; point++)
        {
            vec2_copy(p0, polygone[point]);
            vec2_copy(p1, polygone[((point + 1) % vertex_count)]);

            signed_area += vec2_cross(p0, p1);
        }
     
        // if this polygone is signed, then we flip it
        if(signed_area < 0){
            for (size_t point = 0; point < vertex_count; point++)
            {
                vec2 tmp;
                vec2_copy(tmp, polygone[point]);
                vec2_copy(polygone[point], polygone[vertex_count - 1 - point]);
                vec2_copy(polygone[vertex_count - 1 - point], tmp);
            }
        }
    }

    vec2* prev_vert = (vec2*)1;
    vec2* curr_vert = (vec2*)1;
    vec2* next_vert = (vec2*)1;

    // while there are triangles to be found
    while (curr_vert != 0)
    {
        prev_vert = NULL;
        curr_vert = NULL;
        next_vert = NULL;

        for (size_t point = 0; point < vertex_count; point++)
        {
            prev_vert = &polygone[(point + vertex_count - 1) % vertex_count];
            curr_vert = &polygone[point];
            next_vert = &polygone[(point + 1) % vertex_count];
        
            // determine triangle angle
            float angle = 0;
            {
                vec2 ba, bc;
                vec2_sub(ba, *prev_vert, *curr_vert);
                vec2_sub(bc, *next_vert, *curr_vert);

                float theta = vec2_dot(ba, bc) / (vec2_len(ba) * vec2_len(bc));
                angle = rad_to_deg(acosf(theta));

                if(vec2_cross(ba, bc) < 0){
                    angle = 360 - angle;
                }
            }

            if(angle >= 180){
                // skip because the angle is greater than 180
                continue;
            }
            else {
                bool found_inside = false;

                // check if there is another poly point inside the triangle we found 
                for (size_t i = 0; i < vertex_count; i++)
                {
                    if( ((*prev_vert)[0] == raw_poly[i][0] && (*prev_vert)[1] == raw_poly[i][1]) ||
                        ((*curr_vert)[0] == raw_poly[i][0] && (*curr_vert)[1] == raw_poly[i][1]) ||
                        ((*next_vert)[0] == raw_poly[i][0] && (*next_vert)[1] == raw_poly[i][1])){

                        continue;
                    }

                    if(bvr_is_point_inside_triangle(raw_poly[i], *prev_vert, *curr_vert, *next_vert)){
                        found_inside = true;
                        break;
                    }
                }

                // add the new founded triangle
                if(!found_inside){
                    vec2_copy(triangles[dest->count].a, *prev_vert);
                    vec2_copy(triangles[dest->count].b, *curr_vert);
                    vec2_copy(triangles[dest->count].c, *next_vert);

                    // pop out the ear point from the list
                    for (size_t y = point; y < vertex_count; y++)
                    {
                        vec2_copy(polygone[y], polygone[y + 1]);
                    }

                    vertex_count--;
                    dest->count++;
                    
                    break;
                }
            }
        }
        
        if(curr_vert == NULL){
            break;
        }
    }
    
    dest->type = BVR_VEC2;
    dest->count = dest->count * 3;
    dest->data = malloc(dest->count * sizeof(vec2));

    memcpy(dest->data, &triangles[0], dest->count * sizeof(vec2));

    free(polygone);
}

void bvr_destroy_mesh(bvr_mesh_t* mesh){
    BVR_ASSERT(mesh);

    bvr_vertex_group_t group;
    BVR_POOL_FOR_EACH(group, mesh->vertex_groups){
        //TODO: find why this fucking vertex group mess up string's pointer
        //bvr_destroy_string(&group.name);
    }

    //free(mesh->vertex_groups.data);

    glDeleteVertexArrays(1, &mesh->array_buffer);
    glDeleteBuffers(1, &mesh->vertex_buffer);
    glDeleteBuffers(1, &mesh->element_buffer);
}