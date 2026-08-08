#include <bvr/script/script.h>
#include <bvr/script/sym.h>

#include <bvr/io.h>

#include <tinycc/libtcc.h>
#include <tinycc/tcc.h>

#define BVRI_ENTRY_POINT_NAME "main"
#define BVRI_DRAW_POINT_NAME "draw"

static pthread_mutex_t __assembly_mutex = PTHREAD_MUTEX_INITIALIZER;

static void* bvri_call_thread(void* entry){
    pthread_mutex_lock(&__assembly_mutex);
    
    BVR_CALL((bvr_entry_clbk)entry);
    
    pthread_mutex_unlock(&__assembly_mutex);
    
    return NULL;
}

static void bvri_assembly_error_clbk(void* opaque, const char* message){
    BVR_PRINT(message);
}

static TCCState* bvri_create_new_state(){
    TCCState* state = tcc_new();
    BVR_ASSERT(state);

    // set output type
    tcc_set_error_func(state, NULL, bvri_assembly_error_clbk);

    tcc_set_output_type(state, TCC_OUTPUT_MEMORY);

    // standard config (do not use standard lib here, we implement our own)
    tcc_set_options(state, "-nostdlib");

#if defined(BVR_DEBUG)
    // enable debug warnings
    tcc_set_options(state, "-Wall");
#endif

    tcc_define_symbol(state, "CONFIG_TCCDIR", "\"\"");


    return state;
}

static void* bvri_register_entry(bvr_assembly_t* assembly, const char* name){
    BVR_ASSERT(assembly);
    
    // no op
    if(name == NULL){
        return NULL;
    }

    void* entry = NULL;

    entry = bvr_table_get(&assembly->entries, name);
    if(entry != NULL){
        // when the symbols is already added in the table
        return (*((void**)entry));
    }

    // when we need to register a new sym.
    entry = tcc_get_symbol((TCCState*)assembly->handle, name);
    if(entry == NULL){
        BVR_PRINTF("cannot find symbole '%s'", name);
        return BVR_FALSE;
    } 

    // when we cannot add it because the table is full
    if(assembly->entries.count >= assembly->entries.capacity){
        return entry;
    }
    
    void** entry_p = bvr_table_set(
        &assembly->entries,
        name, &entry_p
    );
    BVR_ASSERT(entry_p);

    *entry_p = entry;
    return (*entry_p);
}

int bvr_create_assembly(bvr_assembly_t* assembly, struct bvr_assembly_attributes_s* attributes){
    BVR_ASSERT(assembly);
    BVR_ASSERT(attributes);

    assembly->handle = NULL;
    assembly->srcf_count = 0;

    bvr_create_string(&assembly->name, attributes->name);
    bvr_create_table(&assembly->entries, sizeof(void*), BVR_MAX_ENTRIES);

    TCCState* state = bvri_create_new_state(); 
    assembly->handle = state;

    // include path
    if(bvr_direxists(attributes->include_path)){
        BVR_PRINTF("added include path '%s'", attributes->include_path);
        tcc_add_include_path(state, attributes->include_path);
    }

    // register native symbols here
    bvr_load_default_sym(assembly->handle);

    // source file
    if(bvr_fexists(attributes->source_file)) {
        if(tcc_add_file(state, attributes->source_file) != -1) {
            assembly->srcf_count++;

            BVR_PRINTF("set source file as '%s'", attributes->source_file);
        }
    }

    if(tcc_relocate(state) == -1){
        BVR_PRINT("failed to relocate assembly");
        return BVR_FALSE;
    }

    if(attributes->entry_func){
        bvri_register_entry(assembly, attributes->entry_func);
    }

    if(attributes->draw_func){
        bvri_register_entry(assembly, attributes->draw_func);
    }

    return BVR_TRUE;
}

int bvr_assembly_reload(bvr_assembly_t* assembly, struct bvr_assembly_attributes_s* attributes){
    /* reload code here */
    BVR_ASSERT(assembly);

    TCCState* state = bvri_create_new_state(); 
    assembly->srcf_count = 0;

    // include path
    if(bvr_direxists(attributes->include_path)){
        BVR_PRINTF("added include path '%s'", attributes->include_path);
        tcc_add_include_path(state, attributes->include_path);
    }

    // register native symbols here
    bvr_load_default_sym(state);

    // source file
    if(bvr_fexists(attributes->source_file)) {
        if(tcc_add_file(state, attributes->source_file) != -1) {
            assembly->srcf_count++;

            BVR_PRINTF("set source file as '%s'", attributes->source_file);
        }
    }

    if(tcc_relocate(state) == -1){
        tcc_delete(state);
        BVR_PRINT("failed to relocate assembly");
        return BVR_FALSE;
    }

    // destroy the previous
    bvr_destroy_table(&assembly->entries);
    tcc_delete((TCCState*)assembly->handle);

    assembly->handle = state;
    bvr_create_table(&assembly->entries, sizeof(void*), BVR_MAX_ENTRIES);

    if(attributes->entry_func){
        bvri_register_entry(assembly, attributes->entry_func);
    }

    if(attributes->draw_func){
        bvri_register_entry(assembly, attributes->draw_func);
    }
}

int bvr_assembly_run(bvr_assembly_t* assembly, const char* function){
    BVR_ASSERT(assembly);
    
    // get the entry point function
    void* sym = bvri_register_entry(assembly, function);
    if(sym){
        bvri_call_thread(sym);
        // if(assembly->thread){
        //     pthread_join(assembly->thread, NULL);
        // }
        // 
        // pthread_create(&assembly->thread, NULL, bvri_call_thread, sym);
        return BVR_TRUE;
    }

    return BVR_FALSE;
}

void bvr_destroy_assembly(bvr_assembly_t* assembly){
    BVR_ASSERT(assembly);

    if(assembly->thread){
        pthread_join(assembly->thread, NULL);
    }

    tcc_delete((TCCState*)assembly->handle);
    bvr_destroy_string(&assembly->name);
    bvr_destroy_table(&assembly->entries);

    assembly->handle = NULL;
}