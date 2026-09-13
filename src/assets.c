#include <bvr/assets.h>
#include <bvr/io.h>
#include <bvr/math.h>

#include <json-c/json.h>

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

/*int bvr_load_pagef(bvr_page_t* page, FILE* file){
    BVR_ASSERT(page);
    BVR_ASSERT(file);

    json_object* json_root = NULL;

    {
        // read the json file
        fseek(file, 0, SEEK_SET);

        bvr_string_t file_as_str;
        json_tokener* tokener = json_tokener_new();
        bvr_create_string(&file_as_str, NULL);
        bvr_read_file(&file_as_str, file);

        json_root = json_tokener_parse_ex(
            tokener,
            file_as_str.string,
            file_as_str.length
        );

        BVR_PRINT(file_as_str.string);

        // clear buffers
        bvr_destroy_string(&file_as_str);
    }

    if(!json_root){
        BVR_PRINT("failed to parse the json page file!");
        return BVR_FALSE;
    }   

}*/

