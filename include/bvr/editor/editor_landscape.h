#include <bvr/editor/editor.h>

#ifndef BVR_NO_NUKLEAR

#include <bvr/math.h>
#include <bvr/actors.h>


/*
    try to find current working tile.
    return tile's id
*/
int bvri_landscape_process_selection(struct bvr_editor_s* editor, bvr_landscape_actor_t* actor);

/*
    try to get tile's values
*/
struct bvr_tile_s bvri_landscape_get_tile(bvr_landscape_actor_t* actor, int id);

#endif