# Beauvoir
> 'Beauvoir' just like Simone de Beauvoir

## Overview
Beauvoir engine is a small game engine developed entirely in C99 with OpenGL ES 2.5 as support. Designed with a minimalist approach, I try to make it compatible with as many platforms as possible. Beauvoir specializes in rendering 2.5D games.
Although the project remains primarily a framework, it is possible to have an interface to facilitate development. It allows you to visualize actors and modify certain aspects in real time!

Beauvoir can handle ```PNG```, ```PSD```, ```TIF``` and ```BMP``` through custom and fast read-only parser but more format still need to be added! It can also load simple ```OBJ``` and ```GLTF``` files (```FBX``` is still in development).

> It's still in early development, but I'm doing my best to improve it!

## Getting Started
Beauvoir can be compiled through **CMake** into Makefiles and Visual Studio solutions. 

You can compile Beauvoir as a shared library and use it with your own project (Cmake templates can be find inside the demo folder). However, you can build it as a single project by using Cmake's flag ```-D BVR_MAIN_FILE=``` (e.g. ```-BVR_MAIN_FILE='demo/image_viewer.c'```) to define your own main file.

### Building for Windows
- To generate Makefiles on Windows, you can run 
```cmake -G="MinGW Makefiles" -B build -D BVR_MAIN_FILE='demo/empty_game.c'```. 

- To generate Visual Studio solutions, use 
```cmake -G="Visual Studio 17 2022" -B build -D BVR_MAIN_FILE='demo/empty_game.c'```. Then, open the generated ```.sln``` file or build using ```cmake --build build --config Release```. After that, link the binaries inside Visual Studio.

### Binaries
Precompiled Windows x64 binaries can be found inside the [Bin](/bin/) directory. But you can build binaries and libraries on your own by using ```build.sh``` or ```build.bat``` scipts.

*You can define your own CMake build target and compiler by changing "BVR_GENERATOR" and "BVR_CC" (or "BVR_CXX") variables in either of those files.*

### Example
In this example, you will get a simple player that move around.
```C
/* include all Beauvoir's headers */
#define BVR_INCLUDE_GEOMETRY
#include <BVR/bvr.h>

/* game's context */
static bvr_book_t book;

/* 
    player's object 
    Because the player is movable, it has to be a dynamic actor.
*/
static bvr_dynamic_actor_t player;

int main(){
    /* create initial game's context */
    bvr_create_book(&book);
    bvr_create_page(&book.page, "scene");

    /* create the window */
    bvr_create_window(&book.window, 800, 800, "Window", 0);

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

        /* create a new actor components */
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

        /* get player's inputs */
        {
            /*
                here, we are getting user's inputs
                because bvr_key_down returns a bool, if a key is down it will return 1.
            */
            vec2 inputs;
            float speed = 5.0f;

            /* get axis inputs (those are default ones) */
            inputs[0] += bvr_axis_down(&book.window.inputs.axis.horizontal);
            inputs[1] += bvr_axis_down(&book.window.inputs.axis.vertical);

            /* we scale input with the speed */
            vec2_scale(inputs, inputs, speed);

            /* create a force to move the player around in the world */
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

        /* push Beauvoir's graphics to the window */
        bvr_render(&book);
    }
    
    /* free */
    bvr_destroy_book(&book);

    return 0;
}
```
> You can find other demos in the [Demo](/demo/) folder.

> Checkout [DOC.md](DOC.md) for more informations!
---

## Third Party 
You can find submodules in the [Extern](/extern/) folder.
- [SDL](https://github.com/libsdl-org/SDL)
- [GLAD](https://glad.dav1d.de/)
- [PortAudio](https://github.com/PortAudio/portaudio)
- [Zlib](https://github.com/madler/zlib)
- [Nuklear](https://github.com/vurtun/nuklear)
- [Libpng16](https://github.com/pnggroup/libpng)
- [Json-c](https://github.com/json-c/json-c)
- [Linmath](https://github.com/datenwolf/linmath.h)