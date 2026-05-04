#include <bvr/editor/editor_landscape.h>
#include <bvr/editor/editor.h>

#ifndef BVR_NO_NUKLEAR

#include <bvr/file.h>
#include <bvr/window.h>

#include <bvr/gl.h>

/*int bvri_landscape_process_selection(struct bvr_editor_s *editor, bvr_landscape_actor_t *actor)
{
    // select tile by double clicking on a tile
    if (bvr_button_double_pressed(BVR_MOUSE_BUTTON_LEFT) && !editor->device.is_gui_hovered)
    {
        float mouse_tile_x = 0, mouse_tile_y = 0;
        vec2 mouse_pos;
        vec3 world_pos;

        bvr_mouse_position(&mouse_pos[0], &mouse_pos[1]);
        bvr_screen_to_world(&editor->book->page->camera, mouse_pos, world_pos);

        mouse_tile_x = MAX(world_pos[0] - actor->self.transform.position[0], 0.0f) / actor->grid.size[0];
        mouse_tile_y = MAX(-world_pos[1] - actor->self.transform.position[1], 0.0f) / actor->grid.size[1];

        editor->memory.landscape.cursor[0] = (uint32)(MIN((int)mouse_tile_x, (int)actor->grid.count[0] - 1));
        editor->memory.landscape.cursor[1] = (uint32)(MIN((int)mouse_tile_y, (int)actor->grid.count[1] - 1));
    }

    // apply y axis
    const int vertices_per_row = actor->grid.count[0] * 2 + 3;

    int target_tile = editor->memory.landscape.cursor[1] * vertices_per_row;
    // apply x axis
    target_tile += (int)clamp(editor->memory.landscape.cursor[0] * 2.0f, 0.0f, vertices_per_row - 2.0f);
    // start offset
    target_tile += 3;

    return target_tile;
}*/

/*struct bvr_tile_s bvri_landscape_get_tile(bvr_landscape_actor_t *actor, int id)
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
}*/

#endif