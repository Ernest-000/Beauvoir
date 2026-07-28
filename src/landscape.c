#include <bvr/landscape.h>
#include <bvr/io.h>
#include <bvr/common.h>

#include <zlib.h>
#include <bvr/gl.h>
#include <json-c/json.h>

#define BVR_VERTEX_PER_TILE 2.0f
#define BVR_VERTEX_PER_ROW(tile_per_row) (tile_per_row * BVR_VERTEX_PER_TILE + 2)
#define BVR_GET_INDEX_WITH_X_Y(tile_per_row, x, y) (y * vert_per_row + x * BVR_VERTEX_PER_TILE + 2 + 1)

static void bvri_update_landscape_buffers(bvr_landscape_t* landscape, uint8 layer){
    BVR_ASSERT(landscape);

    if(!landscape->mesh){
        return;
    }

    if(!landscape->mesh->vertex_buffer){
        return;
    }

    uint64 tiles_per_layer = landscape->grid.tile_count / BVR_BUFFER_COUNT(landscape->grid.layers);
    uint64 layer_size = tiles_per_layer * sizeof(struct bvr_tile_s);

    glBindBuffer(GL_ARRAY_BUFFER, landscape->mesh->vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, layer_size * layer, layer_size, landscape->grid.tiles);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return;
}

static void bvri_landscape_set_tile(bvr_landscape_t* landscape, int index, uint8 layer, struct bvr_tile_s tile){
    BVR_ASSERT(landscape);
    BVR_ASSERT(index > 0 && index < landscape->grid.tile_count);

    landscape->grid.tiles[index] = tile;
    landscape->grid.tiles[index + 1] = tile;

    bvri_update_landscape_buffers(landscape, layer);
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
    landscape->grid.layers.size = MAX(1, layer_count) * landscape->grid.layers.elemsize;

    landscape->grid.tile_count = tile_per_column * BVR_VERTEX_PER_ROW(landscape->grid.tile_per_row);
    landscape->grid.tile_count *= BVR_BUFFER_COUNT(landscape->grid.layers);

    landscape->grid.tiles = malloc(landscape->grid.tile_count * sizeof(struct bvr_tile_s));
    BVR_ASSERT(landscape->grid.tiles);

    landscape->grid.layers.data = calloc(landscape->grid.layers.size, sizeof(char));
    BVR_ASSERT(landscape->grid.layers.data);

    for (size_t i = 0; i < landscape->grid.tile_count; i++)
    {
        landscape->grid.tiles[i].texture = 1;
        landscape->grid.tiles[i].altitude = 0;
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

    uint64 tiles_per_layer = landscape->grid.tile_count / BVR_BUFFER_COUNT(landscape->grid.layers);

    // create layers
    bvr_destroy_pool(&landscape->mesh->vertex_groups);
    bvr_create_pool(&landscape->mesh->vertex_groups, sizeof(bvr_vertex_group_t), BVR_BUFFER_COUNT(landscape->grid.layers));
    for (size_t i = 0; i < BVR_BUFFER_COUNT(landscape->grid.layers); i++)
    {
        bvr_vertex_group_t* group = bvr_pool_alloc(&landscape->mesh->vertex_groups);
        group->name.length = 0;
        group->name.string = NULL;
        group->element_offset = tiles_per_layer * i;
        group->element_count = tiles_per_layer;
        group->texture = 0;

        BVR_IDENTITY_MAT4(group->matrix);

        bvr_create_string(&((struct bvr_landscape_layer_s*)landscape->grid.layers.data)[i].name, NULL);
    }

    return BVR_TRUE;
}

static int bvri_load_landscapejson(bvr_landscape_t* landscape, FILE* file){
    json_object* json_root = NULL;
    json_object* json_width = NULL;
    json_object* json_height = NULL;
    json_object* json_tilewidth = NULL;
    json_object* json_tileheight = NULL;
    json_object* json_layer = NULL;
    json_object* json_layers = NULL;
    json_object* json_tiles = NULL;
    json_object* json_compression = NULL;
    json_object* json_encoder = NULL;
    
    struct bvr_buffer_s buffer;

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
            json_object_array_length(json_layers)
        );
    }

    // if we cannot get tiles informations
    if(!landscape->grid.tiles){
        BVR_PRINT("failed to read tiles informations!");
        return BVR_FALSE;
    }

    // iterate through layers
    for (size_t layer_idx = 0; layer_idx < json_object_array_length(json_layers); layer_idx++)
    {
        json_layer = json_object_array_get_idx(json_layers, layer_idx);
        json_width = json_object_object_get(json_layer, "width");
        json_height = json_object_object_get(json_layer, "height");
        json_tiles = json_object_object_get(json_layer, "data");
        json_compression = json_object_object_get(json_layer, "compression");
        json_encoder = json_object_object_get(json_layer, "encoding");
    
        buffer.data = NULL;
        buffer.elemsize = sizeof(uint32);
        buffer.size = 0;

        // check for sizes
        if (json_object_get_int(json_width) > landscape->grid.tile_per_row &&
            json_object_get_int(json_height) > landscape->grid.tile_per_column){
            
            BVR_PRINT("invalid layer size!");
            return BVR_FALSE;
        }

        uint32 compression = 0;
        if(!json_object_is_type(json_compression, json_type_null)){
            compression = bvr_hash(json_object_get_string(json_compression));
        }

        if (json_object_is_type(json_encoder, json_type_null))
        {
            buffer.elemsize = sizeof(uint32);
            buffer.size = json_object_array_length(json_tiles) * buffer.elemsize;
            buffer.data = malloc(buffer.size);
            BVR_ASSERT(buffer.data);

            for (size_t y = 0; y < BVR_BUFFER_COUNT(buffer); y++)
            {
                ((uint32 *)buffer.data)[y] = json_object_get_int(json_object_array_get_idx(json_tiles, y));
            }
        }
        else
        {
            uint32 encoding = bvr_hash(json_object_get_string(json_encoder));
            uint32 unpacked_length = 0;
            uint8* unpacked = NULL;

            // base 64 encoding hash checking
            //  = "base64"
            if (encoding == -1396204209)
            {
                unpacked = bvr_base64_decode(
                    json_object_get_string(json_tiles),
                    json_object_get_string_len(json_tiles),
                    (size_t *)&unpacked_length);

                BVR_ASSERT(unpacked);
            }
            else
            {
                BVR_ASSERT(0 && "unsupported landscape encoding");
            }

            // no compression
            if(compression == 0){
                // no-op
                buffer.elemsize = sizeof(uint8);
                buffer.data = unpacked;
                buffer.size = unpacked_length;
            }
            // zlib compression
            else if (compression == 0x003917CB)
            {
                buffer.elemsize = sizeof(uint8);
                buffer.size = landscape->grid.tile_per_row * landscape->grid.tile_per_column * buffer.elemsize;
                buffer.data = malloc(buffer.size);
                BVR_ASSERT(buffer.data);

                z_stream stream = {0};
                stream.next_in = unpacked;
                stream.avail_in = unpacked_length;
                stream.next_out = buffer.data;
                stream.avail_out = buffer.size;

                BVR_ASSERT(inflateInit(&stream) == Z_OK);

                int status = inflate(&stream, Z_FINISH);
                inflateEnd(&stream);

                BVR_ASSERT(status == Z_STREAM_END);
            }
            else {
                buffer.elemsize = sizeof(uint8);
                buffer.data = unpacked;
                buffer.size = unpacked_length;
            }

            free(unpacked);
            unpacked = NULL;

            for (size_t i = 0; i < landscape->grid.tile_count; i++)
            {
                // On masque les bits de rotation pour avoir l'ID pur
                fprintf(stdout, "%i,", ((uint8*)buffer.data)[i]);
            }
        }

        // copy data
        if (buffer.data)
        {
            const int vert_per_row = BVR_VERTEX_PER_ROW(landscape->grid.tile_per_row);
            const int vert_per_layer = landscape->grid.tile_count / BVR_BUFFER_COUNT(landscape->grid.layers);

            for (size_t y = 0; y < landscape->grid.tile_per_column; y++)
            {
                for (size_t x = 0; x < landscape->grid.tile_per_row; x++)
                {
                    int texture_id = (y * landscape->grid.tile_per_row + x);
                    int layer_offset = vert_per_layer * layer_idx;
                    int vertex_id_top = layer_offset + BVR_GET_INDEX_WITH_X_Y(vert_per_row, x, y);
                    int vertex_id_bot = layer_offset + BVR_GET_INDEX_WITH_X_Y(vert_per_row, x, y) + 1;
                    
                    BVR_ASSERT(texture_id < BVR_BUFFER_COUNT(buffer));
                    BVR_ASSERT(vertex_id_top < landscape->grid.tile_count);
                    //BVR_ASSERT(vertex_id_bot < landscape->grid.tile_count);

                    // first vertex
                    landscape->grid.tiles[vertex_id_top].texture = ((int32*)buffer.data)[texture_id] - 1;
                    landscape->grid.tiles[vertex_id_top].altitude = 0;
                    
                    // next one
                    if(vertex_id_bot < landscape->grid.tile_count){
                        landscape->grid.tiles[vertex_id_bot].texture = ((int32*)buffer.data)[texture_id] - 1;
                        landscape->grid.tiles[vertex_id_bot].altitude = 0;
                    }
                }
            }
        }
        else {
            BVR_PRINT("invalid or empty layer.");
        }

        free(buffer.data);
        buffer.data = NULL;
        buffer.size = 0;
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
        status |= bvri_load_landscapejson(landscape, file);
    }
    else if(header == '<'){
        BVR_PRINT("xml tilemap are not supported!");
        return false;    
    }
    else {
        BVR_PRINT("invalid landscape format. file might be corrupted or the format is unknown...");
        return false;
    }

    return status;
}

void bvr_landscape_set_tile(bvr_landscape_t* landscape, uint32 x, uint32 y, uint8 layer, struct bvr_tile_s tile){
    BVR_ASSERT(landscape);

    int vert_per_row = BVR_VERTEX_PER_ROW(landscape->grid.tile_per_row);
    int vertex_top = BVR_GET_INDEX_WITH_X_Y(vert_per_row, x, y);

    bvri_landscape_set_tile(landscape, vertex_top, layer, tile);
}

void bvr_destroy_landscape(bvr_landscape_t* landscape){
    BVR_ASSERT(landscape);

    free(landscape->grid.tiles);
    free(landscape->grid.layers.data);
    landscape->grid.tiles = NULL;
    landscape->grid.layers.data = NULL;
}