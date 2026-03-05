#include <bvr/landscape.h>

#include <bvr/file.h>

#include <glad/glad.h>
#include <json-c/json.h>

static void bvri_update_landscape_buffers(bvr_landscape_t* landscape, uint8 layer){
    BVR_ASSERT(landscape);

    if(!landscape->mesh){
        return;
    }

    if(!landscape->mesh->vertex_buffer){
        return;
    }

    uint64 tiles_per_layer = landscape->grid.tile_count / BVR_BUFFER_COUNT(landscape->grid.layers);

    glBindBuffer(GL_ARRAY_BUFFER, landscape->mesh->vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, tiles_per_layer * layer, tiles_per_layer, landscape->grid.tiles);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return;
}

int bvr_create_landscape_empty(bvr_landscape_t* landscape, 
    uint16 tile_per_row, uint16 tile_per_column, vec2 tile_size, uint8 layer_count){

    BVR_ASSERT(landscape);
    BVR_ASSERT(landscape->mesh);

    landscape->grid.tile_per_row = tile_per_row;
    landscape->grid.tile_per_column = tile_per_column;
    landscape->grid.tile_size[0] = tile_size[0];
    landscape->grid.tile_size[1] = tile_size[1];
    landscape->grid.layers.data = NULL;
    landscape->grid.layers.elemsize = sizeof(struct bvr_landscape_layer_s);
    landscape->grid.layers.size = MIN(1, layer_count) * landscape->grid.layers.elemsize;

    landscape->grid.tile_count = tile_per_row * tile_per_column * 2 + tile_per_column * 3;
    landscape->grid.tile_count *= BVR_BUFFER_COUNT(landscape->grid.layers);

    landscape->grid.tiles = malloc(landscape->grid.tile_count * sizeof(struct bvr_tile_s));
    BVR_ASSERT(landscape->grid.tiles);

    landscape->grid.layers.data = calloc(landscape->grid.layers.size, sizeof(char));
    BVR_ASSERT(landscape->grid.layers.data);

    for (size_t i = 0; i < landscape->grid.tile_count; i++)
    {
        landscape->grid.tiles[i].texture = 1;
        landscape->grid.tiles[i].altitude = 0;
        landscape->grid.tiles[i].norm_x = 0;
        landscape->grid.tiles[i].norm_y = 0;
    }
    
    bvr_mesh_buffer_t vertex_buffer;
    vertex_buffer.data = (char*) landscape->grid.tiles;
    vertex_buffer.type = BVR_INT32;
    vertex_buffer.count = landscape->grid.tile_count;

    bvr_mesh_buffer_t element_buffer;
    element_buffer.data = NULL;
    element_buffer.type = BVR_UNSIGNED_INT32;
    element_buffer.count = landscape->grid.tile_count;

    if(!bvr_create_meshv(landscape->mesh, &vertex_buffer, &element_buffer, BVR_MESH_ATTRIB_SINGLE)){
        return BVR_FALSE;
    }

    // create layers
    bvr_destroy_pool(&landscape->mesh->vertex_groups);
    bvr_create_pool(&landscape->mesh->vertex_groups, sizeof(bvr_vertex_group_t), BVR_BUFFER_COUNT(landscape->grid.layers));
    for (size_t i = 0; i < BVR_BUFFER_COUNT(landscape->grid.layers); i++)
    {
        bvr_vertex_group_t* group = bvr_pool_alloc(&landscape->mesh->vertex_groups);
        group->name.length = 0;
        group->name.string = NULL;
        group->element_offset = (landscape->grid.tile_count / BVR_BUFFER_COUNT(landscape->grid.layers)) * i;
        group->element_count = (landscape->grid.tile_count / BVR_BUFFER_COUNT(landscape->grid.layers));
        group->texture = 0;

        BVR_IDENTITY_MAT4(group->matrix);

        bvr_create_string(&((struct bvr_landscape_layer_s*)landscape->grid.layers.data)[i].name, NULL);
    }

    return BVR_TRUE;
}

static int bvri_landscapejson(bvr_landscape_t* landscape, FILE* file){
    json_object* json_root = NULL;
    json_object* json_width = NULL;
    json_object* json_height = NULL;
    json_object* json_tilewidth = NULL;
    json_object* json_tileheight = NULL;
    json_object* json_layer = NULL;
    json_object* json_layers = NULL;
    json_object* json_tiles = NULL;
    
    bvr_string_t buffer;

    // file reading
    {
        fseek(file, 0, SEEK_SET);

        json_tokener* token = json_tokener_new();

        bvr_string_t file_string;

        bvr_create_string(&file_string, NULL);
        bvr_read_file(&file_string, file);

        json_root = json_tokener_parse_ex(token, file_string.string, file_string.length);
        
        bvr_destroy_string(&file_string);
    }

    if(!json_root){
        BVR_PRINT("failed to read file!");
        return BVR_FALSE;
    }

    json_width = json_object_object_get(json_root, "width");
    json_height = json_object_object_get(json_root, "height");
    json_tilewidth = json_object_object_get(json_root, "tilewidth");
    json_tileheight = json_object_object_get(json_root, "tileheight");
    json_layers = json_object_object_get(json_root, "layers");

    BVR_ASSERT(json_width && json_height && json_layers);

    // check for sizes
    if (!(json_object_get_int(json_width) == landscape->grid.tile_per_row &&
        json_object_get_int(json_height) == landscape->grid.tile_per_column) || 
        !landscape->grid.tiles){
        
        bvr_create_landscape_empty(landscape, 
            json_object_get_int(json_width), 
            json_object_get_int(json_height), 
            (vec2){json_object_get_int(json_tilewidth), json_object_get_int(json_tileheight)}, 
            json_object_get_int(json_layers)
        );
    }

    // if we cannot get tiles informations
    if(!landscape->grid.tiles){
        BVR_PRINT("failed to read tiles informations!");
        return BVR_FALSE;
    }

    // iterate through layers
    for (size_t i = 0; i < json_object_array_length(json_layers); i++)
    {
        json_layer = json_object_array_get_idx(json_layers, i);
        json_width = json_object_object_get(json_layer, "width");
        json_height = json_object_object_get(json_layer, "height");
        json_tiles = json_object_object_get(json_layer, "data");
    
        buffer.string = NULL;
        buffer.length = 0;

        // check for sizes
        if (json_object_get_int(json_width) > landscape->grid.tile_per_row &&
            json_object_get_int(json_height) > landscape->grid.tile_per_column){
            
            BVR_PRINT("invalid layer size!");
            return BVR_FALSE;
        }

        uint32 compression = bvr_hash(json_object_get_string(json_object_object_get(json_layer, "compression")));
        uint32 encoding = bvr_hash(json_object_get_string(json_object_object_get(json_layer, "encoding")));

        // when there is no compression
        if(compression == 0){
            // no compression

            // base 64 encoding hash checking
            //  = "base64"
            if(encoding == -1396204209){
                buffer.string = bvr_base64_decode(
                    json_object_get_string(json_tiles),
                    json_object_get_string_len(json_tiles),
                    (size_t*) &buffer.length
                );
            }
            else {
                BVR_ASSERT(0 && "unsupported landscape encoding");
            }

        }
        else {
            BVR_ASSERT(0 && "unsupported landscape compression");
        }

        // copy data
        if(buffer.string){
            uint32 target = 0;
            const uint32 vertices_per_row = landscape->grid.tile_per_row * 2.0f + 3.0f;
            
            for (size_t id = 0; id < landscape->grid.tile_count; id++)
            {                
                target = bvr_landscape_tile_id(landscape, id);

                // trying to not overflow
                target = MIN(target, buffer.length);

                landscape->grid.tiles[id + 1].altitude = 0;
                landscape->grid.tiles[id + 1].texture = ((unsigned int*)buffer.string)[target] - 1;
                landscape->grid.tiles[id + 1].norm_x = 0;
                landscape->grid.tiles[id + 1].norm_y = 0;
            }       
        }

        bvr_destroy_string(&buffer);
    }

    for (size_t i = 0; i < BVR_BUFFER_COUNT(landscape->grid.layers); i++)
    {
        bvri_update_landscape_buffers(landscape, i);
    }
    
    return BVR_TRUE;
}

int bvr_create_landscapef(bvr_landscape_t* landscape, FILE* file){
    BVR_ASSERT(landscape);
    BVR_ASSERT(file);

    fseek(file, 0, SEEK_SET);

    int status = 0;
    char header = bvr_freadu8_le(file);

    // if this is a json file
    if(header == '{'){
        status |= bvri_landscapejson(landscape, file);
    }
    else {
        BVR_PRINT("invalid landscape format. file might be corrupted or the format is unknown...");
        return false;
    }

    return status;
}

void bvr_destroy_landscape(bvr_landscape_t* landscape){
    BVR_ASSERT(landscape);

    free(landscape->grid.tiles);
    free(landscape->grid.layers.data);
    landscape->grid.tiles = NULL;
    landscape->grid.layers.data = NULL;
}

/*
void bvr_create_landscape_grid(bvr_landscape_actor_t* actor, struct bvr_landscape_grid_s* grid){
    BVR_ASSERT(actor);
    BVR_ASSERT(grid);

    struct bvr_tile_s generic_tile;
    generic_tile.texture = 1;
    generic_tile.altitude = 0;
    generic_tile.norm_x = 0;
    generic_tile.norm_y = 0;

    grid->tile_count = grid->count[0] * grid->count[1] * 2 + grid->count[1] * 3;
    grid->tile_count *= MAX(1, grid->layers);

    // if the actor's grid is not the same we copy it
    if(&actor->grid != grid){
        actor->grid = *grid;
    }

    actor->grid.tiles = malloc(grid->tile_count * sizeof(struct bvr_tile_s));
    BVR_ASSERT(actor->grid.tiles);

    // set all tiles to zero
    for (size_t i = 0; i < actor->grid.tile_count; i++)
    {
        actor->grid.tiles[i] = generic_tile;
    }
    
    bvr_mesh_buffer_t vertices_buffer;
    vertices_buffer.data = (char*) actor->grid.tiles;
    vertices_buffer.type = BVR_INT32;
    vertices_buffer.count = actor->grid.tile_count;

    bvr_mesh_buffer_t element_buffer;
    element_buffer.data = (char*) NULL;
    element_buffer.type = BVR_UNSIGNED_INT32;
    element_buffer.count = actor->grid.tile_count;

    bvr_create_meshv(&actor, &vertices_buffer, &element_buffer, BVR_MESH_ATTRIB_SINGLE);

    // create layers
    bvr_destroy_pool(&actor->mesh.vertex_groups);
    bvr_create_pool(&actor->mesh.vertex_groups, sizeof(bvr_vertex_group_t), actor->grid.layers);
    for (size_t i = 0; i < actor->grid.layers; i++)
    {
        bvr_vertex_group_t* group = bvr_pool_alloc(&actor->mesh.vertex_groups);
        group->name.length = 0;
        group->name.string = NULL;
        group->element_offset = (actor->grid.tile_count / actor->grid.layers) * i;
        group->element_count = (actor->grid.tile_count / actor->grid.layers);
        group->texture = 0;

        BVR_IDENTITY_MAT4(group->matrix);
    }
}



int bvr_landscape_loadf(bvr_landscape_actor_t* actor, FILE* file){
    BVR_ASSERT(actor);
    BVR_ASSERT(file);

    if(!actor->mesh.vertex_buffer){
        BVR_PRINT("landscape should be intialized before !");
        return BVR_FALSE;
    }

    fseek(file, 0, SEEK_SET);
    
    char header = bvr_freadu8_le(file);
    
    if(header == '{'){
        bvri_landscapejson(actor, file);
    }
    else {
        BVR_PRINT("file might be corrupted or use an unknown format!");
        return BVR_FALSE;
    }

    return BVR_TRUE;
}

void bvr_destroy_landscape(struct bvr_landscape_grid_s* grid){
    BVR_ASSERT(grid);

    free(grid->tiles);
    grid->tiles = NULL;
}*/