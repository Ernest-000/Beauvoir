#include <bergon/window.h>

static bgs_window_t window;

int main(void){
    
    bgs_create_window(&window, "Bergon", 800, 800, 0);
    bgs_destroy_window(&window);

    return 0;
}