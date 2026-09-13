#pragma once

#include <bvr/config.h>
#include <bvr/common.h>
#include <bvr/assets.h>

#include <bvr/shader.h>
#include <bvr/mesh.h>
#include <bvr/image.h>

#include <bvr/book.h>

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

int bvr_deserialize_mesh(bvr_fhandle_t token, bvr_mesh_t* mesh);

int bvr_deserialize_shader(bvr_fhandle_t token, bvr_shader_t* shader);