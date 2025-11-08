#include <BVR/math.h>
#include <BVR/actors.h>

#include <BVR/editor/editor.h>

#define BVRI_LANDSCAPEID(x) x / 2

/*
    try to find current working tile.
    return tile's id
*/
int bvri_landscapeselect(struct bvr_editor_s* editor, bvr_landscape_actor_t* actor);

/*
    try to get tile's values
*/
struct bvr_tile_s bvri_landscapegettile(bvr_landscape_actor_t* actor, int id);