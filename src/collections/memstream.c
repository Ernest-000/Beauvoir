#include <bvr/collections/memstream.h>

#include <bvr/common.h>

#include <malloc.h>
#include <string.h>

void bvr_create_memstream(bvr_memstream_t* stream, const uint64 size){
    BVR_ASSERT(stream);

    if(stream->data){
        return;
    }

    stream->data = NULL;
    stream->size = size;
    stream->cursor = NULL;

    if(size){
        stream->data = malloc(size);
        stream->cursor = stream->data;
        stream->next = stream->data;
        BVR_ASSERT(stream->data);

        memset(stream->data, 0, stream->size);
    }
}

char* bvr_memstream_write(bvr_memstream_t* stream, const void* data, const uint64 size){
    BVR_ASSERT(stream && stream->data);

    // if there is available space
    if(stream->cursor - (char*)stream->data + size < stream->size){
        if(data){
            memcpy(stream->cursor, data, size);
        }

        stream->cursor += size;
        stream->next += size;
    }
    else {
        BVR_BUFFER_REALLOC(*stream, size);
        
        if(data){
            memcpy(stream->cursor, data, size);

            stream->cursor += size;
            stream->next += size;
        }
    }

    return stream->cursor;
}

char* bvr_memstream_read(bvr_memstream_t* stream, void* dest, const uint64 size){
    BVR_ASSERT(stream && stream->data);
    BVR_ASSERT(dest);

    if(stream->cursor - (char*)stream->data + size < stream->size){
        memcpy(dest, stream->cursor, size);
        stream->cursor += size;
    }
    else {
        BVR_ASSERT(0 && "out of bounds!");
    }

    return stream->cursor;
}

char* bvr_memstream_seek(bvr_memstream_t* stream, uint64 position, int mode){
    BVR_ASSERT(stream);

    switch (mode)
    {
    case SEEK_CUR:
        {
            if(stream->cursor - (char*)stream->data + position < stream->size){
                stream->cursor += position;
            } 
            else {
                BVR_ASSERT(0 && "out of bounds!");
            }
        }
        break;

    case SEEK_SET:
        {
            if(position <= stream->size){
                stream->cursor = stream->data + position;
            }
            else {
                BVR_ASSERT(0 && "out of bounds!");
            }
        }
        break;

    case SEEK_END:
        {
            if(position <= stream->size){
                stream->cursor = stream->data + (stream->size - position);
            }
            else {
                BVR_ASSERT(0 && "out of bounds!");
            }
        }
        break;

    case SEEK_NEXT:
        {
            stream->cursor = stream->next;
        }
        break;

    default:
        BVR_ASSERT(0 && "invalid seeking mode!");
        break;
    }

    return stream->cursor;
}

void bvr_memstream_clear(bvr_memstream_t* stream){
    BVR_ASSERT(stream);

    stream->cursor = stream->data;
    stream->next = stream->data;

    // disabled for now, data will be overwritten
    //memset(stream->data, 0, stream->size);
}

void bvr_destroy_memstream(bvr_memstream_t* stream){
    BVR_ASSERT(stream);

    // clear all stream data
    // so that it will be an empty chunk if we're trying to access
    // to the stream's data after it has been freed.
    memset(stream->data, 0, stream->size);

    free(stream->data);

    stream->size = 0;
    stream->cursor = NULL;
    stream->data = NULL;
}