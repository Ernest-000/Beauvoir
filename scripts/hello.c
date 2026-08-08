#include "include/assert.h"
#include "include/lbvr.h"

#include "help.c"

void draw(){
    BVR_PRINTF("delta %f", bvr_get_delta_time());
}

int main(void){
    BVR_PRINT("RELOAD");
    return 0;
}