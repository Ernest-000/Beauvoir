#include <bvr/collections/string.h>

// include malloc and realloc here
#include <bvr/common.h>

#include <malloc.h>
#include <string.h>

void bvr_create_string(bvr_string_t* string, const char* value){
    BVR_ASSERT(string);

    string->length = 0;
    string->string = NULL;

    // if there is something to copy
    if(value){
        string->length = strlen(value) + 1;
        string->string = malloc(string->length);
        BVR_ASSERT(string->string);

        // duplicate the string's value
        BVR_STRCPY(string->string, value, string->length);
    }
}

int bvr_overwrite_string(bvr_string_t* string, const char* value, uint32 length){
    BVR_ASSERT(string);

    if(!string->string){
        bvr_create_string(string, value);
        return BVR_FALSE;
    }

    if(length == 0){
        length = strlen(value) + 1;
    }

    if(value){

        if(string->length < length){
            string->string = realloc(string->string, length);
            BVR_ASSERT(string->string);
        }

        string->length = length;
        BVR_STRCPY(string->string, value, string->length);
    }

    return BVR_TRUE;
}

void bvr_string_concat(bvr_string_t* string, const char* other){
    BVR_ASSERT(string);
    
    if(other) {
        
        // string is already allocated
        if(string->string){
            unsigned int size = string->length;

            // new size = size - 1 (EOF) + strlen(other) + 1 (new EOF) 
            string->length = string->length + strlen(other);
            string->string = realloc(string->string, string->length);
            BVR_ASSERT(string->string);

            strcat(string->string, other);
            string->string[string->length - 1] = '\0';
        }
        else {
            string->length = strlen(other) + 1;
            string->string = malloc(string->length);
            
            BVR_STRCPY(string->string, other, string->length);
        }
    }
}

void bvr_string_create_and_copy(bvr_string_t* dest, bvr_string_t* source){
    BVR_ASSERT(dest);

    if(source) {
        dest->length = source->length;
        dest->string = malloc(dest->length);
        BVR_ASSERT(dest->string);

        memcpy(dest->string, source->string, dest->length);
        dest->string[dest->length - 1] = '\0';
    }
}

void bvr_string_insert(bvr_string_t* string, const uint64 offset, const char* value){
    BVR_ASSERT(string);
    BVR_ASSERT(string->string);
    BVR_ASSERT(value);
    
    bvr_string_t prev;
    bvr_string_create_and_copy(&prev, string);

    if(value) {
        const uint64 vlen = strlen(value) + 1;

        string->length += vlen;
        string->string = realloc(string->string, string->length);
        BVR_ASSERT(string->string);

        memset(string->string, 0, string->length);

        strncpy(string->string, prev.string, offset);
        string->string[offset] = '\0';
        strncat(string->string, value, vlen);
        strncat(string->string, &prev.string[offset], prev.length - offset);
    }

    bvr_destroy_string(&prev);
}

void bvr_destroy_string(bvr_string_t* string){
    BVR_ASSERT(string);
    free(string->string);
    string->string = NULL;
    string->length = 0;
}