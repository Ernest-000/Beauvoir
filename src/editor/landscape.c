#include <BVR/editor/landscape.h>
#include <BVR/editor/editor.h>

#include <BVR/window.h>

#include <GLAD/glad.h>

int bvri_landscapeselect(struct bvr_editor_s* editor, bvr_landscape_actor_t* actor){
    // select tile by double clicking on a tile
    if(bvr_button_double_pressed(BVR_MOUSE_BUTTON_LEFT) && !editor->device.is_gui_hovered){
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

struct bvr_tile_s bvri_landscapegettile(bvr_landscape_actor_t* actor, int id){
    int* vertices = NULL;
    struct bvr_tile_s tile;

    if(!id && id > actor->mesh.vertex_count){
        return tile;
    }

    glBindBuffer(GL_ARRAY_BUFFER, actor->mesh.vertex_buffer);
    
    vertices = glMapBufferRange(GL_ARRAY_BUFFER, 0, actor->mesh.vertex_count * sizeof(int), GL_MAP_READ_BIT);
    if(vertices){
        tile = ((struct bvr_tile_s*)vertices)[id];
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return tile;
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