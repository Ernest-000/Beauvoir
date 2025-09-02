#include <BVR/mesh.h>
#include <BVR/math.h>
#include <BVR/buffer.h>
#include <BVR/file.h>
#include <BVR/physics.h>

#include <malloc.h>
#include <string.h>
#include <memory.h>

#include <json-c/json.h>

#include <GLAD/glad.h>

static int bvri_create_mesh_buffers(bvr_mesh_t* mesh, uint64 vertices_size, uint64 element_size, 
    int vertex_type, int element_type, bvr_mesh_array_attrib_t attrib);

#ifndef BVR_NO_OBJ

#include <ctype.h>

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
    
    BVR_ASSERT(*buffer == 'E' || *buffer == 'e' || "powers aren't supported :(");

    *v = (float)sig * num;

    return buffer;
}

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
            return BVR_OK;
        }

        if(sig[0] == 'o'){
            return BVR_OK;
        }
    }
    
    return BVR_FAILED;
}

/*
    Push back a new vertex group as the targetted group
*/
static bvr_vertex_group_t* bvri_objpushgrp(struct bvr_buffer_s* group, bvr_string_t* name){
    bvr_vertex_group_t* vertex_group;

    if(group->data){
        group->data = realloc(group->data, group->size + group->elemsize);
        BVR_ASSERT(group->data);

        vertex_group = (bvr_vertex_group_t*)group->data + group->size;
        group->size += group->elemsize;
    }
    else {
        group->data = malloc(group->elemsize);
        group->size += group->elemsize;

        vertex_group = (bvr_vertex_group_t*)group->data;
    }

    bvr_string_create_and_copy(&vertex_group->name, name);
    vertex_group->element_count = 0;
    return vertex_group;
}

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

    struct bvr_buffer_s vertex_group;
    bvr_vertex_group_t* group;

    uint32 vertex_count;
    uint32 uv_count;
    uint32 normal_count;
    uint32 face_count;
};

static int bvri_load_obj(bvr_mesh_t* mesh, FILE* file){
    BVR_ASSERT(mesh);

    struct bvri_objobject_s object;

    object.vertex_count = 0;
    object.uv_count = 0;
    object.normal_count = 0;
    object.face_count = 0;
    object.group = NULL;

    object.vertex_group.data = NULL;
    object.vertex_group.elemsize = sizeof(bvr_vertex_group_t);
    object.vertex_group.size = 0;

    object.elements.data = NULL;
    object.vertices.data = NULL;
    object.elements.count = 0;
    object.vertices.count = 0;
    object.elements.type = BVR_UNSIGNED_INT32;
    object.vertices.type = BVR_FLOAT;

    mesh->stride = 3;
    if(mesh->attrib >= BVR_MESH_ATTRIB_V2UV2) mesh->stride += 2;
    if(mesh->attrib == BVR_MESH_ATTRIB_V3UV2N3) mesh->stride += 3;

    bvr_create_string(&object.name, NULL);
    bvr_create_string(&object.material, NULL);

    char* cursor;
    char buffer[256];
    while(bvri_objreadline(buffer, file)){
        switch (buffer[0])
        {
        case '#':
            /* skip */
            break;
        
        case 'm':
            bvr_create_string(&object.material, &buffer[7]);
            break;    

        case 'o':
            bvr_create_string(&object.name, &buffer[2]);
            object.group = bvri_objpushgrp(&object.vertex_group, &object.name);
            object.group->element_offset = object.elements.count;
            break;

        case 'v':
            {
                cursor = NULL;

                if(buffer[1] == 'n' && mesh->stride >= 8){
                    BVR_ASSERT(object.normal_count < BVR_BUFFER_SIZE);

                    cursor = bvri_objparsefloat(&buffer[3], &object.normals[object.normal_count][0]);
                    cursor = bvri_objparsefloat(++cursor, &object.normals[object.normal_count][1]);
                    cursor = bvri_objparsefloat(++cursor, &object.normals[object.normal_count][2]);
                    object.normal_count++;
                }
                else if(buffer[1] == 't' && mesh->stride >= 5){
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

    if(!object.elements.count || !object.vertices.count){
        BVR_PRINT("failed to load mesh :(");
        goto bvr_objfailed;
    }

    if(bvri_create_mesh_buffers(mesh, 
        object.vertices.count * sizeof(float), 
        object.elements.count * sizeof(uint32),
        object.vertices.type, object.elements.type, mesh->attrib) == BVR_FAILED){

        BVR_PRINT("failed to create object buffers");
        goto bvr_objfailed;
    }

    mesh->vertex_groups.size = object.vertex_group.size;
    mesh->vertex_groups.data = object.vertex_group.data;

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);

    uint64 vertex = 0, element = 0;
    for (uint64 face = 0; face < object.face_count; face++)
    {
        BVR_ASSERT(object.faces[face].edges == 3 || object.faces[face].edges == 4);

        if(object.faces[face].edges == 4){
            BVR_ASSERT(0 || "not supported");
        }
        else {
            BVR_ASSERT(element + 3 <= object.elements.count);

            const uint32 elements_values[3] = {element, element + 1, element + 2};
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, element * sizeof(uint32), sizeof(elements_values), elements_values);
            element += 3;
        }

        for (uint64 i = 0; i < object.faces[face].edges; i++)
        {
            BVR_ASSERT(vertex + 8 <= object.vertices.count);

            glBufferSubData(GL_ARRAY_BUFFER, (vertex + 0) * sizeof(float), sizeof(vec3), object.vertex[object.faces[face].vertex[i] - 1]);
            glBufferSubData(GL_ARRAY_BUFFER, (vertex + 3) * sizeof(float), sizeof(vec2), object.uvs[object.faces[face].uv[i] - 1]);
            glBufferSubData(GL_ARRAY_BUFFER, (vertex + 5) * sizeof(float), sizeof(vec3), object.normals[object.faces[face].normal[i] - 1]);
            vertex += 8;
        }
        
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    bvr_destroy_string(&object.name);
    bvr_destroy_string(&object.material);

    free(object.vertices.data);
    free(object.elements.data);

    return BVR_OK;

bvr_objfailed:
    bvr_destroy_string(&object.name);
    bvr_destroy_string(&object.material);

    for (uint64 i = 0; i < BVR_BUFFER_COUNT(object.vertex_group); i++)
    {
        bvr_destroy_string(&((bvr_vertex_group_t*)object.vertex_group.data)[0].name);
    }
    
    free(object.vertex_group.data);
    free(object.vertices.data);
    free(object.elements.data);

    return BVR_FAILED;
}

#endif

struct bvri_gltfchunk {
    uint32 length;
    uint32 sig;
    size_t offset;

    void* data;
};

struct bvri_gltfobject {
    struct bvri_gltfchunk* json;
    struct bvri_gltfchunk* binary;

    uint32 vertex_count;
    uint32 uv_count;
    uint32 normal_count;
    uint32 element_count;
    
    struct bvr_buffer_s vertex_group;
    bvr_vertex_group_t* group;

    bvr_mesh_buffer_t vertices;
    bvr_mesh_buffer_t elements;

    json_object* json_meshes;
    json_object* json_accessors;
    json_object* json_bufferviews;
    json_object* json_buffers;
};

static int bvri_gltftypeof(const int type){
    switch (type)
    {
    case 5120:
        return BVR_INT8;
    case 5121:
        return BVR_UNSIGNED_INT8;
    case 5122:
        return BVR_INT16;
    case 5123:
        return BVR_UNSIGNED_INT16;
    case 5125:
        return BVR_UNSIGNED_INT32;
    case 5126:
        return BVR_FLOAT;
    
    default:
        break;
    }
}

static void bvri_gltfpushbackattribute(struct bvri_gltfobject* object, json_object* target, uint32_t offset, const uint32 stride){
    json_object* json_accessor = json_object_array_get_idx(object->json_accessors, json_object_get_int(target));
    json_object* json_bufferview = json_object_array_get_idx(object->json_bufferviews, json_object_get_int(json_object_object_get(json_accessor, "bufferView")));

    const int count = json_object_get_int(json_object_object_get(json_accessor, "count"));

    const int buffer = json_object_get_int(json_object_object_get(json_bufferview, "buffer"));
    const int byte_length = json_object_get_int(json_object_object_get(json_bufferview, "byteLength"));
    const int byte_offset = json_object_get_int(json_object_object_get(json_bufferview, "byteOffset"));
    const int target_buffer = json_object_get_int(json_object_object_get(json_bufferview, "target"));

    //TODO: support multiple buffers
    BVR_ASSERT(buffer == 0);
    BVR_ASSERT(target_buffer == GL_ARRAY_BUFFER || target_buffer == GL_ELEMENT_ARRAY_BUFFER);

    // if there is no stride, we just copy all data to the buffer
    if(stride == 1){
        glBufferSubData(
            target_buffer,
            offset,
            byte_length,
            object->binary->data + byte_offset
        );
    }

    // copy data split with a stride
    else {
        char* data = (char*)(object->binary->data + byte_offset);
        char* wrote_bytes = data;

        while (wrote_bytes - data < byte_length)
        {
            glBufferSubData(
                target_buffer,
                offset,
                byte_length / count,
                wrote_bytes
            );

            wrote_bytes += byte_length / count;
            offset += stride * (byte_length / count / 4);
        }
    }    
}

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

    object.vertex_count = 0;
    object.uv_count = 0;
    object.normal_count = 0;
    object.element_count = 0;

    object.vertex_group.size = 0;
    object.vertex_group.elemsize = sizeof(bvr_vertex_group_t);
    object.vertex_group.data = NULL;
    object.group = NULL;

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
        return BVR_FAILED;
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
        return BVR_FAILED;
    }

    json_tokener* json_tok = json_tokener_new();
    
    // read json section and create json context
    {
        fseek(file, json_section.offset, SEEK_SET);

        char* json_content = malloc(json_section.length);
        fread(json_content, sizeof(char), json_section.length, file);

        json_section.data = json_tokener_parse_ex(json_tok, json_content, json_section.length);
        free(json_content);
    }

    object.json_meshes = json_object_object_get((json_object*) json_section.data, "meshes");
    object.json_accessors = json_object_object_get((json_object*) json_section.data, "accessors");
    object.json_bufferviews = json_object_object_get((json_object*) json_section.data, "bufferViews");
    object.json_buffers = json_object_object_get((json_object*) json_section.data, "buffers");

    // check for objects assignments
    if(!(json_section.data && object.json_meshes && object.json_accessors && object.json_bufferviews && object.json_buffers)){
        json_object_put((json_object*) json_section.data);

        BVR_PRINT("corrupted or missing gdb json!");
        return BVR_FAILED;
    }

    object.vertex_group.size = object.vertex_group.elemsize * json_object_array_length(object.json_meshes);
    object.vertex_group.data = malloc(object.vertex_group.size);
    BVR_ASSERT(object.vertex_group.data);

    memset(object.vertex_group.data, 0, object.vertex_group.size);
    object.group = (bvr_vertex_group_t*) object.vertex_group.data;

    json_object* json_mesh = NULL;
    json_object* json_pritimive = NULL;
    json_object* json_accessor = NULL;
    json_object* json_attibutes = NULL;

    // create ressources
    for (size_t i = 0; i < json_object_array_length(object.json_meshes); i++)
    {
        json_mesh = json_object_array_get_idx(object.json_meshes, i);
        json_pritimive = json_object_object_get(json_mesh, "primitives");

        object.group = (bvr_vertex_group_t*)(object.vertex_group.data + object.vertex_group.elemsize * i);
        object.group->element_offset = object.element_count;
        object.group->element_count = 0;

        // create null string but i shall replce with meshes's name
        bvr_create_string(&object.group->name, NULL);

        for (size_t p = 0; p < json_object_array_length(json_pritimive); p++)
        {
            json_attibutes = json_object_object_get(json_object_array_get_idx(json_pritimive, p), "attributes");

            json_object* json_position = json_object_object_get(json_attibutes, "POSITION");
            json_object* json_normal = json_object_object_get(json_attibutes, "NORMAL");

            //FIXME: make it possible to handle multiple tex coords (TEXCOORD_1, TEXCOORD_2...)
            json_object* json_texcoords = json_object_object_get(json_attibutes, "TEXCOORD_0");

            // if there is a position attribute
            if(!json_object_is_type(json_position, json_type_null)){
                json_accessor = json_object_array_get_idx(object.json_accessors, json_object_get_int(json_position));
                int size = bvr_sizeof(bvri_gltftypeof(json_object_get_int(json_object_object_get(json_accessor, "componentType"))));

                // add position count
                object.vertex_count += (size / sizeof(float)) * json_object_get_int(json_object_object_get(json_accessor, "count"));
            }
            else {
                BVR_ASSERT(0 || "invalid gdb primitive (missing position attribute)!");
            }

            // if there is a position attribute
            if(!json_object_is_type(json_normal, json_type_null)){
                json_accessor = json_object_array_get_idx(object.json_accessors, json_object_get_int(json_normal));                
                int size = bvr_sizeof(bvri_gltftypeof(json_object_get_int(json_object_object_get(json_accessor, "componentType"))));

                // add normal count
                object.normal_count += (size / sizeof(float)) * json_object_get_int(json_object_object_get(json_accessor, "count"));
            }
            else {
                BVR_ASSERT(0 || "invalid gdb primitive (missing normal attribute)!");
            }
            
            // if there is a position attribute
            if(!json_object_is_type(json_texcoords, json_type_null)){
                json_accessor = json_object_array_get_idx(object.json_accessors, json_object_get_int(json_texcoords));
                int size = bvr_sizeof(bvri_gltftypeof(json_object_get_int(json_object_object_get(json_accessor, "componentType"))));

                // add texcoords count
                object.uv_count += (size / sizeof(float)) * json_object_get_int(json_object_object_get(json_accessor, "count"));
            }
            else {
                BVR_ASSERT(0 || "invalid gdb primitive (missing texcoords attribute)!");
            }

            json_accessor = json_object_array_get_idx(object.json_accessors, json_object_get_int(json_object_object_get(json_object_array_get_idx(json_pritimive, p), "indices")));
            object.group->element_count += json_object_get_int(json_object_object_get(json_accessor, "count"));
        }
        
        object.element_count += object.group->element_count;
        BVR_PRINTF("%i %i", object.vertex_count, object.element_count);
    }

    // create vertex et element data to create buffers
    object.vertices.count = object.vertex_count + object.normal_count + object.uv_count;
    object.vertices.type = BVR_FLOAT;
    object.vertices.data = NULL;

    object.elements.count = object.element_count;
    object.elements.type = BVR_UNSIGNED_INT16; // is element type always uint16 w/ gltf?
    object.elements.data = NULL;
    bvr_create_meshv(mesh, &object.vertices, &object.elements, BVR_MESH_ATTRIB_V3UV2N3);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);

    // extract binaries
    {
        size_t readed_bytes = 0;
        fseek(file, bin_section.offset, SEEK_SET);

        bin_section.data = malloc(bin_section.length);
        readed_bytes = fread(bin_section.data, sizeof(char), bin_section.length, file);
        BVR_ASSERT(readed_bytes == bin_section.length);
    }

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
            //WARN: vertices will follow BVR_MESH_ATTRIB_V3UV2N3 format!!
            json_object* json_texcoords = json_object_object_get(json_attibutes, "TEXCOORD_0");
        
            // VERTEX
            if(!json_object_is_type(json_position, json_type_null)){
                bvri_gltfpushbackattribute(&object, json_position, 0, 7);
            }
            
            // TEXTURE COORDS
            if(!json_object_is_type(json_texcoords, json_type_null)){
                bvri_gltfpushbackattribute(&object, json_texcoords, 3, 7);         
            }

            // NORMALS
            if(!json_object_is_type(json_normal, json_type_null)){
                bvri_gltfpushbackattribute(&object, json_normal, 5, 7);
            }
        }

        json_object* json_elements = json_object_object_get(json_pritimive, "indices");
        bvri_gltfpushbackattribute(&object, json_elements, 0, 1);
    }

    //debug
    if(1){
        uint16* vertex_buffer = glMapBufferRange(GL_ARRAY_BUFFER, 0, object.elements.count * sizeof(uint16), GL_MAP_READ_BIT);
        for (size_t i = 0; i < object.element_count; i++)
        {
            BVR_PRINTF("i%i", vertex_buffer[i]);
        }
        
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    free(object.vertex_group.data);

    mesh->vertex_groups.data = NULL;
    mesh->vertex_groups.size = object.vertex_group.size;
    mesh->vertex_groups.elemsize = sizeof(bvr_vertex_group_t);

    // free
    json_object_put((json_object*) json_section.data);
    free(bin_section.data);
    
    return BVR_OK;
}

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

    mesh->vertex_groups.size = 0;
    mesh->vertex_groups.elemsize = sizeof(bvr_vertex_group_t);
    mesh->vertex_groups.data = NULL;

    if(bvri_is_gltf(file)){
        status = bvri_load_gltf(mesh, file);
    }

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

    mesh->vertex_groups.size = 0;
    mesh->vertex_groups.elemsize = sizeof(bvr_vertex_group_t);
    mesh->vertex_groups.data = NULL;

    status = bvri_create_mesh_buffers(mesh, 
        vertices->count * bvr_sizeof(vertices->type),
        elements->count * bvr_sizeof(elements->type),
        vertices->type, elements->type, attrib
    );

    // if cannot create buffers
    if(!status){
        return BVR_FAILED;
    }

    // allocate a single vertex group 
    mesh->vertex_groups.size = sizeof(bvr_vertex_group_t);
    mesh->vertex_groups.data = malloc(mesh->vertex_groups.size);
    BVR_ASSERT(mesh->vertex_groups.data);

    ((bvr_vertex_group_t*)mesh->vertex_groups.data)[0].name.length = 0;
    ((bvr_vertex_group_t*)mesh->vertex_groups.data)[0].name.string = NULL;
    ((bvr_vertex_group_t*)mesh->vertex_groups.data)[0].element_offset = 0;
    ((bvr_vertex_group_t*)mesh->vertex_groups.data)[0].element_count = elements->count;

    // copy vertex values over buffers
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);

    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices->count * bvr_sizeof(vertices->type), vertices->data);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, elements->count * bvr_sizeof(elements->type), elements->data);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return BVR_OK;
}

/*
    Generic buffer creation function
*/
static int bvri_create_mesh_buffers(bvr_mesh_t* mesh, uint64 vertices_size, uint64 element_size, 
    int vertex_type, int element_type, bvr_mesh_array_attrib_t attrib){

    BVR_ASSERT(mesh);
    BVR_ASSERT(vertices_size);

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
    mesh->vertex_count = vertices_size / bvr_sizeof(vertex_type);
    mesh->element_count = element_size / bvr_sizeof(element_type);
    mesh->element_type = element_type;

    // define each attributes pointers depending on attribute's type
    switch (attrib)
    {
    case BVR_MESH_ATTRIB_V2:
        {
            mesh->attrib_count = 1;
            mesh->stride = 2 * bvr_sizeof(vertex_type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, vertex_type, GL_FALSE, mesh->stride, (void*)0);
        }
        break;
    
    case BVR_MESH_ATTRIB_V3:
        {
            mesh->attrib_count = 1;
            mesh->stride = 3 * bvr_sizeof(vertex_type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, vertex_type, GL_FALSE, mesh->stride, (void*)0);
        }
        break;

    case BVR_MESH_ATTRIB_V2UV2:
        {
            mesh->attrib_count = 2;
            mesh->stride = (2 + 2) * bvr_sizeof(vertex_type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, vertex_type, GL_FALSE, mesh->stride, (void*)0);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, vertex_type, GL_FALSE, mesh->stride, (void*)(2 * bvr_sizeof(vertex_type)));
        }
        break;
    
    case BVR_MESH_ATTRIB_V3UV2:
        {
            mesh->attrib_count = 2;
            mesh->stride = (3 + 2) * bvr_sizeof(vertex_type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, vertex_type, GL_FALSE, mesh->stride, (void*)0);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, vertex_type, GL_FALSE, mesh->stride, (void*)(3 * bvr_sizeof(vertex_type)));
        }
        break;
    
    case BVR_MESH_ATTRIB_V3UV2N3:
        {
            mesh->attrib_count = 3;
            mesh->stride = (3 + 2 + 3) * bvr_sizeof(vertex_type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, vertex_type, GL_FALSE, mesh->stride, (void*)0);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, vertex_type, GL_FALSE, mesh->stride, (void*)(3 * bvr_sizeof(vertex_type)));

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, vertex_type, GL_FALSE, mesh->stride, (void*)(5 * bvr_sizeof(vertex_type)));
        }
        break;

    default:
        {
            BVR_PRINT("cannot recognize attribute type!");
            bvr_destroy_mesh(mesh);
        }
        return BVR_FAILED;
    }

    if(!mesh->stride){
        BVR_PRINT("cannot get vertex type size!");
        bvr_destroy_mesh(mesh);
        return BVR_FAILED;
    }

    for (uint64 i = 0; i < mesh->attrib_count; i++){ 
        glDisableVertexAttribArray(i); 
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return BVR_OK;
}

void bvr_triangulate(bvr_mesh_buffer_t* src, bvr_mesh_buffer_t* dest, const uint8 stride){

    // https://github.com/joelibaceta/triangulator/blob/main/triangulator/ear_clipping_method.py

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

    for (uint64 i = 0; i < BVR_BUFFER_COUNT(mesh->vertex_groups); i++)
    {
        bvr_destroy_string(&((bvr_vertex_group_t*)mesh->vertex_groups.data)[i].name);
    }
    free(mesh->vertex_groups.data);

    glDeleteVertexArrays(1, &mesh->array_buffer);
    glDeleteBuffers(1, &mesh->vertex_buffer);
    glDeleteBuffers(1, &mesh->element_buffer);
}