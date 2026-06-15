#include <bvr/gui.h>
#include <bvr/file.h>

// #include <SDL3/SDL.h>
#include <bvr/gl.h>

#include <malloc.h>

#ifndef BVR_NO_NUKLEAR

static void bvri_clipboard_paste(nk_handle usr, struct nk_text_edit *edit);
static void bvri_clipboard_copy(nk_handle handle, const char *text, int len);

struct bvri_gui_vertex_s {
    vec2 position;
    vec2 uvs;
    uint8 color[4];
};

int bvr_create_canvas(bvr_canvas_t* context, const bvr_book_t* book){
    BVR_ASSERT(context);
    BVR_ASSERT(book);

    context->win = &book->window;

    context->device.alpha = 1.0f;
    context->device.segment_count = 50;
    context->device.vertex_count = BVR_BUFFER_SIZE * BVR_BUFFER_SIZE;
    context->device.element_count = BVR_BUFFER_SIZE * BVR_BUFFER_SIZE;
    context->device.use_antialiasing = NK_ANTI_ALIASING_OFF;
    context->device.scale = 1.0f;

    context->device.copy_event = bvri_clipboard_copy;
    context->device.paste_event = bvri_clipboard_paste;

    context->device.texture_offset = 0;

    context->device.vs = sizeof(struct bvri_gui_vertex_s);
    context->device.vp = offsetof(struct bvri_gui_vertex_s, position);
    context->device.vt = offsetof(struct bvri_gui_vertex_s, uvs);
    context->device.vc = offsetof(struct bvri_gui_vertex_s, color);

    nk_init_default(&context->context, NULL);
    nk_style_default(&context->context);

    nk_buffer_init_default(&context->device.cmds);
    
    // create shader
    {
        static const char* vertex_shader =
           BVR_SHADER_VERSION
           "uniform mat4 bvr_proj;\n"
           "layout(location=0) in vec2 bvr_position;\n"
           "layout(location=1) in vec2 bvr_uvs;\n"
           "layout(location=2) in vec4 bvr_color;\n"
           "out vec2 o_uvs;\n"
           "out vec4 o_color;\n"
           "void main() {\n"
           "   o_uvs = bvr_uvs;\n"
           "   o_color = bvr_color;\n"
           "   gl_Position = bvr_proj * vec4(bvr_position.xy, 0, 1);\n"
           "}\n";

        static const char* fragment_shader =
           BVR_SHADER_VERSION
           "precision mediump float;\n"
           "uniform sampler2D bvr_texture;\n"
           "uniform sampler2DArray bvr_texture_array;\n"
           "uniform float bvr_layer;\n"
           "in vec2 o_uvs;\n"
           "in vec4 o_color;\n"
           "void main(){\n"
           "    if(bvr_layer < 0.0) {\n"
           "        gl_FragColor = o_color * texture(bvr_texture, o_uvs); }\n"
           "    else {"
            "        gl_FragColor = o_color * texture(bvr_texture_array, vec3(o_uvs, bvr_layer)); }\n"
           "}\n";
        
        const char* shaders[2] = {vertex_shader, fragment_shader};

        bvr_create_shader_raw(&context->device.shader, shaders, BVR_VERTEX_SHADER | BVR_FRAGMENT_SHADER);
        
        context->device.projection = bvr_shader_register_uniform(
            &context->device.shader, BVR_MAT4, BVR_UNIFORM_PROJECTION, 1, "bvr_proj"
        );

        context->device.texture = bvr_shader_register_uniform(
            &context->device.shader, BVR_TEXTURE_2D, BVR_UNIFORM_TEXTURE, 1, "bvr_texture"
        );

        context->device.texture_array = bvr_shader_register_uniform(
            &context->device.shader, BVR_TEXTURE_2D_ARRAY, BVR_UNIFORM_TEXTURE, 1, "bvr_texture_array"
        );

        context->device.texture_layer = bvr_shader_register_uniform(
            &context->device.shader, BVR_FLOAT, BVR_UNIFORM_NONE, 1, "bvr_layer"
        );

        glUseProgram(context->device.shader.program);
        glUniform1i(context->device.texture->location, 0);
        glUniform1i(context->device.texture_array->location, 1); 
    }

    // create buffers
    glGenVertexArrays(1, &context->device.vao);
    glGenBuffers(1, &context->device.vbo);
    glGenBuffers(1, &context->device.ebo);

    glBindBuffer(GL_ARRAY_BUFFER, context->device.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context->device.ebo);

    glBufferData(GL_ARRAY_BUFFER, context->device.vertex_count, NULL, GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, context->device.element_count, NULL, GL_STREAM_DRAW);

    // create a new default font
    bvr_create_fontf(&context->default_font, NULL, 14);
    bvr_register_font(context, &context->default_font);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    BVR_ASSERT(context->device.vbo);
    BVR_ASSERT(context->device.ebo);
    BVR_ASSERT(context->context.style.font);
    BVR_ASSERT(context->context.style.font->width);

    return BVR_TRUE;
}

void bvr_destroy_canvas(bvr_canvas_t* context){
    BVR_ASSERT(context);

    bvr_destroy_shader(&context->device.shader);
    bvr_destroy_font(&context->default_font);

    glDeleteVertexArrays(1, &context->device.vao);
    glDeleteBuffers(1, &context->device.vbo);
    glDeleteBuffers(1, &context->device.ebo);

    nk_buffer_free(&context->device.cmds);

    nk_free(&context->context);
}

void bvr_canvas_new_frame(bvr_canvas_t* context){
    BVR_ASSERT(context);

    nk_input_begin(&context->context);

    // context->device.texture_offset = -1;
    /*
    if(((context->win->events & SDL_EVENT_KEY_DOWN) == SDL_EVENT_KEY_DOWN) ||
        (context->win->events & SDL_EVENT_KEY_UP) == SDL_EVENT_KEY_UP){
        const uint8* state = (Uint8*)SDL_GetKeyboardState(0);
        
        if(context->win->inputs.keys[BVR_KEY_RIGHT_SHIFT] ||
           context->win->inputs.keys[BVR_KEY_LEFT_SHIFT]) {
            nk_input_key(&context->context, NK_KEY_SHIFT, 
                NK_MAX(context->win->inputs.keys[BVR_KEY_LEFT_SHIFT] - 1, 
                    context->win->inputs.keys[BVR_KEY_RIGHT_SHIFT] - 1)
            );

            if(context->win->inputs.keys[BVR_KEY_Z]){
                nk_input_key(&context->context, NK_KEY_TEXT_UNDO, 
                    context->win->inputs.keys[BVR_KEY_Z] - 1);
            }
            if(context->win->inputs.keys[BVR_KEY_R]){
                nk_input_key(&context->context, NK_KEY_TEXT_REDO, 
                    context->win->inputs.keys[BVR_KEY_R] - 1);
            }
            if(context->win->inputs.keys[BVR_KEY_C]){
                nk_input_key(&context->context, NK_KEY_COPY, 
                    context->win->inputs.keys[BVR_KEY_C] - 1);
            }
            if(context->win->inputs.keys[BVR_KEY_P]){
                nk_input_key(&context->context, NK_KEY_PASTE, 
                    context->win->inputs.keys[BVR_KEY_P] - 1);
            }
            if(context->win->inputs.keys[BVR_KEY_X]){
                nk_input_key(&context->context, NK_KEY_CUT, 
                    context->win->inputs.keys[BVR_KEY_X] - 1);
            }
        }

        if(context->win->inputs.keys[BVR_KEY_LEFT]){
            int down = context->win->inputs.keys[BVR_KEY_LEFT] - 1;
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(&context->context, NK_KEY_TEXT_WORD_LEFT, down);
            else nk_input_key(&context->context, NK_KEY_LEFT, down);
        }
        if(context->win->inputs.keys[BVR_KEY_RIGHT]){
            int down = context->win->inputs.keys[BVR_KEY_RIGHT] - 1;
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(&context->context, NK_KEY_TEXT_WORD_RIGHT, down);
            else nk_input_key(&context->context, NK_KEY_RIGHT, down);
        }
    }

    if(((context->win->events & SDL_EVENT_MOUSE_BUTTON_UP) == SDL_EVENT_MOUSE_BUTTON_UP) ||
        (context->win->events & SDL_EVENT_MOUSE_BUTTON_DOWN) == SDL_EVENT_MOUSE_BUTTON_DOWN){

        float x, y;
        SDL_GetMouseState(&x, &y);

        {
            int down = context->win->inputs.buttons[BVR_MOUSE_BUTTON_LEFT];

            if(context->win->inputs.buttons[BVR_MOUSE_BUTTON_LEFT] == BVR_MOUSE_BUTTON_DOUBLE_PRESSED){
                nk_input_button(&context->context, NK_BUTTON_DOUBLE, (int)x, (int)y, down);
            }
            nk_input_button(&context->context, NK_BUTTON_LEFT, (int)x, (int)y, down);
        }
        
        nk_input_button(&context->context, NK_BUTTON_MIDDLE, (int)x, (int)y, context->win->inputs.buttons[BVR_MOUSE_BUTTON_MIDDLE]);
        nk_input_button(&context->context, NK_BUTTON_RIGHT, (int)x, (int)y, context->win->inputs.buttons[BVR_MOUSE_BUTTON_RIGHT]);
    }

    if((context->win->events & SDL_EVENT_MOUSE_MOTION) == SDL_EVENT_MOUSE_MOTION){
        if (((struct nk_context*)&context->context)->input.mouse.grabbed) {
            int x = (int)((struct nk_context*)&context->context)->input.mouse.prev.x;
            int y = (int)((struct nk_context*)&context->context)->input.mouse.prev.y;
            nk_input_motion(&context->context, x + context->win->inputs.relative_motion[0], y + context->win->inputs.relative_motion[1]);
        }
        else {
            nk_input_motion(&context->context, context->win->inputs.motion[0], context->win->inputs.motion[1]);
        }
    }
    if((context->win->events & SDL_EVENT_TEXT_INPUT) == SDL_EVENT_TEXT_INPUT){
        nk_glyph glyph;
        memcpy(glyph, context->win->inputs.text_input, NK_UTF_SIZE);
        nk_input_glyph(&context->context, glyph);
    }*/

    nk_input_end(&context->context);
}

void bvr_canvas_render(bvr_canvas_t* context){
    BVR_ASSERT(context);

    float tex_layer = -1.0f;
    bvr_texture_t texture;
    texture.target = BVR_TEXTURE_2D;
    texture.unit = 0;
    texture.id = 0;

    vec2 scale;
    BVR_CREATE_VEC2(scale, 1.0f, 1.0f);

    mat4x4 view = {
        {  2.0f,  0.0f,  0.0f, 0.0f },
        {  0.0f, -2.0f,  0.0f, 0.0f },
        {  0.0f,  0.0f, -1.0f, 0.0f },
        { -1.0f,  1.0f,  0.0f, 1.0f },
    };

    view[0][0] /= (GLfloat)context->win->framebuffer.width;
    view[1][1] /= (GLfloat)context->win->framebuffer.height;

    // set blending and disable depth test
    bvr_pipeline_state_enable(&bvr_get_instance()->pipeline.gui_pass);

    // update shaders
    bvr_shader_set_uniform_raw(context->device.projection, &view[0][0]);

    bvr_shader_set_uniform_raw(context->device.texture, NULL);
    bvr_shader_set_uniform_raw(context->device.texture_array, NULL);

    bvr_shader_enable(&context->device.shader);

    {
        const struct nk_draw_command* cmd;
        const nk_draw_index* offset = NULL;
        void* vertices = NULL;
        void* elements = NULL;

        glBindVertexArray(context->device.vao);
        glBindBuffer(GL_ARRAY_BUFFER, context->device.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context->device.ebo);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        
        glVertexAttribPointer(
            0, 2, GL_FLOAT, GL_FALSE, 
            context->device.vs, (void*)context->device.vp
        );

        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE, 
            context->device.vs, (void*)context->device.vt
        );

        glVertexAttribPointer(
            2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
            context->device.vs, (void*)context->device.vc
        );

        vertices = malloc(context->device.vertex_count);
        elements = malloc(context->device.element_count);

        {
            struct nk_buffer vertex_buffer;
            struct nk_buffer element_buffer;

            struct nk_convert_config config;
            static const struct nk_draw_vertex_layout_element vertex_layout[] = {
                {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct bvri_gui_vertex_s, position)},
                {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct bvri_gui_vertex_s, uvs)},
                {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct bvri_gui_vertex_s, color)},
                {NK_VERTEX_LAYOUT_END}
            };

            memset(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(struct bvri_gui_vertex_s);
            config.vertex_alignment = NK_ALIGNOF(struct bvri_gui_vertex_s);
            config.tex_null = context->default_font.null_tex;
            config.circle_segment_count = context->device.segment_count;
            config.curve_segment_count = context->device.segment_count;
            config.arc_segment_count = context->device.segment_count;
            config.global_alpha = context->device.alpha;
            config.shape_AA = context->device.use_antialiasing;
            config.line_AA = context->device.use_antialiasing;

            nk_buffer_init_fixed(&vertex_buffer, vertices, context->device.vertex_count);
            nk_buffer_init_fixed(&element_buffer, elements, context->device.element_count);

            nk_convert(&context->context, &context->device.cmds, &vertex_buffer, &element_buffer, &config);

            nk_buffer_clear(&vertex_buffer);
            nk_buffer_clear(&element_buffer);
        }

        glBufferSubData(GL_ARRAY_BUFFER, 0, context->device.vertex_count, vertices);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, context->device.element_count, elements);

        free(vertices);
        free(elements);

        nk_draw_foreach(cmd, &context->context, &context->device.cmds){
            if(!cmd->elem_count) continue;

            if(cmd->texture.id >= 0){
                texture.target = BVR_TEXTURE_2D;
                texture.id = cmd->texture.id;
                texture.unit = BVR_TEXTURE_UNIT0;

                tex_layer = -1.0f;

                bvr_shader_use_uniform(context->device.texture, &texture);
            }
            else {
                texture.target = BVR_TEXTURE_2D_ARRAY;
                texture.id = (-cmd->texture.id) >> 16;
                texture.unit = BVR_TEXTURE_UNIT1;

                tex_layer = (-cmd->texture.id) & 0xFFFF;

                bvr_shader_use_uniform(context->device.texture_array, &texture);
            }

            bvr_shader_use_uniform(context->device.texture_layer, &tex_layer);

            glScissor((GLint)(cmd->clip_rect.x * scale[0]),
                (GLint)((context->win->framebuffer.height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * scale[1]),
                (GLint)(cmd->clip_rect.w * scale[0]),
                (GLint)(cmd->clip_rect.h * scale[1])
            );

            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }

        nk_clear(&context->context);
        nk_buffer_clear(&context->device.cmds);
    }

    // glDisableVertexAttribArray(0);
    // glDisableVertexAttribArray(1);
    // glDisableVertexAttribArray(2);

    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    bvr_shader_disable();

    // reset pass
    bvr_pipeline_state_enable(&bvr_get_instance()->pipeline.rendering_pass);
}

static void bvri_clipboard_paste(nk_handle usr, struct nk_text_edit *edit){
    //BVR_ASSERT(0);
}

static void bvri_clipboard_copy(nk_handle handle, const char *text, int len){
    //BVR_ASSERT(0);
}

int bvr_create_fontf(bvr_font_t* font, FILE* file, const uint16 size){
    BVR_ASSERT(font);

    font->id = 0;
    font->font = NULL;

    uint64 total_bytes, readed_bytes;
    uint8* file_bytes = NULL;

    struct nk_font_config config;
    bvr_image_t baked_font;

    baked_font.width = 0;
    baked_font.height = 0;
    baked_font.pixels = NULL;
    
    nk_font_atlas_init_default(&font->atlas);
    nk_font_atlas_begin(&font->atlas);

    // create from the file
    if(file){   
        fseek(file, 0, SEEK_SET);

        total_bytes = bvr_fsize(file);
        file_bytes = malloc(readed_bytes + 1);
        BVR_ASSERT(file_bytes);

        readed_bytes = fread(file_bytes, sizeof(char), total_bytes, file);
        BVR_ASSERT(total_bytes == readed_bytes);

        config = nk_font_config(size);
        config.ttf_blob = file_bytes;
        config.ttf_size = readed_bytes;
        config.size = size;
        config.ttf_data_owned_by_atlas = false;

        font->font = nk_font_atlas_add(&font->atlas, &config);   
        BVR_ASSERT(font->font);
    }
    // create with default's nuklear font
    else {
        config = nk_font_config(size);
        config.ttf_blob = NULL;
        config.ttf_size = 0;
        config.size = size;
        config.ttf_data_owned_by_atlas = false;

        font->font = nk_font_atlas_add_default(&font->atlas, size, &config);
        BVR_ASSERT(font->font);
    }

    baked_font.pixels = nk_font_atlas_bake(
        &font->atlas, &baked_font.width, &baked_font.height, 
        NK_FONT_ATLAS_RGBA32
    );

    BVR_ASSERT(baked_font.pixels);
    BVR_ASSERT(baked_font.width && baked_font.height);

    glGenTextures(1, &font->id);
    glBindTexture(GL_TEXTURE_2D, font->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, 
        baked_font.width, baked_font.height, 0, 
        GL_RGBA, GL_UNSIGNED_BYTE, baked_font.pixels
    );

    nk_font_atlas_end(&font->atlas, nk_handle_id(font->id), &font->null_tex);
    nk_font_atlas_cleanup(&font->atlas);

    free(file_bytes);

    return BVR_TRUE;
}

int bvr_register_font(bvr_canvas_t* canvas, bvr_font_t* font){
    BVR_ASSERT(canvas);
    BVR_ASSERT(font);

    if(font->font){
        nk_style_set_font(&canvas->context, &font->font->handle);

        return BVR_TRUE;
    }
    
    BVR_PRINT("font registration failed!");
    return BVR_FALSE;
}

void bvr_destroy_font(bvr_font_t* font){
    BVR_ASSERT(font);

    nk_font_atlas_cleanup(&font->atlas);
    glDeleteTextures(1, &font->id);
}

#endif