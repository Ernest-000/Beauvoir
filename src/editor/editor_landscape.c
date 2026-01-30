#include <bvr/editor/editor_landscape.h>
#include <bvr/editor/editor.h>

#ifndef BVR_NO_NUKLEAR

#include <bvr/file.h>
#include <bvr/window.h>

#include <glad/glad.h>
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
        bvr_screen_to_world(&editor->book->page->camera, mouse_pos, world_pos);

        mouse_tile_x = MAX(world_pos[0] - actor->self.transform.position[0], 0.0f) / actor->dimension.resolution[0];
        mouse_tile_y = MAX(-world_pos[1] - actor->self.transform.position[1], 0.0f) / actor->dimension.resolution[1];

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
    int* vertices = NULL;
    struct bvr_tile_s tile;

    if (!id && id >= actor->mesh.vertex_count)
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
        return BVR_FALSE;
    }

    json_width = json_object_object_get(json_root, "width");
    json_height = json_object_object_get(json_root, "height");
    json_layers = json_object_object_get(json_root, "layers");

    BVR_ASSERT(json_width && json_height && json_layers);

    // check for sizes
    if (json_object_get_int(json_width) != actor->dimension.count[0] &&
        json_object_get_int(json_height) != actor->dimension.count[1]){
        
        BVR_PRINT("map sizes aren't the same!");
        return BVR_FALSE;
    }

    glBindBuffer(GL_ARRAY_BUFFER, actor->mesh.vertex_buffer);

    // check if memory is correctly unmapped
    tiles = (struct bvr_tile_s*)glMapBufferRange(GL_ARRAY_BUFFER, 0, 
        actor->mesh.vertex_count * sizeof(int), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT  
    );

    // if we cannot get tiles informations
    if(!tiles){
        BVR_PRINTF("failed to read tiles informations! (0x%x)", glGetError());
        return BVR_FALSE;
    }

    // iterate through layers
    for (size_t i = 0; i < json_object_array_length(json_layers); i++)
    {
        json_layer = json_object_array_get_idx(json_layers, i);
        json_width = json_object_object_get(json_layer, "width");
        json_height = json_object_object_get(json_layer, "height");
        json_tiles = json_object_object_get(json_layer, "data");
    
        memory.string = NULL;
        memory.length = 0;

        // check for sizes
        if (json_object_get_int(json_width) != actor->dimension.count[0] &&
            json_object_get_int(json_height) != actor->dimension.count[1]){
            
            BVR_PRINT("layer sizes aren't the same!");
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
                memory.string = bvr_base64_decode(
                    json_object_get_string(json_tiles),
                    json_object_get_string_len(json_tiles),
                    (size_t*) &memory.length
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

    return BVR_TRUE;
}

int bvr_landscape_loadf(FILE* file, bvr_landscape_actor_t* actor){
    BVR_ASSERT(actor);
    BVR_ASSERT(file);

    if(!actor->mesh.vertex_buffer){
        BVR_PRINT("landscape should be loaded before!");
        return BVR_FALSE;
    }

    fseek(file, 0, SEEK_SET);
    
    char header = bvr_freadu8_le(file);
    
    if(header == '{'){
        bvri_landscapejson(file, actor);
    }
    else {
        BVR_PRINT("file might be corrupted or use an unknown format!");
        return BVR_FALSE;
    }

    return BVR_TRUE;
}

#endif