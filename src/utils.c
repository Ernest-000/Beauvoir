#include <BVR/utils.h>

#include <BVR/config.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32
	#include <Windows.h>
#elif
	#include <signal.h>
#endif

int bvr_sizeof(const int type){
    switch (type)
    {
        case BVR_INT8: case BVR_UNSIGNED_INT8: return 1;
        case BVR_INT16: case BVR_UNSIGNED_INT16: return 2;
        case BVR_FLOAT: 
        case BVR_INT32: case BVR_UNSIGNED_INT32: return 4;
        case BVR_DOUBLE: case BVR_INT64: case BVR_UNSIGNED_INT64: return 8;
        case BVR_VEC2: return 8;
        case BVR_VEC3: return 12;
        case BVR_VEC4: return 16;
        case BVR_MAT3: return 36;
        case BVR_MAT4: return 64;
        case BVR_TEXTURE_2D:
        case BVR_TEXTURE_2D_ARRAY: return 8;
        default: return 0;
    }
}

void bvr_nameof(const int type, char* name){
    switch (type)
    {
        case BVR_INT8: 
            strcpy(name, "INT8");
            return;

        case BVR_UNSIGNED_INT8:
            strcpy(name, "UNSIGNED_INT8");
            return;

        case BVR_INT16: 
            strcpy(name, "INT16");
            return;

        case BVR_UNSIGNED_INT16:
            strcpy(name, "UNSIGNED_INT16");
            return;
 
        case BVR_FLOAT:
            strcpy(name, "FLOAT");
            return;
 
        case BVR_INT32:
            strcpy(name, "INT32");
            return;

        case BVR_UNSIGNED_INT32: 
            strcpy(name, "UNSIGNED_INT32");
            return;

        case BVR_VEC2: 
            strcpy(name, "VEC2");
            return;

        case BVR_VEC3: 
            strcpy(name, "VEC3");
            return;

        case BVR_VEC4: 
            strcpy(name, "VEC4");
            return;

        case BVR_MAT3: 
            strcpy(name, "MAT3");
            return;

        case BVR_MAT4: 
            strcpy(name, "MAT4");
            return;
        case BVR_TEXTURE_2D:
            strcpy(name, "TEXTURE_2D");
            return;
        
        case BVR_TEXTURE_2D_ARRAY:
            strcpy(name, "TEXTURE_2D_ARRAY");
            return;

        default:
            return;
    }
}

/*
    https://gist.github.com/sgsfak/9ba382a0049f6ee885f68621ae86079b
    
    The Java hash, but really no-one seems to know where it came from, see
    https://bugs.java.com/bugdatabase/view_bug.do?bug_id=4045622
*/
unsigned int bvr_hash(const char* string)
{
    unsigned int h = 0;
    unsigned int len = strlen(string);

    while (len) {
        h = 31 * h + *string++;
        --len;
    }
    return h;
}

void bvr_create_uuid(bvr_uuid_t uuid){
    const char hex_digits[] = "0123456789abcdefABCDEF";

    for (uint64 i = 0; i < 36; i++)
    {
        (uuid)[i] = hex_digits[rand() % 22];
    }
    
    uuid[8] = '-';
    uuid[13] = '-';
    uuid[18] = '-';
    uuid[23] = '-';
    uuid[36] = '\0';
}

void bvr_copy_uuid(bvr_uuid_t src, bvr_uuid_t dest){
    memcpy(dest, src, sizeof(bvr_uuid_t));
}

int bvr_uuid_equals(bvr_uuid_t const a, bvr_uuid_t const b){
    return strncmp(a, b, sizeof(bvr_uuid_t)) == 0;
}

#ifdef BVR_INCLUDE_DEBUG

#define BVR_UTILS_BUFFER_SIZE 100

static char bvri_debug_buffer[BVR_UTILS_BUFFER_SIZE];

char* bvri_string_format(const char* __string, ...){
	va_list arg_list;
	int f;
	
	va_start(arg_list, __string);
	f = vsnprintf(bvri_debug_buffer, BVR_UTILS_BUFFER_SIZE, __string, arg_list);
	va_end(arg_list);
	
	return &bvri_debug_buffer[0];
}

char* bvri_get_buffer(){
    return &bvri_debug_buffer[0];
}

void bvri_wmessage(FILE* __stream, const int __line, const char* __file, const char* __message, ...){
    va_list arg_list;
    int f;

    if(__line > 0 || __file){
        fprintf(__stream, "%s:%i ", __file, __line);
    }

    va_start(arg_list, __message);
    f = vfprintf(__stream, __message, arg_list);
    va_end(arg_list);

    fprintf(__stream, "\n");
}

void bvri_wassert(const char* __message, const char* __file, unsigned long long __line){
    bvri_wmessage(stderr, -1, 0, "assertion failed: %s, %s, line %i\n", __message, __file, __line);

    exit(0);
}

void bvri_wassert_break(const char* __message, const char* __file, unsigned long long __line){
    bvri_wmessage(stderr, -1, 0, "assertion failed: %s, %s, line %i\n", __message, __file, __line);

	bvri_break(__file, __line);
}

int bvri_werror(const char* __message, int __code){
    assert(0 && "not implemented!");
}

void bvri_break(const char* __file, unsigned long long __line){	
	bvri_wmessage(stderr, __line, __file, "program break!");

#ifdef _WIN32
	DebugBreak();
#elif
	raise(SIGINT);
#endif
}

#endif