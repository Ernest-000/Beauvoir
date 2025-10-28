#pragma once

#include <stdint.h>

#include <BVR/utils.h>
#include <BVR/buffer.h>

typedef enum bvr_drawmode_e {
    BVR_DRAWMODE_LINES = 0x0001,
    BVR_DRAWMODE_LINE_STRIPE = 0x0003,
    BVR_DRAWMODE_TRIANGLES = 0x0004,
    BVR_DRAWMODE_TRIANGLES_STRIP = 0x0005,
    BVR_DRAWMODE_QUADS = 0x0007
} bvr_drawmode_t;

typedef struct bvr_mesh_buffer_s {
    char* data;
    unsigned long long count;
    unsigned int type;
} bvr_mesh_buffer_t;

typedef enum bvr_mesh_array_attrib_e {
    /*
        vertices    -> vec2
    */
    BVR_MESH_ATTRIB_V2 = 2,
    
    /*
        vertices    -> vec3
    */
    BVR_MESH_ATTRIB_V3 = 3,

    /*
        vertices    -> vec2
        uvs         -> vec2
    */
    BVR_MESH_ATTRIB_V2UV2 = 4,
    
    /*
        vertices    -> vec3
        uvs         -> vec2
    */
    BVR_MESH_ATTRIB_V3UV2 = 5,

    /*
        vertices    -> vec3
        uvs         -> vec2
        normals     -> vec3 
    */
    BVR_MESH_ATTRIB_V3UV2N3 = 8,

    /*
        special attribute use for landscapes

        0x000000FF -> tex id
        0x0000FF00 -> altitude
        0x00FF0000 -> normal yaw
        0xFF000000 -> normal pitch
    */
    BVR_MESH_ATTRIB_SINGLE = 1
} bvr_mesh_array_attrib_t;

typedef struct bvr_vertex_group_s {
    bvr_string_t name;

    uint32 element_count;
    uint32 element_offset;

    uint16 texture;
    uint16 flags;
} __attribute__ ((packed)) bvr_vertex_group_t; 

typedef struct bvr_mesh_s {
    uint32 array_buffer;
    uint32 vertex_buffer;
    uint32 element_buffer;

    uint32 vertex_count;
    uint32 element_count;
    
    bvr_pool_t vertex_groups;

    int element_type;

    bvr_mesh_array_attrib_t attrib;
    uint16 stride;

    uint8 attrib_count;
} bvr_mesh_t;

/*
    Create a new mesh by using raw vertices and indices data
*/
int bvr_create_meshv(bvr_mesh_t* mesh, bvr_mesh_buffer_t* vertices, bvr_mesh_buffer_t* elements, bvr_mesh_array_attrib_t attrib);

/*
    Create a new mesh by using a FILE
*/
int bvr_create_meshf(bvr_mesh_t* mesh, FILE* file, bvr_mesh_array_attrib_t attrib);

/*
    Create a new mesh from path
*/
BVR_H_FUNC int bvr_create_mesh(bvr_mesh_t* mesh, const char* path, bvr_mesh_array_attrib_t attrib){
    BVR_FILE_EXISTS(path);
    FILE* file = fopen(path, "rb");
    int status = bvr_create_meshf(mesh, file, attrib);
    fclose(file);
    return status;
}

void bvr_triangulate(bvr_mesh_buffer_t* src, bvr_mesh_buffer_t* dest, const uint8 stride);

void bvr_destroy_mesh(bvr_mesh_t* mesh);

#ifdef BVR_INCLUDE_GEOMETRY

BVR_H_FUNC void bvr_create_2d_square_mesh(bvr_mesh_t* mesh, float width, float height){
    float vertices[16] = {
        -width,  height, 0, 1,
        -width, -height, 0, 0,
         width, -height, 1, 0,
         width,  height, 1, 1
    };

    uint32_t indices[6] = {0, 1, 2, 0, 2, 3};

    bvr_mesh_buffer_t vertices_buffer;
    vertices_buffer.data = (char*) vertices;
    vertices_buffer.type = BVR_FLOAT;
    vertices_buffer.count = 16;

    bvr_mesh_buffer_t element_buffer;
    element_buffer.data = (char*) indices;
    element_buffer.type = BVR_UNSIGNED_INT32;
    element_buffer.count = 6;

    bvr_create_meshv(mesh, &vertices_buffer, &element_buffer, BVR_MESH_ATTRIB_V2UV2);
}


BVR_H_FUNC void bvr_create_3d_square_mesh(bvr_mesh_t* mesh, float width, float height){
    float vertices[20] = {
        -width, 0,  height, 0, 1,
        -width, 0, -height, 0, 0,
         width, 0, -height, 1, 0,
         width, 0,  height, 1, 1
    };

    uint32_t indices[6] = {0, 1, 2, 0, 2, 3};

    bvr_mesh_buffer_t vertices_buffer;
    vertices_buffer.data = (char*) vertices;
    vertices_buffer.type = BVR_FLOAT;
    vertices_buffer.count = 20;

    bvr_mesh_buffer_t element_buffer;
    element_buffer.data = (char*) indices;
    element_buffer.type = BVR_UNSIGNED_INT32;
    element_buffer.count = 6;

    bvr_create_meshv(mesh, &vertices_buffer, &element_buffer, BVR_MESH_ATTRIB_V3UV2);
}

#endif