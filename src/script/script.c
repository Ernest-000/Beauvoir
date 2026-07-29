#include <bvr/script/script.h>
#include <bvr/script/sym.h>

#include <bvr/io.h>

#include <tinycc/libtcc.h>
#include <tinycc/tcc.h>

#define BVRI_ENTRY_POINT_NAME "main"

BVR_H_FUNC void* foo(void* entry){
    BVR_CALL((bvr_entry_clbk)entry);
    return NULL;
}

int bvr_create_assembly(bvr_assembly_t* assembly, struct bvr_assembly_attributes_s* attributes){
    BVR_ASSERT(assembly);
    BVR_ASSERT(attributes);

    assembly->handle = NULL;
    assembly->entry = NULL;
    assembly->srcf_count = 0;

    bvr_create_string(&assembly->name, attributes->name);
    bvr_create_string(&assembly->entry_name, attributes->entry_name);
    
    if(assembly->entry_name.string == NULL){
        // if no entry point is set
        // we set it as the default one
        bvr_create_string(&assembly->entry_name, BVRI_ENTRY_POINT_NAME);
    }

    assembly->handle = tcc_new();
    BVR_ASSERT(assembly->handle);

    TCCState* state = ((TCCState*)assembly->handle);

    // set output type
    tcc_set_output_type(state, TCC_OUTPUT_MEMORY);

    // include path
    if(bvr_direxists(attributes->include_path)){
        BVR_PRINTF("added include path '%s'", attributes->include_path);
        tcc_add_include_path(state, attributes->include_path);
    }
    
    // standard config
    tcc_set_options(state, "-nostdlib");

    tcc_define_symbol(state, "CONFIG_TCCDIR", "\"\"");

    // register native symbols here
    bvr_load_default_sym(assembly->handle);

    // source file
    if(bvr_fexists(attributes->source_file)) {
        if(tcc_add_file(state, attributes->source_file) != -1) {
            assembly->srcf_count++;

            BVR_PRINTF("set source file as '%s'", attributes->source_file);
        }
    }
}

int bvr_assembly_run(bvr_assembly_t* assembly){
    BVR_ASSERT(assembly);

    if(assembly->srcf_count == 0){
        BVR_PRINT("failed to run assembly with no linked source file!");
        return BVR_FALSE;
    }

    if(tcc_relocate((TCCState*)assembly->handle) == -1){
        BVR_PRINT("failed to relocate assembly");
        return BVR_FALSE;
    }
    
    assembly->entry = tcc_get_symbol((TCCState*)assembly->handle, assembly->entry_name.string);
    if(assembly->entry == NULL){
        BVR_PRINT("failed to run scripts!");
        return BVR_FALSE;
    }

    pthread_create(&assembly->thread, NULL, foo, assembly->entry);

    return BVR_TRUE;
}

void bvr_destroy_assembly(bvr_assembly_t* assembly){
    BVR_ASSERT(assembly);
    
    pthread_join(assembly->thread, NULL);

    tcc_delete((TCCState*)assembly->handle);
    bvr_destroy_string(&assembly->name);

    assembly->handle = NULL;
    assembly->entry = NULL;
}