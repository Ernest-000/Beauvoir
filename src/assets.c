#include <bvr/assets.h>
#include <bvr/io.h>
#include <bvr/math.h>

#include <json-c/json.h>

#include <bvr/actors.h>

static struct {
    char path[128];
    bool used;
} __asset_table[BVR_MAX_ASSET];

bvr_fhandle_t bvr_create_fhandle(const char* path){
    bvr_fhandle_t fhandle = {.path = -1};

    for (size_t i = 0; i < BVR_MAX_ASSET; i++)
    {
        if(__asset_table[i].used && BVR_STRCMP(__asset_table[i].path, path)){
            // if the file is already handle
            fhandle.path = i;
            return fhandle;
        }

        if(__asset_table[i].used == 0){
            BVR_STRCPY(
                __asset_table[i].path, path, 
                MIN(strlen(path), sizeof(__asset_table[i].path))
            );
            __asset_table[i].used = true;

            fhandle.path = i;
            return fhandle;
        }
    }
    
    return fhandle;
}

bvr_phandle_t bvr_create_literal_phandle(uint8 size, void* value){
    bvr_phandle_t h;
    h.origin = BVR_PHANDLE_LITERAL;
    h.pointer.literal.size = size;
    
    memcpy(
        h.pointer.literal.value,
        value,
        MIN(size, sizeof(h.pointer.literal.value))
    );
    
    return h;
}

void* bvr_phandle_get(bvr_phandle_t* handle){
    if(!handle){
        return NULL;
    }

    switch (handle->origin)
    {
    case BVR_PHANDLE_FIELD: 
        return (void*)handle->pointer.field.self + (size_t)handle->pointer.field.field.offset;
    case BVR_PHANDLE_LITERAL:
        return (void*)handle->pointer.literal.value;
    case BVR_PHANDLE_RAW:
        return handle->pointer.raw.pointer;
    default:
        return NULL;
    }
}