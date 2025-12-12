/*
    This file contains the fundation of every Beauvoir projects.
*/

/* include all Beauvoir's headers */
#define BVR_INCLUDE_GEOMETRY
#include <BVR/bvr.h>

/* game's context */
static bvr_book_t book;

int main(){
    /* create initial game's context */
    bvr_create_book(&book);
    bvr_create_page(&book.page, "empty");

    /* create the window */
    bvr_create_window(&book.window, 800, 800, "Window", BVR_WINDOW_DEFAULT);

    /* Allocate buffers */
    bvr_create_book_memories(&book, BVR_BUFFER_SIZE, 0);
    
    /* create the camera */
    bvr_create_main_camera(&book, BVR_CAMERA_ORTHOGRAPHIC, 0.0f, 1000.0f, 1.0f);

    /* main loop */
    while (1)
    {
        /* ask Beauvoir to prepare a new frame */
        bvr_new_frame(&book);

        /* quit the main loop if Beauvoir is not running */
        if(!bvr_is_awake(&book)){
            break;
        }

        /* update colliders and physics */
        bvr_update(&book);

        /* push Beauvoir's graphics to the window */
        bvr_render(&book);
    }
    
    /* free */
    bvr_destroy_book(&book);

    return 0;
}