#include <bvr/io.h>
#include <bvr/common.h>

#include <malloc.h>
#include <memory.h>

uint64 bvr_fsize(FILE* file){
    uint64 cursor = ftell(file);
    
    fseek(file, 0, SEEK_END);
    uint64 size = ftell(file);
    fseek(file, cursor, SEEK_SET);
    
    return size;
}

int bvr_read_file(bvr_string_t* string, FILE* file){
    BVR_ASSERT(string);
    BVR_ASSERT(file);

    if(string->string){
        BVR_ASSERT(0 && "cannot copy on a previously allocated string :(");
    }
    else {
        // TODO: check if size is correct
        string->length = bvr_fsize(file) - ftell(file);
        string->string = malloc(string->length + 1);
        BVR_ASSERT(string->string);

        uint64 readed_bytes = fread(string->string, sizeof(char), string->length, file);
        BVR_ASSERT(readed_bytes == string->length);

        string->string[string->length] = '\0';

    }

    return BVR_TRUE;
}

short bvr_fread16_le(FILE* file){
    uint8 a, b;
    a = bvr_freadu8_le(file);
    b = bvr_freadu8_le(file);
    return (short)((b << 8) | a);
}

int bvr_fread24_le(FILE* file){
    uint8 a, b, c, d;
    a = 0;
    b = bvr_freadu8_le(file);
    c = bvr_freadu8_le(file);
    d = bvr_freadu8_le(file);
    return (int)((((d << 8) | c) << 8 | b) << 8 | a);
}

int bvr_fread32_le(FILE* file){
    uint8 a, b, c, d;
    a = bvr_freadu8_le(file);
    b = bvr_freadu8_le(file);
    c = bvr_freadu8_le(file);
    d = bvr_freadu8_le(file);
    return (int)((((d << 8) | c) << 8 | b) << 8 | a);
}

int64 bvr_fread64_le(FILE* file){
    int a, b;
    a = bvr_fread32_le(file);
    b = bvr_fread32_le(file);
    return (long)(b << 32) | a;
}

uint8 bvr_freadu8_le(FILE* file){
    //int v = getc(file);
    // if(v == EOF){
    //     BVR_PRINTF("failed to read character %i", errno);
    //     return 0;
    // }

    return (uint8)getc(file);
}

float bvr_freadf(FILE* file){
    float f;
    fread(&f, sizeof(float), 1, file);
    return f;
}

void bvr_freadstr(char* string, uint64 size, FILE* file){
    if(string){
        fread(string, sizeof(uint8), size - 1, file);
        string[size - 1] = '\0';
    }
}

uint16 bvr_freadu16_le(FILE* file){
    uint8 a, b;
    a = bvr_freadu8_le(file);
    b = bvr_freadu8_le(file);
    return (uint16)((b << 8) | a);
}

uint32 bvr_freadu32_le(FILE* file){
    uint8 a, b, c, d;
    a = bvr_freadu8_le(file);
    b = bvr_freadu8_le(file);
    c = bvr_freadu8_le(file);
    d = bvr_freadu8_le(file);
    return (uint32)((((d << 8) | c) << 8 | b) << 8 | a);
}

uint64 bvr_freadu64_le(FILE* file){
    uint32 a, b;
    a = bvr_fread32_le(file);
    b = bvr_fread32_le(file);
    return (uint64)(b << 32) | a;
}

short bvr_mread16_le(uint8** ptr){
    uint8 a, b;
    a = bvr_mread8_le(ptr);
    b = bvr_mread8_le(ptr);
    return (short)((b << 8) | a);
}

int bvr_mread32_le(uint8** ptr){
    uint8 a, b, c, d;
    a = bvr_mread8_le(ptr);
    b = bvr_mread8_le(ptr);
    c = bvr_mread8_le(ptr);
    d = bvr_mread8_le(ptr);
    return (int)((((d << 8) | c) << 8 | b) << 8 | a);
}

uint32 bvr_mreadu32_le(uint8** ptr){
    uint8 a, b, c, d;
    a = bvr_mread8_le(ptr);
    b = bvr_mread8_le(ptr);
    c = bvr_mread8_le(ptr);
    d = bvr_mread8_le(ptr);
    return (uint32)((((d << 8) | c) << 8 | b) << 8 | a);
}