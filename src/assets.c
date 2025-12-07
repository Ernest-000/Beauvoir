#include <BVR/assets.book.h>
#include <BVR/assets.h>

#include <BVR/editor/flags.h>

#include <BVR/file.h>

#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include <GLAD/glad.h>

#define BVR_FILE_SIG "BVRB"
#define BVR_PAGE_SIG "PAJ"
#define BVR_BIN_SIG  "BIN"

struct bvri_chunk_data_s {
    uint32 length;
    uint16 flag;
    void* buffer;
};

struct bvri_header_data_s {
    char sig[4];
    uint32 size;
};

#pragma region asset db

bvr_uuid_t* bvr_register_asset(const char* path, char open_mode){
    BVR_ASSERT(path);

    bvr_book_t* book = bvr_get_instance();

    // if the file does not exists
    if(access(path, F_OK) != 0){
        BVR_PRINTF("%s does not exist!", path);
        return NULL;
    }   

    if(!book->asset_stream.data){
        BVR_PRINT("asset stream is not created!");
        return NULL;
    }

    bvr_asset_t asset;
    bvr_uuid_t* uuid = bvr_find_asset(path, NULL);
    
    if(uuid){
        BVR_PRINT("asset already created!");
        return uuid;
    }

    asset.open_mode = open_mode;

    bvr_create_uuid(asset.id);
    bvr_create_string(&asset.path, path);

    bvr_memstream_seek(&book->asset_stream, 0, SEEK_NEXT);
    bvr_uuid_t* id_ptr = (bvr_uuid_t*)book->asset_stream.cursor;

    bvr_memstream_write(&book->asset_stream, asset.id, sizeof(bvr_uuid_t));
    bvr_memstream_write(&book->asset_stream, &asset.path.length, sizeof(unsigned short));
    bvr_memstream_write(&book->asset_stream, asset.path.string, asset.path.length);
    bvr_memstream_write(&book->asset_stream, &asset.open_mode, sizeof(char));

    bvr_destroy_string(&asset.path);
    return id_ptr;
}

// TODO: improve with an hash map
bvr_uuid_t* bvr_find_asset(const char* path, bvr_asset_t* asset){
    BVR_ASSERT(path);

    bvr_book_t* book = bvr_get_instance();

    bvr_uuid_t* uuid = NULL;
    uint16 string_length;
    const char* prev_cursor = book->asset_stream.cursor;

    bvr_memstream_seek(&book->asset_stream, 0, SEEK_SET);
    while (book->asset_stream.cursor < prev_cursor)
    {
        // skip uuid
        book->asset_stream.cursor += sizeof(bvr_uuid_t);

        string_length = *((uint16*)book->asset_stream.cursor);

        if(!strncmp(book->asset_stream.cursor + sizeof(unsigned short), path, string_length)){
            uuid = (bvr_uuid_t*)book->asset_stream.cursor;
            if(asset == NULL){
                return uuid;
            }

            memcpy(&asset->id, book->asset_stream.cursor - sizeof(bvr_uuid_t), sizeof(bvr_uuid_t));
            asset->path.length = string_length;
            asset->path.string = book->asset_stream.cursor + sizeof(unsigned short);
            
            book->asset_stream.cursor += string_length;
            asset->open_mode = *book->asset_stream.cursor;
            return uuid;
        }

        book->asset_stream.cursor += string_length;

        // go to the end
        bvr_memstream_seek(&book->asset_stream, sizeof(uint16) + sizeof(char), SEEK_CUR);
    }
    
    return uuid;
}

int bvr_find_asset_uuid(const bvr_uuid_t uuid, bvr_asset_t* asset){
    BVR_ASSERT(asset);

    bvr_book_t* book = bvr_get_instance();

    uint16 string_length;
    bvr_uuid_t other;
    const char* prev_cursor = book->asset_stream.cursor;

    while (book->asset_stream.cursor < prev_cursor)
    {
        bvr_memstream_read(&book->asset_stream, &other, sizeof(bvr_uuid_t));

        if(bvr_uuid_equals(other, uuid)){
            
            string_length = *((uint16*)book->asset_stream.cursor);
            asset->path.length = string_length;
            asset->path.string = book->asset_stream.cursor + sizeof(unsigned short);
            
            book->asset_stream.cursor += string_length;
            asset->open_mode = *book->asset_stream.cursor;

            return BVR_TRUE;
        }

        book->asset_stream.cursor += sizeof(bvr_uuid_t);
        string_length = *((uint16*)book->asset_stream.cursor);

        bvr_memstream_seek(&book->asset_stream, sizeof(uint16) + sizeof(char) + string_length, SEEK_CUR);
    }
    
    return BVR_FALSE;
}

#pragma endregion

#pragma region write

static void bvri_clear_file(FILE* file){
    BVR_ASSERT(file);

    fflush(file);
    fseek(file, 0, SEEK_SET);
    ftruncate(fileno(file), 0);
}

static void bvri_write_string(FILE* file, bvr_string_t* string){
    fwrite(&string->length, sizeof(uint16), 1, file);
    if(string->string){
        fwrite(string->string, sizeof(char), string->length, file);
    }
}

static void bvri_write_asset_reference(FILE* file, struct bvr_asset_reference_s* asset){
    fwrite(&asset->origin, sizeof(enum bvr_asset_reference_origin_e), 1, file);
    switch (asset->origin)
    {
    case BVR_ASSET_ORIGIN_PATH:
        fwrite(asset->pointer.asset_id, sizeof(bvr_uuid_t), 1, file);
        break;
    
    case BVR_ASSET_ORIGIN_RAW:
        fwrite(&asset->pointer.raw_data.size, sizeof(unsigned long), 1, file);
        fwrite(asset->pointer.raw_data.data, asset->pointer.raw_data.size, 1, file);
        break;

    default:
        break;
    }
}

static void bvri_write_chunk(FILE* file, struct bvri_chunk_data_s* data){
    fwrite(&data->length, sizeof(uint32), 1, file);
    fwrite(&data->flag, sizeof(uint16), 1, file);
    if(data->buffer){
        fwrite(data->buffer, sizeof(char), data->length, file);
    }
}

static void bvri_write_chunk_data(FILE* file, uint32 length, uint16 flag, void* buffer){
    fwrite(&length, sizeof(uint32), 1, file);
    fwrite(&flag, sizeof(uint16), 1, file);
    if(buffer){
        fwrite(buffer, sizeof(char), length, file);
    }
}

void bvr_write_book_dataf(FILE* file, bvr_book_t* book){
    BVR_ASSERT(file);
    BVR_ASSERT(book);

    struct bvri_header_data_s header;
    header.size = 0;
    strncpy(header.sig, BVR_FILE_SIG, 4);

    bvri_clear_file(file);

    fseek(file, 0, SEEK_SET);
    fwrite(&header, sizeof(struct bvri_header_data_s), 1, file);

    uint32 bin_offset = 0;
    // write asset informaions
    {
        uint32 stream_size = book->asset_stream.next - (char*)book->asset_stream.data;
        uint16 asset_flag = BVR_EDITOR_ASSETS;
        bin_offset = 0;

        fwrite(&stream_size, sizeof(uint32), 1, file);
        fwrite(&asset_flag, sizeof(uint16), 1, file);
        fwrite(&bin_offset, sizeof(uint32), 1, file);
        fwrite(book->asset_stream.data, stream_size, 1, file);
    }

    // write page informations
    {
        bvr_page_t* page = &book->page;

        struct {
            float near, far, scale;
            bvr_transform_t transform;
        } camera;

        camera.near = page->camera.near;
        camera.far = page->camera.far;
        camera.scale = page->camera.field_of_view.scale;
        memcpy(&camera.transform, &page->camera.transform, sizeof(bvr_transform_t));

        bvri_write_string(file, &page->name);
        bvri_write_chunk_data(file, sizeof(camera), BVR_EDITOR_CAMERA, &camera);

        // here, write other scene infos
    }

    // actors chunk
    {
        const uint32 section_start = ftell(file);

        uint32 size_offset;
        uint32 prev_offset; 
        uint32 length_offset;

        uint32 section_size = 0;
        uint32 actor_count = book->page.actors.count;
        uint16 actor_flag = BVR_EDITOR_ACTOR;

        fwrite(&section_size, sizeof(uint32), 1, file);
        fwrite(&actor_flag, sizeof(uint16), 1, file);
        fwrite(&actor_count, sizeof(uint32), 1, file);

        struct {
            uint32 size;
            uint32 offset;

            bvr_string_t name;
            bvr_actor_type_t type;

            bvr_uuid_t id;
            uint8 active;
            int flags;
            uint16 order_in_layer;

            bvr_transform_t transform;
        } target;

        struct bvr_actor_s* actor = NULL;
        BVR_POOL_FOR_EACH(actor, book->page.actors){
            if(actor == NULL){
                break;
            }

            target.size = 0;
            target.offset = 0;
            target.type = actor->type;
            target.flags = actor->flags;
            target.active = actor->active;
            target.order_in_layer = actor->order_in_layer;
            
            memcpy(&target.transform, &actor->transform, sizeof(bvr_transform_t));
            memcpy(&target.id, &actor->id, sizeof(bvr_uuid_t));
            bvr_string_create_and_copy(&target.name, &actor->name);

            size_offset = ftell(file);
            fwrite(&target.size, sizeof(uint32), 1, file);
            fwrite(&target.offset, sizeof(uint32), 1, file);

            bvri_write_string(file, &target.name);
            
            fwrite(&target.type, sizeof(bvr_actor_type_t), 1, file);
            fwrite(&target.id, sizeof(bvr_uuid_t), 1, file);
            fwrite(&target.active, sizeof(uint8), 1, file);
            fwrite(&target.flags, sizeof(int), 1, file);
            fwrite(&target.order_in_layer, sizeof(uint16), 1, file);
            fwrite(&target.transform, sizeof(bvr_transform_t), 1, file);

            // actor overwrite 
            switch (target.type)
            {
            case BVR_NULL_ACTOR:
            case BVR_EMPTY_ACTOR:
                break;
            
            case BVR_LAYER_ACTOR:
                {
                    bvr_layer_t* layer;
                    uint32 layer_count = BVR_BUFFER_COUNT(((bvr_layer_actor_t*)actor)->texture.image.layers);
                    
                    fwrite(&layer_count, sizeof(uint32), 1, file);
                    for (size_t i = 0; i < layer_count; i++)
                    {
                        layer = &((bvr_layer_t*)((bvr_layer_actor_t*)actor)->texture.image.layers.data)[i];

                        fwrite(&layer->flags, sizeof(uint16), 1, file);
                        fwrite(&layer->anchor_x, sizeof(int), 1, file);
                        fwrite(&layer->anchor_y, sizeof(int), 1, file);
                        fwrite(&layer->opacity, sizeof(short), 1, file);
                        fwrite(&layer->blend_mode, sizeof(bvr_layer_blend_mode_t), 1, file);
                    }
                    
                }
                break;

            case BVR_LANDSCAPE_ACTOR:
                {
                    bvr_landscape_actor_t* landscape = (bvr_landscape_actor_t*)actor;
                    uint32 landscape_byte_length = landscape->mesh.vertex_count * sizeof(int);

                    fwrite(&landscape->dimension, sizeof(landscape->dimension), 1, file);
                    fwrite(&landscape_byte_length, sizeof(uint32), 1, file);
                    
                    glBindBuffer(GL_ARRAY_BUFFER, landscape->mesh.vertex_buffer);
                    int* map = glMapBufferRange(GL_ARRAY_BUFFER, 0, landscape_byte_length, GL_MAP_READ_BIT);
                    
                    if(map){
                        fwrite(map, sizeof(char), landscape_byte_length, file);
                        
                        glUnmapBuffer(GL_ARRAY_BUFFER);
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                    }
                }
            default:
                break;
            }

            prev_offset = ftell(file);

            // calc actor data section size
            target.size = prev_offset - size_offset;
            section_size += target.size;
            
            // update size informations
            fseek(file, size_offset, SEEK_SET);
            fwrite(&target.size, sizeof(uint32), 1, file);
            fseek(file, section_start, SEEK_SET);
            fwrite(&section_size, sizeof(uint32), 1, file);
            fseek(file, prev_offset, SEEK_SET);
            
            // free elements
            bvr_destroy_string(&target.name);
        }
    }

    fflush(file);
}

#pragma endregion

#pragma region open

static void bvri_read_string(FILE* file, bvr_string_t* string){
    char buffer[BVR_BUFFER_SIZE];
    uint16 length = bvr_freadu16_le(file);
    BVR_ASSERT(length < BVR_BUFFER_SIZE);

    fread(buffer, sizeof(char), length, file);
    bvr_overwrite_string(string, buffer, length);
}

static void bvri_read_chunk(FILE* file, struct bvri_chunk_data_s* chunk, void* object){
    BVR_ASSERT(object);
    BVR_ASSERT(chunk);

    chunk->length = bvr_freadu32_le(file);
    chunk->flag = bvr_freadu16_le(file);
    chunk->buffer = object;

    if(chunk->length){
        fread(object, sizeof(char), chunk->length, file);
    }
}

static void bvri_read_asset_reference(FILE* file, struct bvr_asset_reference_s* asset){
    asset->origin = (enum bvr_asset_reference_origin_e)bvr_fread32_le(file);
    switch (asset->origin)
    {
    case BVR_ASSET_ORIGIN_PATH:
        fread(asset->pointer.asset_id, sizeof(bvr_uuid_t), 1, file);
        break;
    
    case BVR_ASSET_ORIGIN_RAW:
        BVR_ASSERT(0 || "not supported");

    default:
        break;
    }
}

void bvr_open_book_dataf(FILE* file, bvr_book_t* book){
    BVR_ASSERT(file);
    BVR_ASSERT(book);

    fseek(file, 0, SEEK_SET);

    // read the header
    struct bvri_header_data_s header;
    fread(&header.sig, sizeof(char), 4, file);
    header.size = bvr_fread32_le(file);

    // check for BRVB signature
    BVR_ASSERT(
        header.sig[0] == 'B' && 
        header.sig[1] == 'V' && 
        header.sig[2] == 'R' && 
        header.sig[3] == 'B'
    );

    // read asset informations
    {
        uint32 section_size = bvr_fread32_le(file);
        uint16 asset_flag = bvr_fread16_le(file);
        uint32 asset_offset = bvr_fread32_le(file);

        BVR_ASSERT(asset_flag == BVR_EDITOR_ASSETS);

        // detroy current asset stream
        if(!book->asset_stream.data || book->asset_stream.size < section_size){
            bvr_destroy_memstream(&book->asset_stream);
            bvr_create_memstream(&book->asset_stream, section_size);
        }
        else {
            bvr_memstream_clear(&book->asset_stream);
        }

        // copy previously saved asset data stream into the asset stream
        fread(book->asset_stream.data, sizeof(char), section_size, file);
    }

    // read page informations
    {
        bvr_page_t* page = &book->page;
        struct bvri_chunk_data_s camera;

        bvr_destroy_string(&page->name);

        // get scene name
        bvri_read_string(file, &page->name);

        // get camera component
        if(bvr_fread32_le(file)){
            BVR_ASSERT(bvr_freadu16_le(file) == BVR_EDITOR_CAMERA);

            // get camera datas
            float near = bvr_freadf(file);
            float far = bvr_freadf(file);
            float scale = bvr_freadf(file);

            bvr_create_camera(&book->page.camera, &book->window.framebuffer, BVR_CAMERA_ORTHOGRAPHIC, near, far, scale);

            // copy transform
            fread(&page->camera.transform, sizeof(bvr_transform_t), 1, file);            
        }
    }

    // actor chunk
    {
        uint32 readed_bytes = 0;

        uint32 section_size = bvr_fread32_le(file);
        uint16 actor_flag = bvr_fread16_le(file);
        uint32 actor_count = bvr_fread32_le(file);
        uint32 section_start = ftell(file);

        struct {
            uint32 size;
            uint32 offset;

            bvr_string_t name;
            bvr_actor_type_t type;

            bvr_uuid_t id;
            uint8 active;
            int flags;
            uint16 order_in_layer;

            uint32 padding;

            bvr_transform_t transform;
        } target_data;

        BVR_ASSERT(actor_flag == BVR_EDITOR_ACTOR);

        target_data.name.string = NULL;

        // while this section isn't finished
        while (readed_bytes < section_size)
        {
            target_data.size = bvr_fread32_le(file);
            target_data.offset = bvr_fread32_le(file);


            // read binary data
            bvri_read_string(file, &target_data.name);

            target_data.type = bvr_fread32_le(file);
            fread(&target_data.id, sizeof(bvr_uuid_t), 1, file);
            target_data.active = bvr_freadu8_le(file);
            target_data.flags = (int)bvr_fread32_le(file);
            target_data.order_in_layer = bvr_fread16_le(file);
            target_data.padding = 0;
            fread(&target_data.transform, sizeof(bvr_transform_t), 1, file);

            struct bvr_actor_s* target = bvr_find_actor_uuid(book, target_data.id);
            if(target){
                bvr_destroy_string(&target->name);
                bvr_string_create_and_copy(&target->name, &target_data.name);

                target->type = target_data.type;
                target->flags = target_data.flags;
                target->active = target_data.active;
                target->order_in_layer = target_data.order_in_layer;

                memcpy(&target->transform, &target_data.transform, sizeof(bvr_transform_t));
            }

             // actor overwrite 
            switch (target->type)
            {
            case BVR_NULL_ACTOR:
            case BVR_EMPTY_ACTOR:
                break;
            
            case BVR_LAYER_ACTOR:
                {
                    bvr_layer_t* layer;
                    uint32 layer_count = bvr_fread32_le(file);
                    
                    if(layer_count == BVR_BUFFER_COUNT(((bvr_layer_actor_t*)target)->texture.image.layers)){
                        for (size_t i = 0; i < layer_count; i++)
                        {
                            layer = &((bvr_layer_t*)((bvr_layer_actor_t*)target)->texture.image.layers.data)[i];

                            layer->flags = bvr_fread16_le(file);
                            layer->anchor_x = (int)bvr_fread32_le(file);
                            layer->anchor_y = (int)bvr_fread32_le(file);
                            layer->opacity = (short)bvr_fread16_le(file);
                            fread(&layer->blend_mode, sizeof(bvr_layer_blend_mode_t), 1, file);
                        }
                    }
                }
                break;
            case BVR_LANDSCAPE_ACTOR:
                {
                    bvr_landscape_actor_t* landscape = (bvr_landscape_actor_t*)target;

                    // landscape's dimensions
                    landscape->dimension.count[0] = bvr_fread32_le(file);
                    landscape->dimension.count[1] = bvr_fread32_le(file);
                    landscape->dimension.resolution[0] = bvr_fread32_le(file);
                    landscape->dimension.resolution[1] = bvr_fread32_le(file);
                    landscape->dimension.layers = bvr_fread32_le(file);

                    // size of the landscape buffer
                    uint32 landscape_bytes_length = bvr_fread32_le(file);

                    // clamp byte size
                    landscape_bytes_length = MIN(landscape_bytes_length, landscape->mesh.vertex_count * sizeof(int));
                    
                    if(landscape->mesh.array_buffer && landscape->mesh.vertex_buffer){
                        glBindBuffer(GL_ARRAY_BUFFER, landscape->mesh.vertex_buffer);
                        int* map = glMapBufferRange(GL_ARRAY_BUFFER, 0, landscape_bytes_length, GL_MAP_WRITE_BIT);
                        
                        if(map){
                            fread(map, sizeof(char), landscape_bytes_length, file);

                            glUnmapBuffer(GL_ARRAY_BUFFER);
                        }

                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                    }

                }
            default:
                break;
            }

            // make sure to go to the sector actor
            // might delete later
            readed_bytes += target_data.size;
            fseek(file, section_start + readed_bytes, SEEK_SET);
        }
    }

    // binary chunk
    {

    }
}

#pragma endregion