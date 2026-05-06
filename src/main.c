#include <stdio.h>
#include <malloc.h>

#include <libtcc.h>

static struct script_context_s {
    TCCState* tcc;
    int (*entryp)(void);
} __context;

// engine code

void print(const char* message){
    fprintf(stdout, message);
}

// scripts

size_t fsize(FILE* file){
    size_t size = 0;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

const char* read_file(FILE* file){
    size_t length = fsize(file);
    char* str = malloc(length + 1);
    
    fread(str, sizeof(char), length, file);
    
    str[length] = '\0';
    return (const char*)str;
}

int init_ctx(){
    __context.tcc = tcc_new();
    if(__context.tcc == NULL) {
        return 0;
    }

    tcc_set_output_type(__context.tcc, TCC_OUTPUT_MEMORY);
    tcc_add_include_path(__context.tcc, "include/");
    
    tcc_set_lib_path(__context.tcc, "");
    tcc_define_symbol(__context.tcc, "CONFIG_TCCDIR", "\"\"");

    // register functions here
    tcc_add_symbol(__context.tcc, "print", print);
}

void free_ctx(){
    if(__context.tcc){
        tcc_delete(__context.tcc);
        __context.entryp;
    }
}

int add_script(const char* path){
    if(path == NULL) {
        return 0;
    }

    FILE* f = fopen(path, "rb");
    if(f == NULL) {
        return 0;
    }

    if(tcc_add_file(__context.tcc, path) == -1) {
        fclose(f);
        return 0;
    }

    tcc_relocate(__context.tcc);

    __context.entryp = tcc_get_symbol(__context.tcc, "entry");
    if(__context.entryp == NULL){
        print("no entry point found");
    }

    fclose(f);
    return 1;
}

int main(void){

    init_ctx();
    add_script("scripts/hello.c");

    if(__context.entryp){
        __context.entryp();
    }

    free_ctx();
}