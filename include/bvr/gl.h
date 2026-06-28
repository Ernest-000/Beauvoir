#pragma once

#ifndef BVR_GLENTRIY
#define BVR_GLENTRIY

#ifdef _cplusplus
extern "C" {
#endif

/*
    Android and apple must use the gles version
    Unix should use the gles because it will switch to the compat mode of MESA
    which allow a best compatibility (and idk why, this will catch errors)
*/
#if defined(__ANDROID__) || defined(__APPLE__) || defined(__unix__)
    #define BVR_USE_GLES 1

/*
    Windows system uses opengl desktop
*/
#elif _WIN32 
    #define BVR_USE_GLDESK 1
#else
    #error cannot find a correct opengl configuration
    #define BVR_USE_GLDESK 0
    #define BVR_USE_GLES 0
#endif

#if defined(BVR_USE_GLES)
    #include <glad/glad.es.h>
#else
    #include <glad/glad.core.h>
#endif

#ifndef BVR_H_GL
#define BVR_H_GL 

// wrappers
#if BVR_USE_GLDESK 
    #define GL_PIXEL_STOREI(pname, pvalue) glPixelStorei(pname, pvalue)
    #define GL_READ_PIXEL(x, y, width, height, format, type, bufsize, pixels) \
        glReadPixels(x, y, width, height, format, type, pixels)

    #define GL_TEX_STORAGE_2D(target, format, type, internal_format, width, height) \
        glTexImage2D(target, 0, internal_format, width, height, 0, format, type, NULL)

    #define GL_TEX_STORAGE_3D(target, format, type, internal_format, width, height, depth) \
        glTexImage3D(target, 0, internal_format, width, height, depth, 0, format, type, NULL)
#elif BVR_USE_GLES
    #define GL_PIXEL_STOREI(pname, pvalue) {}
    #define GL_READ_PIXEL(x, y, width, height, format, type, bufsize, pixels) \
        glReadnPixels(x, y, width, height, format, type, bufsize, pixels)
    
    #define GL_TEX_STORAGE_2D(target, format, type, internal_format, width, height) \
        glTexStorage2D(target, 1, internal_format, width, height)

    #define GL_TEX_STORAGE_3D(target, format, type, internal_format, width, height, depth) \
        glTexStorage3D(target, 1, internal_format, width, height, depth)
#else
// no-op
#endif

// loader functions

int bvr_load_gles(GLADloadproc proc);
int bvr_load_glcore(GLADloadproc proc);
int bvr_load_gl(GLADloadproc proc);

#endif

#ifdef _cplusplus
}
#endif

#endif