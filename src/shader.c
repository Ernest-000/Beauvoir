#include <BVR/shader.h>
#include <BVR/math.h>
#include <BVR/file.h>
#include <BVR/image.h>

#include <string.h>
#include <memory.h>
#include <malloc.h>

#include <GLAD/glad.h>

#define BVR_MAX_GLSL_HEADER_SIZE 100

static int bvri_compile_shader(uint32* shader, bvr_string_t* const content, int type);
static int bvri_link_shader(const uint32 program);
static int bvri_register_shader_state(bvr_shader_t* program, bvr_shader_stage_t* shader, bvr_string_t* content, 
    const char* header, const char* name, int type);

static int bvri_compile_shader(uint32* shader, bvr_string_t* const content, int type){
    *shader = glCreateShader(type);

    glShaderSource(*shader, 1, (const char**)&content->string, NULL);
    glCompileShader(*shader);

    int state;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &state);
    if(!state){
        char buffer[BVR_BUFFER_SIZE];
        glGetShaderInfoLog(*shader, BVR_BUFFER_SIZE, NULL, buffer);
        BVR_PRINT(buffer);

        return BVR_FAILED;
    }

    return BVR_OK;
}

static int bvri_link_shader(const uint32 program) {
    glLinkProgram(program);

    int state;
    glGetProgramiv(program, GL_LINK_STATUS, &state);
    if(!state){
        char buffer[BVR_BUFFER_SIZE];
        glGetProgramInfoLog(program, BVR_BUFFER_SIZE, NULL, buffer);
        BVR_PRINT(buffer);

        return BVR_FAILED;
    }

    return BVR_OK;
}

static int bvri_register_shader_state(bvr_shader_t* program, bvr_shader_stage_t* shader, bvr_string_t* content, 
    const char* header, const char* name, int type){
    
    BVR_ASSERT(shader);
    BVR_ASSERT(content);
    BVR_ASSERT(header);
    BVR_ASSERT(name);

    char shader_header_str[BVR_BUFFER_SIZE];
    memset(shader_header_str, 0, BVR_BUFFER_SIZE);

    bvr_string_t shader_str;

    strncpy(shader_header_str, header, strnlen(header, 100));
    strncat(shader_header_str, "#define \0", 10);
    strncat(shader_header_str, name, strnlen(name, 100));
    strncat(shader_header_str, "\n", 1);

    bvr_create_string(&shader_str, shader_header_str);
    bvr_string_concat(&shader_str, content->string);

    if (type && shader_str.length) {
        if (bvri_compile_shader(&shader->shader, &shader_str, type)) {
            glAttachShader(program->program, shader->shader);
            shader->type = type;
        }
        else {
            BVR_PRINTF("failed to compile shader '%s'!", name);
        }
    }

    bvr_destroy_string(&shader_str);
}

int bvr_create_shaderf(bvr_shader_t* shader, FILE* file, const int flags){
    BVR_ASSERT(shader);
    BVR_ASSERT(file);

    int version_offset = 0;
    char version_header_content[BVR_MAX_GLSL_HEADER_SIZE];
    bvr_string_t file_content;


    { // retrieve the end offset of the #version header
        fseek(file, 0, SEEK_SET);
        do
        {
            version_header_content[version_offset] = getc(file);
            version_offset++;

        } while(
            version_header_content[version_offset - 1] != EOF  &&
            version_header_content[version_offset - 1] != '\n' &&
            version_offset < BVR_MAX_GLSL_HEADER_SIZE - 1
        );
        version_header_content[version_offset] = '\0';
    }

    // read file
    bvr_create_string(&file_content, NULL);
    BVR_ASSERT(bvr_read_file(&file_content, file));
    
    // create shader's program
    shader->program = glCreateProgram();
    shader->flags = flags;
    shader->shader_count = 0;

    // by default there is:
    // - camera block
    //
    shader->block_count = 1;

    // by default there is
    // - transformation uniform
    //
    shader->uniform_count = 1;

    /*
        Framebuffers shader must jump over vertex and fragment sections
    */
    if(BVR_HAS_FLAG(flags, BVR_FRAMEBUFFER_SHADER)){
        bvri_register_shader_state(shader,
            &shader->shaders[shader->shader_count++], &file_content,
            version_header_content, "_VERTEX_", GL_VERTEX_SHADER
        );
        
        bvri_register_shader_state(shader,
            &shader->shaders[shader->shader_count++], &file_content,
            version_header_content, "_FRAGMENT_", GL_FRAGMENT_SHADER
        );

        goto shader_cstor_bidings;
    }

    // check if it contains a vertex shader and create vertex shader stage.
    if (BVR_HAS_FLAG(flags, BVR_VERTEX_SHADER)) {
        bvri_register_shader_state(shader,
            &shader->shaders[shader->shader_count++], &file_content,
            version_header_content, "_VERTEX_", GL_VERTEX_SHADER
        );
    }
    else {
        BVR_PRINT("missing vertex shader!");
    }

    // check if it contains a fragment shader and create fragment shader stage.
    if (BVR_HAS_FLAG(flags, BVR_FRAGMENT_SHADER)) {
        bvri_register_shader_state(shader,
            &shader->shaders[shader->shader_count++], &file_content,
            version_header_content, "_FRAGMENT_", GL_FRAGMENT_SHADER
        );
    }
    else {
        BVR_PRINT("missing fragment shader!");
    }

shader_cstor_bidings:
    // try to compile shader
    if (!bvri_link_shader(shader->program)) {
        BVR_PRINT("failed to compile shader!");
    }

    if(BVR_HAS_FLAG(flags, BVR_FRAMEBUFFER_SHADER)){
        bvr_destroy_string(&file_content);
        return BVR_OK;
    }

    // create default blocks
    
    // create camera block
    shader->blocks[0].type = BVR_MAT4;
    shader->blocks[0].count = 2;
    shader->blocks[0].location = glGetUniformBlockIndex(shader->program, BVR_UNIFORM_CAMERA_NAME);
    if (shader->blocks[0].location == -1) {
        BVR_PRINT("cannot find camera block uniform!");
    }
    else {
        glUniformBlockBinding(shader->program, shader->blocks[0].location, BVR_UNIFORM_BLOCK_CAMERA);
    }

    // create transform uniform
    shader->uniforms[0].location = glGetUniformLocation(shader->program, BVR_UNIFORM_TRANSFORM_NAME);
    shader->uniforms[0].memory.data = NULL;
    shader->uniforms[0].memory.size = sizeof(mat4x4);
    shader->uniforms[0].memory.elemsize = sizeof(mat4x4);
    shader->uniforms[0].name.string = NULL;
    shader->uniforms[0].name.length = 0;
    shader->uniforms[0].type = BVR_MAT4;
    if (shader->blocks[0].location == -1) {
        BVR_PRINT("cannot find transform uniform!");
    }
        
    bvr_destroy_string(&file_content);

    return BVR_OK;
}

int bvri_create_shader_vert_frag(bvr_shader_t* shader, const char* vert, const char* frag){
    BVR_ASSERT(shader);
    BVR_ASSERT(vert);
    BVR_ASSERT(frag);

    shader->program = glCreateProgram();
    shader->flags = 0;
    shader->shader_count = 2;
    shader->uniform_count = 1;
    shader->block_count = 1;

    bvr_string_t vertex;
    bvr_string_t fragment;

    bvr_create_string(&vertex, vert);
    bvr_create_string(&fragment, frag);

    bvri_compile_shader(&shader->shaders[0].shader, &vertex, GL_VERTEX_SHADER);
    bvri_compile_shader(&shader->shaders[1].shader, &fragment, GL_FRAGMENT_SHADER);
    
    glAttachShader(shader->program, shader->shaders[0].shader);
    glAttachShader(shader->program, shader->shaders[1].shader);
    bvri_link_shader(shader->program);
}

void bvr_create_uniform_buffer(uint32* buffer, uint64 size){
    glGenBuffers(1, buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, *buffer);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, *buffer, 0, size);
}

void bvr_enable_uniform_buffer(uint32 buffer){
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
}

void bvr_uniform_buffer_set(uint32 offset, uint64 size, void* data){
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}

void* bvr_uniform_buffer_map(uint32 offset, uint64 size){
    return glMapBufferRange(GL_UNIFORM_BUFFER, offset, size, GL_MAP_READ_BIT);
}

void bvr_uniform_buffer_close(){
    glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void bvr_destroy_uniform_buffer(uint32* buffer){
    if(!buffer){
        return;
    }

    glDeleteBuffers(1, buffer);
}

bvr_shader_uniform_t* bvr_shader_register_uniform(bvr_shader_t* shader, int type, int count, const char* name){
    BVR_ASSERT(shader);
    BVR_ASSERT(name);

    if (shader->uniform_count + 1 >= BVR_MAX_UNIFORM_COUNT) {
        BVR_PRINTF("uniform maximum capacity reached for shader '%i'!", shader->program);
        return NULL;
    }

    int location = glGetUniformLocation(shader->program, name);
    if(location != -1){
        shader->uniforms[shader->uniform_count].location = location;
        shader->uniforms[shader->uniform_count].type = type;

        shader->uniforms[shader->uniform_count].memory.elemsize = bvr_sizeof(type);
        shader->uniforms[shader->uniform_count].memory.size = count * shader->uniforms[shader->uniform_count].memory.elemsize;
            
        shader->uniforms[shader->uniform_count].memory.data = calloc(shader->uniforms[shader->uniform_count].memory.elemsize, count);
        BVR_ASSERT(shader->uniforms[shader->uniform_count].memory.data);

        bvr_create_string(&shader->uniforms[shader->uniform_count].name, name);

        return &shader->uniforms[shader->uniform_count++];
    }

    BVR_PRINTF("cannot find uniform '%s'!", name);
    return NULL;
}

bvr_shader_uniform_t* bvr_shader_register_texture(bvr_shader_t* shader, int type, void* texture, const char* name)
{
    BVR_ASSERT(shader);
    BVR_ASSERT(name);

    if(type < BVR_TEXTURE_2D || type > BVR_TEXTURE_2D_LAYER){
        BVR_PRINT("wrong texture type!");
        return NULL;
    }

    bvr_shader_uniform_t* uniform = bvr_shader_register_uniform(shader, type, 1, name);
    if(uniform){
        // just copy texture's pointer
        memcpy(uniform->memory.data, &texture, sizeof(struct bvr_texture_s*));
    }
    else {
        BVR_PRINT("failed to register texture's uniform");
    }
    return uniform;
}

bvr_shader_block_t* bvr_shader_register_block(bvr_shader_t* shader, const char* name, int type, int count, int index){
    BVR_ASSERT(shader);
    BVR_ASSERT(count > 0);

    if(shader->block_count + 1 >= BVR_MAX_SHADER_BLOCK_COUNT){
        BVR_PRINTF("block maximum capacity reached for shader '%i'!", shader->program);
        return NULL;
    }

    if(name && index >= 0){
        shader->blocks[shader->block_count].type = type;
        shader->blocks[shader->block_count].count = count;
        shader->blocks[shader->block_count].location = glGetUniformBlockIndex(shader->program, name);
        if(shader->blocks[shader->block_count].location == -1){
            BVR_PRINT("cannot find unfirm block!");
            return NULL;
        }

        glUniformBlockBinding(shader->program, shader->blocks[shader->block_count].location, index);
        return &shader->blocks[shader->block_count++];
    }
    else {
        BVR_PRINT("cannot find uniform block!");
        return NULL;
    }
}

void bvr_shader_set_uniformi(bvr_shader_uniform_t* uniform, void* data){
    if(data && uniform){
        BVR_ASSERT(uniform->memory.data);

        memcpy(uniform->memory.data, data, uniform->memory.size);
    }
    else {
        BVR_PRINT("skipping uniform data updating, data is NULL");
    }
}

void bvr_shader_set_texturei(bvr_shader_uniform_t* uniform, void* texture){
    if(texture && uniform){
        BVR_ASSERT(uniform->memory.data);

        memcpy(uniform->memory.data, &texture, uniform->memory.size);
    }
    else {
        BVR_PRINT("skipping texture updating, data is NULL");
    }
}

void bvr_shader_set_uniform(bvr_shader_t* shader, const char* name, void* data){
    BVR_ASSERT(shader);
    BVR_ASSERT(name);

    bvr_shader_set_uniformi(bvr_find_uniform(shader, name), data);
}

void bvr_shader_use_uniform(bvr_shader_uniform_t* uniform, void* data){
    if(!uniform || uniform->location == -1) {
        BVR_PRINTF("cannot find uniform %i", uniform->name.string);
        return;
    }

    if(!data){
        data = uniform->memory.data;
    }

    if(data){

        switch (uniform->type)
        {
        case BVR_FLOAT: 
            glUniform1fv(uniform->location, uniform->memory.size / uniform->memory.elemsize, (float*)data); 
            return;

        case BVR_INT32: 
            glUniform1iv(uniform->location, uniform->memory.size / uniform->memory.elemsize, (int*)data); 
            return;
        
        case BVR_VEC3:
            glUniform3fv(uniform->location, uniform->memory.size / uniform->memory.elemsize, (float*)data);
            return;

        case BVR_VEC4:
            glUniform4fv(uniform->location, uniform->memory.size / uniform->memory.elemsize, (float*)data);
            return;

        case BVR_MAT4: 
            glUniformMatrix4fv(uniform->location, uniform->memory.size / uniform->memory.elemsize, GL_FALSE, (float*)data); 
            return;
        
        case BVR_TEXTURE_2D:
            {
                bvr_texture_t* texture = *(bvr_texture_t**)data;

                bvr_texture_enable(texture);
                glUniform1i(uniform->location, (int)texture->unit);
            }
            break;
        
        case BVR_TEXTURE_2D_ARRAY:            
            {
                bvr_texture_atlas_t* texture = *(bvr_texture_atlas_t**)data;

                bvr_texture_atlas_enablei(texture);
                glUniform1i(uniform->location, (int)texture->unit);
            }
            break;

        case BVR_TEXTURE_2D_LAYER:
            {
                bvr_layered_texture_t* texture = *(bvr_layered_texture_t**)data;

                bvr_layered_texture_enable(texture);
                glUniform1i(uniform->location, (int)texture->unit);
            }
            break;

        default:
            break;
        }
    }
}

void bvr_shader_enable(bvr_shader_t* shader){
    glUseProgram(shader->program);
    
    // start at one it order to omit transform uniform
    for (uint64 uniform = 0; uniform < shader->uniform_count; uniform++)
    {
        bvr_shader_use_uniform(&shader->uniforms[uniform], NULL);
    }
}

void bvr_shader_disable(void){
    glUseProgram(0);
}

void bvr_destroy_shader(bvr_shader_t* shader){
    BVR_ASSERT(shader);

    for (uint64 shader_i = 0; shader_i < shader->shader_count; shader_i++)
    {
        glDeleteShader(shader->shaders[shader_i].shader);
    }

    for (uint64 uniform = 0; uniform < shader->uniform_count; uniform++)
    {
        bvr_destroy_string(&shader->uniforms[uniform].name);

        free(shader->uniforms[uniform].memory.data);
        shader->uniforms[uniform].memory.data = NULL;
    }

    glDeleteProgram(shader->program);
}