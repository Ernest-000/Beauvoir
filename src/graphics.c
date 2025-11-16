#include <BVR/graphics.h>

#include <GLAD/glad.h>

#include <BVR/math.h>
#include <BVR/utils.h>
#include <BVR/scene.h>

#include <memory.h>
#include <malloc.h>

void bvr_pipeline_state_enable(struct bvr_pipeline_state_s* const state){
    BVR_ASSERT(state);

    if(state->blending){
        glEnable(GL_BLEND);
        switch(state->blending)
        {
        case BVR_BLEND_FUNC_ALPHA_ONE_MINUS:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case BVR_BLEND_FUNC_ALPHA_ADD:
            glBlendFunc(GL_ONE, GL_ONE);
            break;
        case BVR_BLEND_FUNC_ALPHA_MULT:
            glBlendFunc(GL_ONE, GL_SRC_COLOR);
            break;
        default:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        }
    }
    else {
        glDisable(GL_BLEND);
    }

    if(state->depth){
        glEnable(GL_DEPTH_TEST);

        switch (state->depth)
        {
        case BVR_DEPTH_FUNC_NEVER:
            glDepthFunc(GL_NEVER);
            break;
        
        case BVR_DEPTH_FUNC_ALWAYS:
            glDepthFunc(GL_ALWAYS);
            break;
        
        case BVR_DEPTH_FUNC_LESS:
            glDepthFunc(GL_LESS);
            break;
    
        case BVR_DEPTH_FUNC_GREATER:
            glDepthFunc(GL_GREATER);
            break;

        case BVR_DEPTH_FUNC_LEQUAL:
            glDepthFunc(GL_LEQUAL);
            break;
        
        case BVR_DEPTH_FUNC_GEQUAL:
            glDepthFunc(GL_GEQUAL);
            break;

        case BVR_DEPTH_FUNC_NOTEQUAL:
            glDepthFunc(GL_NOTEQUAL);
            break;
        
        case BVR_DEPTH_FUNC_EQUAL:
            glDepthFunc(GL_EQUAL);
            break;

        default:
            glDepthFunc(GL_ALWAYS);
            break;
        }
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }
}

void bvr_pipeline_draw_cmd(struct bvr_draw_command_s* cmd){
    bvr_shader_enable(cmd->shader);
    
    // try to apply local uniform
    bvr_shader_use_uniform(
        bvr_find_uniform(cmd->shader, "bvr_local"), 
        &cmd->vertex_group.matrix[0][0]
    );

    glBindVertexArray(cmd->array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, cmd->vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmd->element_buffer);

    for (uint64 i = 0; i < cmd->attrib_count; i++)
    {
        glEnableVertexAttribArray(i);
    }
    
    // if use element 
    if(cmd->element_buffer){ 
        glDrawElementsBaseVertex(cmd->draw_mode, 
            cmd->vertex_group.element_count, 
            cmd->element_type, 
            NULL, 
            cmd->vertex_group.element_offset
        );
    }
    else {
        glDrawArrays(cmd->draw_mode, cmd->vertex_group.element_offset, cmd->vertex_group.element_count);
    }

    for (uint64 i = 0; i < cmd->attrib_count; i++)
    {
        glDisableVertexAttribArray(i);
    }

    glBindVertexArray(cmd->array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, cmd->vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmd->element_buffer);

    bvr_shader_disable();
}

void bvr_pipeline_add_draw_cmd(struct bvr_draw_command_s* cmd){
    BVR_ASSERT(cmd);

    if(bvr_get_book_instance()->pipeline.command_count + 1 < BVR_MAX_DRAW_COMMAND){
        memcpy(
            &bvr_get_book_instance()->pipeline.commands[bvr_get_book_instance()->pipeline.command_count++], 
            cmd, sizeof(struct bvr_draw_command_s)
        );
    }
}

void bvr_poll_errors(void){
    char found_error = 0;
    uint32 err;

    while ((err = glGetError()) != GL_NO_ERROR)
    {
        switch (err)
        {
        case GL_INVALID_ENUM:
            BVR_PRINT("GL_INVALID_ENUM");
            break;
    
        case GL_INVALID_VALUE:
            BVR_PRINT("GL_INVALID_VALUE");
            break;
    
        case GL_INVALID_OPERATION:
            BVR_PRINT("GL_INVALID_OPERATION");
            break;
    
        case GL_STACK_OVERFLOW:
            found_error = 1;
            
            BVR_PRINT("GL_STACK_OVERFLOW");
            break;
    
        case GL_STACK_UNDERFLOW:
            found_error = 1;
            
            BVR_PRINT("GL_STACK_UNDERFLOW");
            break;
    
        case GL_OUT_OF_MEMORY:
            found_error = 1;
            
            BVR_PRINT("GL_OUT_OF_MEMORY");
            break;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            BVR_PRINT("GL_INVALID_FRAMEBUFFER_OPERATION");
            break;

        default:
            BVR_ASSERT(0);
        }
    }

    // break if a fatal error is catch
    if(found_error){
        BVR_BREAK();
    }
}

int bvr_create_framebuffer(bvr_framebuffer_t* framebuffer, const uint16 width, const uint16 height, const char* shader){
    BVR_ASSERT(framebuffer);
    BVR_ASSERT(width > 0 && height > 0);

    framebuffer->width = width;
    framebuffer->height = height;
    framebuffer->target_width = width;
    framebuffer->target_height = height;

    {
        int half_width = width * 0.5f;
        int half_height = height * 0.5f;

        const float quad[24] = {
            -half_width,   half_height, 0.0f, 1.0f,
            -half_width,  -half_height, 0.0f, 0.0f,
             half_width,  -half_height, 1.0f, 0.0f,
            -half_width,   half_height, 0.0f, 1.0f,
             half_width,  -half_height, 1.0f, 0.0f,
             half_width,   half_height, 1.0f, 1.0f,
        };

        glGenVertexArrays(1, &framebuffer->vertex_buffer);
        glBindVertexArray(framebuffer->vertex_buffer);

        glGenBuffers(1, &framebuffer->array_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, framebuffer->array_buffer);

        glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), &quad, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)8);
    }

    if(shader){
        bvr_create_shader(&framebuffer->shader, shader, BVR_FRAMEBUFFER_SHADER);
    }
    else {
        const char* vertex_shader = 
            "#version 400\n"
            "layout(location=0) in vec2 in_position;\n"
            "layout(location=1) in vec2 in_uvs;\n"
            "uniform mat4 bvr_projection;\n"
            "out V_DATA {\n"
            "	vec2 uvs;\n"
            "} vertex;\n"
            "void main() {\n"
            "	gl_Position = bvr_projection * vec4(in_position, 0.0, 1.0);\n"
            "	vertex.uvs = in_uvs;\n"
            "}";

        const char* fragment_shader = 
            "#version 400\n"
            "in V_DATA {\n"
	        "vec2 uvs;\n"
            "} vertex;\n"
            "uniform sampler2D bvr_texture;\n"
            "void main() {\n"
            	"vec4 tex = texture(bvr_texture, vertex.uvs);\n"
            	"gl_FragColor = vec4(tex.rgb, 1.0);\n"
            "}";
        
        bvri_create_shader_vert_frag(&framebuffer->shader, vertex_shader, fragment_shader);
        bvr_shader_register_uniform(&framebuffer->shader, BVR_MAT4, 1, "bvr_projection");
    }

    glGenFramebuffers(1, &framebuffer->buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->buffer);

    glGenTextures(1, &framebuffer->color_buffer);
    glBindTexture(GL_TEXTURE_2D, framebuffer->color_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer->color_buffer, 0);

    glGenRenderbuffers(1, &framebuffer->depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebuffer->depth_buffer);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        BVR_PRINT("failed to create a new framebuffer!");
        return BVR_FAILED;
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return BVR_OK;
}

void bvr_framebuffer_enable(bvr_framebuffer_t* framebuffer){
    int viewport[4];

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->buffer);
    glGetIntegerv(GL_VIEWPORT, viewport);
    framebuffer->target_width = viewport[2];
    framebuffer->target_height = viewport[3];

    glViewport(0, 0, framebuffer->width, framebuffer->height);
}

void bvr_framebuffer_disable(bvr_framebuffer_t* framebuffer){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, framebuffer->target_width, framebuffer->target_height);
}

void bvr_framebuffer_clear(bvr_framebuffer_t* framebuffer, vec3 const color){
    glClearColor(color[0], color[1], color[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void bvr_framebuffer_blit(bvr_framebuffer_t* framebuffer){
    mat4x4 ortho;
    mat4_ortho(ortho, 
        -framebuffer->width  / 2.0f,
         framebuffer->width  / 2.0f,
        -framebuffer->height / 2.0f,
         framebuffer->height / 2.0f,
        0.0f, 0.1f
    );

    bvr_shader_set_uniformi(&framebuffer->shader.uniforms[1], &ortho[0][0]);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    bvr_shader_enable(&framebuffer->shader);

    glBindVertexArray(framebuffer->vertex_buffer);
    glBindTexture(GL_TEXTURE_2D, framebuffer->color_buffer);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    bvr_shader_disable();
}

void bvr_destroy_framebuffer(bvr_framebuffer_t* framebuffer){
    bvr_destroy_shader(&framebuffer->shader);

    glDeleteVertexArrays(1, &framebuffer->vertex_buffer);
    glDeleteBuffers(1, &framebuffer->array_buffer);
    glDeleteTextures(1, &framebuffer->color_buffer);
    glDeleteRenderbuffers(1, &framebuffer->depth_buffer);
    glDeleteFramebuffers(1, &framebuffer->buffer);
}