#define BVR_INCLUDE_GEOMETRY
#define BVR_MAX_PAGE_COUNT 2

#define BVR_INCLUDE_NUKLEAR
#include <BVR/bvr.h>
#include <BVR/editor/editor.h>

static bvr_book_t book;
static bvr_editor_t editor;

static struct {
    vec3 color;
    bvr_static_actor_t* square;
} scene_a;

static struct {
    vec3 color;
    bvr_static_actor_t* square;
} scene_b;

static void _create_side_a();
static void _create_side_b();

static void _update_side_a(bvr_page_t* self);
static void _update_side_b(bvr_page_t* self);

int main(){
    /* create initial game's context */
    bvr_create_book(&book);

    /* create the window */
    bvr_create_window(&book.window, 800, 800, "Window", BVR_WINDOW_DEFAULT);

    /* Allocate buffers */
    bvr_create_book_memories(&book, BVR_BUFFER_SIZE, 0);
    bvr_create_editor(&editor, &book);

    _create_side_a();

    /* main loop */
    while (1)
    {
        /* ask Beauvoir to prepare a new frame */
        bvr_new_frame(&book);

        /* quit the main loop if Beauvoir is not running */
        if(!bvr_is_awake(&book)){
            break;
        }

        if(bvr_key_down(BVR_KEY_1)){
            _create_side_a();
            continue;
        }

        if(bvr_key_down(BVR_KEY_2)){
            _create_side_b();
            continue;
        }

        /* update colliders and physics */
        bvr_update(&book);

        bvr_flush(&book);

        /* draw editor */
        bvr_editor_new_frame();
        bvr_editor_draw_page_hierarchy();
        bvr_editor_draw_inspector();
        bvr_editor_render();

        /* push Beauvoir's graphics to the window */
        bvr_render(&book);
    }
    
    /* free */
    bvr_destroy_book(&book);
    bvr_destroy_editor(&editor);

    return 0;
}

static void _create_side_a(){
    bvr_create_page(&book.slots[0].page, "Side A");
    bvr_enable_page(&book.slots[0].page);
    
    bvr_create_main_camera(&book, BVR_CAMERA_ORTHOGRAPHIC, 0.0f, 1000.0f, 1.0f);

    book.page->events.construct = NULL;
    book.page->events.load = NULL;
    book.page->events.update = _update_side_a;
    book.page->events.destroy = NULL;

    scene_a.square = (bvr_static_actor_t*) bvr_alloc_actor(book.page, BVR_STATIC_ACTOR);
    bvr_create_actor(&scene_a.square->self, "square", BVR_COLLISION_DISABLE, NULL);

    bvr_create_2d_square_mesh(&scene_a.square->mesh, 20.0f, 20.0f);
    bvr_create_shader(&scene_a.square->shader, "monochrome.glsl", BVR_VERTEX_SHADER | BVR_FRAGMENT_SHADER);
    bvr_shader_register_uniform(&scene_a.square->shader, BVR_VEC3, BVR_UNIFORM_NONE, 1, "bvr_color");

    BVR_CREATE_VEC3(scene_a.color, 1.0f, 1.0f, 1.0f);
    bvr_shader_set_uniform(&scene_a.square->shader, "bvr_color", &scene_a.color[0]);
}

static void _create_side_b(){
    bvr_create_page(&book.slots[1].page, "Side B");
    bvr_enable_page(&book.slots[1].page);
    
    bvr_create_main_camera(&book, BVR_CAMERA_ORTHOGRAPHIC, 0.0f, 1000.0f, 1.0f);

    book.page->events.construct = NULL;
    book.page->events.load = NULL;
    book.page->events.update = _update_side_b;
    book.page->events.destroy = NULL;

    scene_b.square = (bvr_static_actor_t*) bvr_alloc_actor(book.page, BVR_STATIC_ACTOR);
    bvr_create_actor(&scene_b.square->self, "square", BVR_COLLISION_DISABLE, NULL);

    bvr_create_2d_square_mesh(&scene_b.square->mesh, 20.0f, 20.0f);
    bvr_create_shader(&scene_b.square->shader, "monochrome.glsl", BVR_VERTEX_SHADER | BVR_FRAGMENT_SHADER);
    bvr_shader_register_uniform(&scene_b.square->shader, BVR_VEC3, BVR_UNIFORM_NONE, 1, "bvr_color");

    BVR_CREATE_VEC3(scene_b.color, 1.0f, 1.0f, 1.0f);
    bvr_shader_set_uniform(&scene_b.square->shader, "bvr_color", &scene_b.color[0]);
}

static void _update_side_a(bvr_page_t* self){
    bvr_draw_actor(&scene_a.square->self, BVR_DRAWMODE_TRIANGLES);

    BVR_PRINT(self->name.string);
}

static void _update_side_b(bvr_page_t* self){
    bvr_draw_actor(&scene_b.square->self, BVR_DRAWMODE_TRIANGLES);

    BVR_PRINT(self->name.string);
}