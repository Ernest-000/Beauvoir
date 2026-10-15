#pragma once

#include <bvr/config.h>
#include <bvr/common.h>
#include <bvr/assets.h>

#include <bvr/shader.h>
#include <bvr/mesh.h>
#include <bvr/image.h>

#include <bvr/book.h>

#ifdef __cplusplus
extern "C" {
#endif


#pragma region deserialize

/**
 * @brief deserialize a json object to get an int.
 * @param token a reference to a serialized object
 * @returns if the token is valid, it will retrieve the object's value, 
 * otherwise it will return 0.
 */
int bvr_deserialize_int32(bvr_fhandle_t token);

/**
 * @brief deserialize a json object to get a long.
 * @param token a reference to a serialized object
 * @returns if the token is valid, it will retrieve the object's value, 
 * otherwise it will return 0.
 */
long bvr_deserialize_int64(bvr_fhandle_t token);

/**
 * @brief deserialize a json object to get an unsigned int.
 * @param token a reference to a serialized object
 * @returns if the token is valid, it will retrieve the object's value, 
 * otherwise it will return 0.
 */
uint32 bvr_deserialize_uint32(bvr_fhandle_t token);

/**
 * @brief deserialize a json object to get an unsigned long.
 * @param token a reference to a serialized object
 * @returns if the token is valid, it will retrieve the object's value, 
 * otherwise it will return 0.
 */
uint64 bvr_deserialize_uint64(bvr_fhandle_t token);

/**
 * @brief deserialize a json object to get a float.
 * @param token a reference to a serialized object
 * @returns if the token is valid, it will retrieve the object's value, 
 * otherwise it will return 0.0.
 */
float bvr_deserialize_float(bvr_fhandle_t token);

/**
 * @brief deserialize a json object to get a float array
 * @param token a reference to a serialized object.
 * @param array a reference to a previously allocated array of N size.
 * @param length the count of element in the array. 
 */
int bvr_deserialize_farray(bvr_fhandle_t token, float* array, uint32 length);

/**
 * @brief deserialize a json object to get a vec2
 * @param token a reference to a serialized object.
 * @param vec a reference to an existing vector.
 */
BVR_H_FUNC int bvr_deserialize_vec2(bvr_fhandle_t token, vec2 vec){
    return bvr_deserialize_farray(token, (float*)vec, 2);
}

/**
 * @brief deserialize a json object to get a vec2
 * @param token a reference to a serialized object.
 * @param vec a reference to an existing vector.
 */
BVR_H_FUNC int bvr_deserialize_vec3(bvr_fhandle_t token, vec3 vec){
    return bvr_deserialize_farray(token, (float*)vec, 3);
}

/**
 * @brief deserialize a json object to get a vec2
 * @param token a reference to a serialized object.
 * @param vec a reference to an existing vector.
 */
BVR_H_FUNC int bvr_deserialize_vec4(bvr_fhandle_t token, vec4 vec){
    return bvr_deserialize_farray(token, (float*)vec, 4);
}

/**
 * @brief deserialize a json object to get a boolean.
 * @param token a reference to a serialized object
 * @returns if the token is valid, it will retrieve the object's value, 
 * otherwise it will return false.
 */
bool bvr_deserialize_bool(bvr_fhandle_t token);

/**
 * @brief deserialize a json object to get a new string.
 * @param token a reference to a serialized object
 * @param string a pointer to the string to create
 * @returns if the token is valid, it will retrieve and create a new string, 
 * otherwise it will create an empty string.
 */
void bvr_deserialize_string(bvr_fhandle_t token, bvr_string_t* string);

/**
 * @brief deserialize a json object to get a new file handle. 
 * If the token is valid, it will retrieve and create a new
 * file handle, otherwise, it will create a NULL handle.
 * @param token a reference to a serialized object
 * @param string a pointer to the handle to create
 * @returns return TRUE if the file handle is successfully deserialized.
 */
int bvr_deserialize_fhandle(bvr_fhandle_t token, bvr_fhandle_t* handle);

/**
 * @brief deserialize a json object to get a new pointer handle. 
 * If the token is valid, it will retrieve and create a new
 * pointer handle, otherwise, it will create a NULL handle.
 * @param token a reference to a serialized object
 * @param string a pointer to the handle to create
 * @returns return TRUE if the pointer handle is successfully deserialized.
 */
int bvr_deserialize_phandle(bvr_fhandle_t token, bvr_phandle_t* handle);

/**
 * @brief deserialize a json object to get a new transform structure.
 * If the token is valid, it will retrieve and create a new transform,
 * otherwise, it will create an empty tranform.
 * @param token a reference to a serialize object
 * @param transform a pointer to the transform to create
 * @returns return TRUE if the transform is successfully deserialized.
 */
int bvr_deserialize_transform(bvr_fhandle_t token, bvr_transform_t* transform);

/**
 * @brief deserialize a json object to get a new instantiated and allocated mesh object.
 * If the token is valid, it will retrieve and create a new mesh,
 * otherwise, it will create an empty mesh.
 * @param token a reference to a serialize object
 * @param mesh a pointer to the mesh to create
 * @returns return TRUE if the mesh is successfully deserialized.
 */
int bvr_deserialize_mesh(bvr_fhandle_t token, bvr_mesh_t* mesh);

/**
 * @brief deserialize a json object to get a new instantiated and allocated shader object.
 * If the token is valid, it will retrieve and create a new shader,
 * otherwise, it will create an empty shader.
 * @param token a reference to a serialize object
 * @param shader a pointer to the shader to create
 * @returns return TRUE if the shader is successfully deserialized.
 */
int bvr_deserialize_shader(bvr_fhandle_t token, bvr_shader_t* shader);

/**
 * @brief deserialize a json object to get a new camera object.
 * If the token is valid, it will retrieve and create a new camera,
 * otherwise, it will create an empty camera.
 * @param token a reference to a serialize object
 * @param camera a pointer to the camera to create
 * @returns return TRUE if the camera is successfully deserialized.
 */
int bvr_deserialize_camera(bvr_fhandle_t token, bvr_camera_t* camera);

/**
 * @brief deserialize a json object to get a new valid page.
 * If the token is valid, it will retrieve and create a new page,
 * otherwise, it will create an empty page.
 * @param token a reference to a serialize object
 * @param page a pointer to the page to create
 * @returns return TRUE if the page is successfully deserialized.
 */
int bvr_deserialize_page(bvr_fhandle_t token, bvr_page_t* page);

#pragma endregion deserialize

#pragma region serialize

#pragma endregion serialize

#ifdef __cplusplus
}
#endif