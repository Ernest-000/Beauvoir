#include <bvr/bvr.h>
#include <bvr/scripts.h>

#include <stdio.h>

int main(void){
    bvr_assembly_t assembly;
    bvr_create_assembly(&assembly, "");
    bvr_assembly_add(&assembly, "scripts/hello.c");

    bvr_run(&assembly);

    bvr_destroy_assembly(&assembly);
}