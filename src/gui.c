#include <BVR/gui.h>
#include <BVR/utils.h>

#ifdef BVR_INCLUDE_NUKLEAR

#define NK_SDL_GLES2_IMPLEMENTATION
#include "nuklear_sdl_gles2.h"

int bvr_create_nuklear(bvr_nuklear_t* nuklear, bvr_window_t* window){
    BVR_ASSERT(nuklear);
    BVR_ASSERT(window);
    
    nuklear->antialiasing = NK_ANTI_ALIASING_ON;
    nuklear->vertex_buffer_length = 512 * 1024;
    nuklear->element_buffer_length = 128 * 1024;

    nuklear->window = window;
    nuklear->scale = 1.0f;
    nuklear->context = nk_sdl_init(window->handle, window->framebuffer.width, window->framebuffer.height);

    nk_sdl_font_stash_begin(&nuklear->atlas);
    /*
        include new fonts
    */
    nk_sdl_font_stash_end();
}

void bvr_nuklear_handle(bvr_nuklear_t* nuklear){
    nk_input_begin(nuklear->context);

    if(((nuklear->window->events & SDL_EVENT_KEY_DOWN) == SDL_EVENT_KEY_DOWN) ||
        (nuklear->window->events & SDL_EVENT_KEY_UP) == SDL_EVENT_KEY_UP){
        const uint8* state = (Uint8*)SDL_GetKeyboardState(0);
        
        if(nuklear->window->inputs.keys[BVR_KEY_RIGHT_SHIFT] ||
           nuklear->window->inputs.keys[BVR_KEY_LEFT_SHIFT]) {
            nk_input_key(nuklear->context, NK_KEY_SHIFT, 
                NK_MAX(nuklear->window->inputs.keys[BVR_KEY_LEFT_SHIFT] - 1, 
                    nuklear->window->inputs.keys[BVR_KEY_RIGHT_SHIFT] - 1)
            );

            if(nuklear->window->inputs.keys[BVR_KEY_Z]){
                nk_input_key(nuklear->context, NK_KEY_TEXT_UNDO, 
                    nuklear->window->inputs.keys[BVR_KEY_Z] - 1);
            }
            if(nuklear->window->inputs.keys[BVR_KEY_R]){
                nk_input_key(nuklear->context, NK_KEY_TEXT_REDO, 
                    nuklear->window->inputs.keys[BVR_KEY_R] - 1);
            }
            if(nuklear->window->inputs.keys[BVR_KEY_C]){
                nk_input_key(nuklear->context, NK_KEY_COPY, 
                    nuklear->window->inputs.keys[BVR_KEY_C] - 1);
            }
            if(nuklear->window->inputs.keys[BVR_KEY_P]){
                nk_input_key(nuklear->context, NK_KEY_PASTE, 
                    nuklear->window->inputs.keys[BVR_KEY_P] - 1);
            }
            if(nuklear->window->inputs.keys[BVR_KEY_X]){
                nk_input_key(nuklear->context, NK_KEY_CUT, 
                    nuklear->window->inputs.keys[BVR_KEY_X] - 1);
            }
        }

        if(nuklear->window->inputs.keys[BVR_KEY_LEFT]){
            int down = nuklear->window->inputs.keys[BVR_KEY_LEFT] - 1;
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(nuklear->context, NK_KEY_TEXT_WORD_LEFT, down);
            else nk_input_key(nuklear->context, NK_KEY_LEFT, down);
        }
        if(nuklear->window->inputs.keys[BVR_KEY_RIGHT]){
            int down = nuklear->window->inputs.keys[BVR_KEY_RIGHT] - 1;
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(nuklear->context, NK_KEY_TEXT_WORD_RIGHT, down);
            else nk_input_key(nuklear->context, NK_KEY_RIGHT, down);
        }
    }

    if(((nuklear->window->events & SDL_EVENT_MOUSE_BUTTON_UP) == SDL_EVENT_MOUSE_BUTTON_UP) ||
        (nuklear->window->events & SDL_EVENT_MOUSE_BUTTON_DOWN) == SDL_EVENT_MOUSE_BUTTON_DOWN){

        float x, y;
        SDL_GetMouseState(&x, &y);

        {
            int down = nuklear->window->inputs.buttons[BVR_MOUSE_BUTTON_LEFT];

            if(nuklear->window->inputs.buttons[BVR_MOUSE_BUTTON_LEFT] == BVR_MOUSE_BUTTON_DOUBLE_PRESSED){
                nk_input_button(nuklear->context, NK_BUTTON_DOUBLE, (int)x, (int)y, down);
            }
            nk_input_button(nuklear->context, NK_BUTTON_LEFT, (int)x, (int)y, down);
        }
        
        nk_input_button(nuklear->context, NK_BUTTON_MIDDLE, (int)x, (int)y, nuklear->window->inputs.buttons[BVR_MOUSE_BUTTON_MIDDLE]);
        nk_input_button(nuklear->context, NK_BUTTON_RIGHT, (int)x, (int)y, nuklear->window->inputs.buttons[BVR_MOUSE_BUTTON_RIGHT]);
    }

    if((nuklear->window->events & SDL_EVENT_MOUSE_MOTION) == SDL_EVENT_MOUSE_MOTION){
        if (((struct nk_context*)nuklear->context)->input.mouse.grabbed) {
            int x = (int)((struct nk_context*)nuklear->context)->input.mouse.prev.x;
            int y = (int)((struct nk_context*)nuklear->context)->input.mouse.prev.y;
            nk_input_motion(nuklear->context, x + nuklear->window->inputs.relative_motion[0], y + nuklear->window->inputs.relative_motion[1]);
        }
        else {
            nk_input_motion(nuklear->context, nuklear->window->inputs.motion[0], nuklear->window->inputs.motion[1]);
        }
    }
    if((nuklear->window->events & SDL_EVENT_TEXT_INPUT) == SDL_EVENT_TEXT_INPUT){
        nk_glyph glyph;
        memcpy(glyph, nuklear->window->inputs.text_input, NK_UTF_SIZE);
        nk_input_glyph(nuklear->context, glyph);
    }

    nk_sdl_handle_grab();
    nk_input_end(nuklear->context);
}

void bvr_nuklear_render(bvr_nuklear_t* nuklear){
    /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
     * with blending, scissor, face culling, depth test and viewport and
     * defaults everything back into a default state.
     * Make sure to either a.) save and restore or b.) reset your own state after
     * rendering the UI. */

    nk_sdl_render(nuklear->antialiasing, 
        nuklear->vertex_buffer_length, 
        nuklear->element_buffer_length, 
        nuklear->scale
    );
}

void bvr_destroy_nuklear(bvr_nuklear_t* nuklear){
    BVR_ASSERT(nuklear);
    
    nk_sdl_shutdown();
}

void bvr_nuklear_actor_label(bvr_nuklear_t* nuklear, struct bvr_actor_s* actor){
    BVR_ASSERT(nuklear);
    if(((struct nk_context*)nuklear->context)->begin){
        bvr_nuklear_vec3_label(nuklear, actor->name.string, actor->transform.position);
    }
}

void bvr_nuklear_vec3_label(bvr_nuklear_t* nuklear, const char* text, float* value){
    BVR_ASSERT(nuklear);
    if(((struct nk_context*)nuklear->context)->begin){
        nk_label(nuklear->context, 
            BVR_FORMAT("%s %f %f %f",
            text, value[0], value[1], value[2]), 
            NK_TEXT_ALIGN_LEFT
        );
    }
}

#endif