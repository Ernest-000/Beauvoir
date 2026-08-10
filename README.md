# Beauvoir
> 'Beauvoir' just like Simone de Beauvoir

## Overview
Beauvoir engine is a small game engine developed entirely in C99 with OpenGL ES 2.5 as support. Designed with a minimalist approach, I try to make it compatible with as many platforms as possible. Beauvoir specializes in rendering 2.5D games.
Although the project remains primarily a framework, it is possible to have an interface to facilitate development. It allows you to visualize actors and modify certain aspects in real time!

Beauvoir can handle ```PNG```, ```PSD```, ```TIF``` and ```BMP``` through custom and fast read-only parser but more format still need to be added! It can also load simple ```OBJ``` and ```GLTF```.

> It's still in early development, but I'm doing my best to improve it!

## Building
 
### Prerequisites
 
- **CMake** (3.x+)
- **Git**
- **A C compiler** — `gcc`/`g++` (Linux/macOS) or MinGW-w64 `gcc`/`g++` (Windows)
- **Python 3** — used to generate the single-header `nuklear.h`
- **Make** — GNU Make on Linux/macOS, or `mingw32-make`/Ninja on Windows
### Linux / macOS
 
```bash
./build.sh
```
 
### Windows
 
```bat
build.bat
```
 
### Options
 
Both scripts accept the same flags:
 
| Flag               | Description                                    |
|---------------------|------------------------------------------------|
| `--clear`           | Wipe caches, `bin/`, `lib/`, and build artifacts before building |
| `--force`           | Force a rebuild of all modules, even if unchanged |
| `--skip-binaries`   | Skip building third-party libraries (Zlib, json-c) |
| `--skip-git`        | Skip syncing/updating git submodules            |
| `--skip-nk`         | Skip regenerating the `nuklear.h` header        |


## Creating a project 

## Example
In this example, you will get a simple player that move around.
```C
#include <bvr/bvr.h>

static bvr_book_t book;

int main(void){
    // define struct options
    struct bvr_book_attributes_s book_infos = {0};
    book_infos.name = "COUCOU";
    book_infos.window_flags = BVR_WINDOW_DEFAULT;
    
    // create the application
    bvr_create_book_attributes(&book, &book_infos);

    while (true)
    {
        // ask for a new frame
        bvr_new_frame();

        // exist the loop when the application is terminated
        if(BVR_CAN_QUIT()){
            break;
        }

        // render the frame
        bvr_render();
    }
    
    // freeing
    bvr_destroy_book(&book);

    return 0;
}
```
> You can find other demos in the [Demo](/demo/) folder.

> Checkout [DOC.md](DOC.md) for more information!
---

## Third Party 
You can find submodules in the [Extern](/extern/) folder.
- [Zlib](https://github.com/madler/zlib)
- [Nuklear](https://github.com/vurtun/nuklear)
- [Json-c](https://github.com/json-c/json-c)
- [Tinycc](https://github.com/tinycc/tinycc)
- [Linmath](https://github.com/datenwolf/linmath.h)