#include <BVR/editor/landscape.h>
#include <BVR/editor/editor.h>

#include <BVR/file.h>
#include <BVR/window.h>

#include <GLAD/glad.h>
#include <json-c/json.h>

int bvri_landscape_process_selection(struct bvr_editor_s *editor, bvr_landscape_actor_t *actor)
{
    // select tile by double clicking on a tile
    if (bvr_button_double_pressed(BVR_MOUSE_BUTTON_LEFT) && !editor->device.is_gui_hovered)
    {
        float mouse_tile_x = 0, mouse_tile_y = 0;
        vec2 mouse_pos;
        vec3 world_pos;

        bvr_mouse_position(&mouse_pos[0], &mouse_pos[1]);
        bvr_screen_to_world_coords(editor->book, mouse_pos, world_pos);

        mouse_tile_x = MAX(world_pos[0] - actor->object.transform.position[0], 0.0f) / actor->dimension.resolution[0];
        mouse_tile_y = MAX(-world_pos[1] - actor->object.transform.position[1], 0.0f) / actor->dimension.resolution[1];

        editor->memory.landscape.cursor[0] = (uint32)(MIN((int)mouse_tile_x, (int)actor->dimension.count[0] - 1));
        editor->memory.landscape.cursor[1] = (uint32)(MIN((int)mouse_tile_y, (int)actor->dimension.count[1] - 1));
    }

    // apply y axis
    const int vertices_per_row = actor->dimension.count[0] * 2 + 3;

    int target_tile = editor->memory.landscape.cursor[1] * vertices_per_row;
    // apply x axis
    target_tile += (int)clamp(editor->memory.landscape.cursor[0] * 2.0f, 0.0f, vertices_per_row - 2.0f);
    // start offset
    target_tile += 3;

    return target_tile;
}

struct bvr_tile_s bvri_landscape_get_tile(bvr_landscape_actor_t *actor, int id)
{
    int *vertices = NULL;
    struct bvr_tile_s tile;

    if (!id && id > actor->mesh.vertex_count)
    {
        return tile;
    }

    glBindBuffer(GL_ARRAY_BUFFER, actor->mesh.vertex_buffer);

    vertices = glMapBufferRange(GL_ARRAY_BUFFER, 0, actor->mesh.vertex_count * sizeof(int), GL_MAP_READ_BIT);
    if (vertices)
    {
        tile = ((struct bvr_tile_s *)vertices)[id];
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return tile;
}

static int bvri_landscapejson(FILE* file, bvr_landscape_actor_t* actor){
    json_object* json_root = NULL;
    json_object* json_width = NULL;
    json_object* json_height = NULL;
    json_object* json_layer = NULL;
    json_object* json_layers = NULL;
    json_object* json_tiles = NULL;
    struct bvr_tile_s* tiles = NULL;
    
    bvr_string_t memory;

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
        return BVR_FAILED;
    }

    json_width = json_object_object_get(json_root, "width");
    json_height = json_object_object_get(json_root, "height");
    json_layers = json_object_object_get(json_root, "layers");

    BVR_ASSERT(json_width && json_height && json_layers);

    if (json_object_get_int(json_width) != actor->dimension.count[0] &&
        json_object_get_int(json_height) != actor->dimension.count[1]){
        
        BVR_PRINT("map sizes aren't the same!");
        return BVR_FAILED;
    }

    glBindBuffer(GL_ARRAY_BUFFER, actor->mesh.vertex_buffer);
    tiles = (struct bvr_tile_s*)glMapBufferRange(GL_ARRAY_BUFFER, 0, 
        actor->mesh.vertex_count * sizeof(int), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT
    );

    if(!tiles){
        BVR_PRINT("failed to read tiles informations!");
        return BVR_FAILED;
    }

    for (size_t i = 0; i < json_object_array_length(json_layers); i++)
    {
        json_layer = json_object_array_get_idx(json_layers, i);
        json_width = json_object_object_get(json_layer, "width");
        json_height = json_object_object_get(json_layer, "height");
        json_tiles = json_object_object_get(json_layer, "data");
    
        memory.string = NULL;
        memory.length = 0;

        if (json_object_get_int(json_width) != actor->dimension.count[0] &&
            json_object_get_int(json_height) != actor->dimension.count[1]){
            
            BVR_PRINT("layer sizes aren't the same!");
            return BVR_FAILED;
        }

        uint32 compression = bvr_hash(json_object_get_string(json_object_object_get(json_layer, "compression")));
        uint32 encoding = bvr_hash(json_object_get_string(json_object_object_get(json_layer, "encoding")));

        if(compression == 0){
            // no compression

            // base 64 encoding hash checking
            //  = "base64"
            if(encoding == -1396204209){
                memory.string = bvr_base64_decode(
                    json_object_get_string(json_tiles),
                    json_object_get_string_len(json_tiles),
                    (size_t*) &memory.length
                );
            }

        }
        else {
            BVR_ASSERT(0 || "unsupported landscape compression");
        }

        // copy data
        if(memory.string){
            uint32 target = 0;
            const uint32 vertices_per_row = actor->dimension.count[0] * 2.0f + 3.0f;
            
            for (size_t id = 0; id < actor->mesh.vertex_count; id++)
            {                
                target = bvri_landscape_id(actor, id);

                // trying to not overflow
                target = MIN(target, memory.length);

                tiles[id + 1].altitude = 0;
                tiles[id + 1].texture = ((unsigned int*)memory.string)[target] - 1;
                tiles[id + 1].norm_x = 0;
                tiles[id + 1].norm_y = 0;
            }       
        }

        bvr_destroy_string(&memory);
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return BVR_OK;
}

int bvri_landscape_loadf(FILE* file, bvr_landscape_actor_t* actor){
    BVR_ASSERT(actor);
    BVR_ASSERT(file);

    if(!actor->mesh.vertex_buffer){
        BVR_PRINT("landscape should be loaded before!");
        return BVR_FAILED;
    }

    fseek(file, 0, SEEK_SET);
    
    char header = bvr_freadu8_le(file);
    
    if(header == '{'){
        bvri_landscapejson(file, actor);
    }
    else {
        BVR_PRINT("file might be corrupted or use an unknown format!");
        return BVR_FAILED;
    }

    return BVR_OK;
}

/*{
    bvr_texture_t view;
    struct nk_image view_id;
    nk_property_int(__editor->gui.context, "texture id", 0, &__editor->inspector_cmd.user_data3, actor->atlas.tiles.count, 1, .5f);
    bvr_create_view_texture(&actor->atlas.texture, &view, actor->atlas.tiles.width, actor->atlas.tiles.height, __editor->inspector_cmd.user_data3);
    nk_layout_row_dynamic(__editor->gui.context, BVR_INSPECTOR_RECT(0, 0).w / 2, 1);
    view_id = nk_image_id(view.id);
    nk_image(__editor->gui.context, view_id);
    bvr_destroy_texture(&view);
            }*/
/*if(0){
    nk_label(__editor->gui.context, "TILE INFORMATIONS", NK_TEXT_ALIGN_LEFT);

    glBindBuffer(GL_ARRAY_BUFFER, actor->mesh.vertex_buffer);
    int* vertices_map = glMapBufferRange(GL_ARRAY_BUFFER, 0, actor->mesh.vertex_count * sizeof(int), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    if(vertices_map){
        // extract each values through bitwise operations
        int texture_id = (0xff00 & vertices_map[target_tile]) >> 8;
        int altitude[2] = {0, 0};
        altitude[0] = 0xff & vertices_map[target_tile + 0];
        altitude[1] = 0xff & vertices_map[target_tile + 1];
        //TODO: better texture id selection
        nk_property_int(__editor->gui.context, "texture id", 0, &texture_id, actor->atlas.tile_count_x * actor->atlas.tile_count_y, 1, .5f);

        nk_layout_row_dynamic(__editor->gui.context, 15, 2);
        nk_property_int(__editor->gui.context, "Altutide 0", 0, &altitude[0], 255, 1, 1.0f);
        nk_property_int(__editor->gui.context, "Altutide 1", 0, &altitude[1], 255, 1, 1.0f);
        nk_layout_row_dynamic(__editor->gui.context, 15, 1);

        // if values are differents
        if(texture_id != ((0xff00 & vertices_map[target_tile + 0]) >> 8)){
            // write texture id on the two opposites vertices
            // bitmask: remove all previous texture id and overwrite with the new texid value shifted 8 bits
            vertices_map[target_tile + 0] = (vertices_map[target_tile + 0] & ~0xff00) | (texture_id) << 8;
            vertices_map[target_tile + 1] = (vertices_map[target_tile + 1] & ~0xff00) | (texture_id) << 8;
        }
        // check for altitude on the first vertex
        if(altitude[0] != (0xff & vertices_map[target_tile + 0])){
            // try to overwrite the vertex on the other side of the edge (on top of it)
            int prev_row_vertex = (int)clamp(target_tile - vertices_per_row + 1, 0, actor->mesh.vertex_count);
            vertices_map[target_tile + 0] = (vertices_map[target_tile + 0] & ~0xff) | altitude[0];
            vertices_map[prev_row_vertex] = (vertices_map[prev_row_vertex] & ~0xff) | altitude[0];
        }
        if(altitude[1] != (0xff & vertices_map[target_tile + 1])){
            // try to overwrite the vertex on the other side of the edge (bottom)
            int next_row_vertex = (int)clamp(target_tile + vertices_per_row, 0, actor->mesh.vertex_count);

            vertices_map[target_tile + 1] = (vertices_map[target_tile + 1] & ~0xff) | altitude[1];
            vertices_map[next_row_vertex] = (vertices_map[next_row_vertex] & ~0xff) | altitude[1];
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}*/