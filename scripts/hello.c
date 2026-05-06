#include "include/stdio.h"
#include "include/time.h"

int entry(void){
    time_t timestamp = time(NULL);

    printf("hello, world!\n");
    printf("time is %i\n", timestamp);

    return 0;
}