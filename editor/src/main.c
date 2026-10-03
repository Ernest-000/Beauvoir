#include <bergson/window.h>

static bgs_window_t window;

void _create(struct bgs_window_s* self){
    // bvr_create_page(self->components.viewport.page, "scene.json");
}

void _tick(struct bgs_window_s* self, float delta){
    
}

void _destroy(struct bgs_window_s* self){
    
}

int main(void){

    // override calls
    window.events.on_create = _create;
    window.events.on_tick = _tick;
    window.events.on_destroy = _destroy;
    
    bgs_create_window(&window, "Bergon", 1000, 800, 0);

    bgs_destroy_window(&window);

    return 0;
}