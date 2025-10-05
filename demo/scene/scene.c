/*
    This file contains the fundation of every Beauvoir projects.
*/

/* include all Beauvoir's headers */
#define BVR_GEOMETRY_IMPLEMENTATION
#include <BVR/bvr.h>
#include <BVR/editor/editor.h>

/* game's context */
static bvr_book_t book;

/* 
    player's object 
    Because the player is movable, it has to be a dynamic actor.
*/
static bvr_dynamic_actor_t player;

/* editor's context */
static bvr_editor_t editor;

int main(){
    /* create initial game's context */
    bvr_create_book(&book);
    bvr_create_page(&book.page, "scene");

    /* create the window */
    bvr_create_window(&book.window, 800, 800, "Window", 0);
    
    /* link the editor to the game context */
    bvr_create_editor(&editor, &book);
    
    /* create an audio stream */
    bvr_create_audio_stream(&book.audio, BVR_DEFAULT_SAMPLE_RATE, BVR_DEFAULT_AUDIO_BUFFER_SIZE);

    /* create an ortho camera */
    bvr_create_orthographic_camera(&book.page, &book.window.framebuffer, 0.1f, 100.0f, 1.0f);

    {
        /* create player's mesh (here a square) */
        bvr_create_2d_square_mesh(&player.mesh, 20.0f, 20.0f);

        /* 
            create player's shader. 
            the shader has BVR_VERTEX_SHADER and BVR_FRAGMENT_SHADER flags, meaning that it has a vertex and a fragment stage 
        */
        bvr_create_shader(&player.shader, "monochrome.glsl", BVR_VERTEX_SHADER | BVR_FRAGMENT_SHADER);

        /*
            because we want to define player's color inside the shader, we need to define this parameter (=uniform in OpenGL)
            so, we firstly register the uniform (because it's an RGB color we use a vector3).
        */
        bvr_shader_register_uniform(&player.shader, BVR_VEC3, 1, "bvr_color");

        /*
            then, we copy 'color' value into our uniform 'bvr_color'.
        */
        vec3 color = {1.0f, 1.0f, 1.0f};
        bvr_shader_set_uniform(&player.shader, "bvr_color", &color[0]);

        /* create actor components */
        bvr_create_actor(
            &player.object,
            "player",
            BVR_DYNAMIC_ACTOR,
            BVR_COLLISION_ENABLE | /* means that we enable collision */
            BVR_DYNACTOR_AGGRESSIVE | /* means that this actor can respond to physics */
            BVR_DYNACTOR_CREATE_COLLIDER_FROM_BOUNDS /* means that we automaticly create collision boxes based on mesh's vertices */
        );

        /* link this object to a scene */
        bvr_link_actor_to_page(&book.page, &player.object);
    }

    /* main loop */
    while (1)
    {
        /* ask Beauvoir to prepare a new frame */
        bvr_new_frame(&book);

        /* quit the main loop if Beauvoir is not running */
        if(!bvr_is_awake(&book)){
            break;
        }

        // get player's inputs
        {
            /*
                here, we are getting user's inputs
                because bvr_key_down returns a bool, if a key is down it will return 1.
            */
            vec2 inputs;
            float speed = 5.0f;

            inputs[0] = bvr_key_down(BVR_KEY_RIGHT);
            inputs[0] += -bvr_key_down(BVR_KEY_LEFT);
            inputs[1] = bvr_key_down(BVR_KEY_UP);
            inputs[1] += -bvr_key_down(BVR_KEY_DOWN);

            /* we scale input with the speed */
            vec2_scale(inputs, inputs, speed);

            bvr_body_add_force(
                &player.collider.body,
                inputs[0],  /* x axis */
                inputs[1],  /* y axis */
                0           /* z axis */
            );
        }

        /* update collisions and physics */
        bvr_update(&book);

        /* draw player */
        bvr_draw_actor((bvr_static_actor_t*)&player.object, BVR_DRAWMODE_TRIANGLES);

        /* draw editor */
        bvr_editor_handle();
        bvr_editor_draw_page_hierarchy();
        bvr_editor_draw_inspector();
        bvr_editor_render();

        /* push Beauvoir's graphics to the window */
        bvr_render(&book);
    }
    
    /* free */
    bvr_destroy_book(&book);

    return 0;
}