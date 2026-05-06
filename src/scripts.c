#include <bvr/scripts.h>

#include <libtcc.h>
#include <tcc.h>

#define BVRI_INCLUDE_PATH "scripts/include/"
#define BVRI_INCLUDE_PATH2 "scripts/"
#define BVRI_ENTRY "entry"

int bvr_create_assembly(bvr_assembly_t* assembly, const char* name){
    BVR_ASSERT(assembly);
    BVR_ASSERT(name);

    assembly->handle = NULL;
    assembly->main_clbk = NULL;

    bvr_create_string(&assembly->name, name);

    assembly->handle = tcc_new();
    BVR_ASSERT(assembly->handle);

    TCCState* state = ((TCCState*)assembly->handle);

    // set output type
    tcc_set_output_type(state, TCC_OUTPUT_MEMORY);

    // include path
    tcc_add_include_path(state, BVRI_INCLUDE_PATH);
    
    // standard config
    tcc_set_lib_path(state, "");
    tcc_define_symbol(state, "CONFIG_TCCDIR", "\"\"");

    // register native symbols here
    bvr_load_c_symbols(assembly->handle);

    return BVR_TRUE;
}

int bvr_assembly_add(bvr_assembly_t* assembly, const char* path){
    BVR_ASSERT(assembly);
    BVR_ASSERT(path);
    
    if(tcc_add_file((TCCState*)assembly->handle, path) == -1){
        BVR_PRINTF("failed to add script '%s'", path);
        return BVR_FALSE;
    }

    return BVR_TRUE;
}

int bvr_run(bvr_assembly_t* assembly){
    BVR_ASSERT(assembly);

    tcc_relocate((TCCState*)assembly->handle);
    
    assembly->main_clbk = tcc_get_symbol((TCCState*)assembly->handle, BVRI_ENTRY);
    if(assembly->main_clbk == NULL){
        BVR_PRINT("failed to run scripts!");
        return BVR_FALSE;
    }

    BVR_CALL(assembly->main_clbk);

    return BVR_TRUE;
}

void bvr_destroy_assembly(bvr_assembly_t* assembly){
    BVR_ASSERT(assembly);

    tcc_delete((TCCState*)assembly->handle);
    bvr_destroy_string(&assembly->name);

    assembly->handle = NULL;
    assembly->main_clbk = NULL;
}