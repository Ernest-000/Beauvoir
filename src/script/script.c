#include <bvr/script/script.h>

#include <tinycc/libtcc.h>
#include <tinycc/tcc.h>

int bvr_create_assembly(bvr_assembly_t* assembly, struct bvr_assembly_attributes_s* attributes){
    BVR_ASSERT(assembly);
    BVR_ASSERT(attributes);

    assembly->handle = NULL;
    assembly->entry = NULL;

    bvr_create_string(&assembly->name, attributes->name);

    assembly->handle = tcc_new();
    BVR_ASSERT(assembly->handle);

    TCCState* state = ((TCCState*)assembly->handle);

    // set output type
    tcc_set_output_type(state, TCC_OUTPUT_MEMORY);

    // include path
    // tcc_add_include_path(state, BVRI_INCLUDE_PATH);
    
    // standard config
    tcc_set_lib_path(state, "");
    tcc_define_symbol(state, "CONFIG_TCCDIR", "\"\"");

    // register native symbols here
    //bvr_load_c_symbols(assembly->handle);
}

void bvr_destroy_assembly(bvr_assembly_t* assembly){

}