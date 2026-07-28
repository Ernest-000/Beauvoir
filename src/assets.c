#include <bvr/assets.h>
#include <bvr/io.h>

#include <json-c/json.h>

int bvr_load_pagef(bvr_page_t* page, FILE* file){
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

        // clear buffers
        bvr_destroy_string(&file_as_str);
    }

    if(!json_root){
        BVR_PRINT("failed to parse the json page file!");
        return BVR_FALSE;
    }

    
}