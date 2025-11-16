#include <BVR/math.h>
#include <BVR/actors.h>

#include <BVR/editor/editor.h>

/*
    Return tile's id from vertex id
*/
BVR_H_FUNC uint32 bvri_landscape_id(bvr_landscape_actor_t* actor, int32 id){
    //
    const uint32 vertices_per_row = actor->dimension.count[0] * 2 + 3;

    return (id - (3 * (id / vertices_per_row))) / 2 - 1;
}

/*
    try to find current working tile.
    return tile's id
*/
int bvri_landscape_process_selection(struct bvr_editor_s* editor, bvr_landscape_actor_t* actor);

/*
    try to get tile's values
*/
struct bvr_tile_s bvri_landscape_get_tile(bvr_landscape_actor_t* actor, int id);

int bvr_landscape_loadf(FILE* file, bvr_landscape_actor_t* actor);

BVR_H_FUNC int bvr_landscape_load(const char* path, bvr_landscape_actor_t* actor){
    BVR_FILE_EXISTS(path);

    FILE* file = fopen(path, "rb");
    int result = bvr_landscape_loadf(file, actor);
    fclose(file);

    return result;
}