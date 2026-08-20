#pragma once

#include <bvr/config.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>

typedef float vec2[2];
typedef float vec3[3];
typedef float vec4[4];
typedef float quat[4];

typedef vec4 mat4x4[4];

struct bvr_transform_s {
    vec3 position;
    vec3 scale;
    quat rotation;

    mat4x4 local;
    mat4x4 world;
} __struct_align16;

typedef struct bvr_transform_s bvr_transform_t;

struct bvr_bounds_s {
    vec2 coords;
    float width;
    float height;
};

#if !defined(BVR_PI)
    #define BVR_PI 3.14159265358979323846
#endif

#if !defined(BVR_HALF_PI)
    #define BVR_HALF_PI (BVR_PI / 2.0)
#endif

/**
 * @brief create a new vector 2 from any number typed array.
 * @param vec the vector object that will be defined.
 * @param x the x value.
 * @param y the y value.
 */
#define BVR_CREATE_VEC2(vec, x, y) vec[0] = x; vec[1] = y;

/**
 * @brief create a new vector 3 from any number typed array.
 * @param vec the vector object that will be defined.
 * @param x the x value.
 * @param y the y value.
 * @param z the z value.
 */
#define BVR_CREATE_VEC3(vec, x, y, z) vec[0] = x; vec[1] = y; vec[2] = z;

/**
 * @brief create a new vector 4 from any number typed array.
 * @param vec the vector object that will be defined.
 * @param x the x value.
 * @param y the y value.
 * @param z the z value.
 * @param w the w value.
 */
#define BVR_CREATE_VEC4(vec, x, y, z, w) vec[0] = x; vec[1] = y; vec[2] = z; vec[3] = w;

/**
 * @brief set all values of a vector 2 to a.
 * @param vec the vector object that will be defined.
 * @param a the new value of each vector's components.
 */
#define BVR_SET_VEC2(vec, a) vec[0] = a; vec[1] = a;

/**
 * @brief set all values of a vector 3 to a.
 * @param vec the vector object that will be defined.
 * @param a the new value of each vector's components.
 */
#define BVR_SET_VEC3(vec, a) vec[0] = a; vec[1] = a; vec[2] = a;

/**
 * @brief set all values of a vector 4 to a.
 * @param vec the vector object that will be defined.
 * @param a the new value of each vector's components.
 */
#define BVR_SET_VEC4(vec, a) vec[0] = a; vec[1] = a; vec[2] = a; vec[3] = a;

/**
 * @brief set a vector 2 of any number to (0, 0).
 * @param vec the vector object that will be defined.
 */
#define BVR_IDENTITY_VEC2(vec) BVR_SET_VEC2(vec, 0.0f);

/**
 * @brief set a vector 3 of any number to (0, 0, 0).
 * @param vec the vector object that will be defined.
 */
#define BVR_IDENTITY_VEC3(vec) BVR_SET_VEC3(vec, 0.0f);

/**
 * @brief set a vector 4 of any number to (0, 0, 0, 0).
 * @param vec the vector object that will be defined.
 */
#define BVR_IDENTITY_VEC4(vec) BVR_SET_VEC4(vec, 0.0f);

/**
 * @brief set a matrix 4x4 of any number to's identity value.
 * @param mat the matrix object array that will be defined.
 */
#define BVR_IDENTITY_MAT4(mat)  mat[0][0] = 1.0f;\
                                mat[0][1] = 0.0f;\
                                mat[0][2] = 0.0f;\
                                mat[0][3] = 0.0f;\
                                mat[1][0] = 0.0f;\
                                mat[1][1] = 1.0f;\
                                mat[1][2] = 0.0f;\
                                mat[1][3] = 0.0f;\
                                mat[2][0] = 0.0f;\
                                mat[2][1] = 0.0f;\
                                mat[2][2] = 1.0f;\
                                mat[2][3] = 0.0f;\
                                mat[3][0] = 0.0f;\
                                mat[3][1] = 0.0f;\
                                mat[3][2] = 0.0f;\
                                mat[3][3] = 1.0f;

/*
    Define each transform's values. 
    x, y, z sets x, y and z position values
    roll, pitch, yaw sets roll, pitch and yaw rotations values
    scale sets transform's uniform scale
*/
#define BVR_TRANSFORM(tr, x, y, z, roll, pitch, yaw, scalev) \
    tr.position[0] = x;                                     \
    tr.position[1] = y;                                     \
    tr.position[2] = z;                                     \
    tr.rotation[0] = roll;                                  \
    tr.rotation[1] = pitch;                                 \
    tr.rotation[2] = yaw;                                   \
    BVR_SET_VEC3(tr.scale, scalev)                         

// returns the smallest of a or b
#define MIN(a,b) (((a)<(b))?(a):(b))

// returns the biggest of a or b
#define MAX(a,b) (((a)>(b))?(a):(b))

/**
 * @brief return the next power of two of x.
 */
BVR_H_FUNC int npow2(int x){
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    // x |= x >> 32;
    return x;
}

/**
 * @brief linearly interpolate between a and b by t.
 * @param a the start value.
 * @param b the end value.
 * @param t the interpolation value.
 * @returns the interpolated value between a and b by t.
 */
BVR_H_FUNC double lerp(double a, double b, double t){
    return a + t * (b - a);
}

/**
 * @brief linearly interpolate between a and b by t as floating numbers.
 * @param a the start value.
 * @param b the end value.
 * @param t the interpolation value.
 * @returns the interpolated value between a and b by t.
 */
BVR_H_FUNC float flerp(float a, float b, float t){
    return a + t * (b - a);
}

/**
 * @brief Linearly interpolate between a and b by t as integers.
 * @param a the start value.
 * @param b the end value.
 * @param t the interpolation value.
 * @returns The interpolated value between a and b by t.
 */
BVR_H_FUNC int ilerp(int a, int b, int t){
    return a + t * (b - a);
}

/**
 * @brief convert an angle in radians to degrees.
 * @param rad the angle in radians.
 * @returns the angle expressed in degrees.
 */
BVR_H_FUNC float rad_to_deg(float rad) { 
    return rad * 180 / BVR_PI; 
}

/**
 * @brief convert an angle in degrees to radians.
 * @param deg the angle in degrees.
 * @returns the angle expressed in radians.
 */
BVR_H_FUNC float deg_to_rad(float deg) { 
    return deg * BVR_PI / 180; 
}

/**
 * @brief clamp a floating point value between a minimum and a maximum.
 * @param d the value to clamp.
 * @param min the lower bound.
 * @param max the upper bound.
 * @returns d clamped between min and max.
 */
BVR_H_FUNC float clamp(float d, float min, float max) {
  const float t = d < min ? min : d;
  return t > max ? max : t;
}

/**
 * @brief clamp an integer value between a minimum and a maximum.
 * @param d the value to clamp.
 * @param min the lower bound.
 * @param max the upper bound.
 * @returns d clamped between min and max.
 */
BVR_H_FUNC int clampi(int d, int min, int max) {
  const int t = d < min ? min : d;
  return t > max ? max : t;
}

/**
 * @brief add two vector 2 together.
 * @param result the vector that will receive the result of a + b.
 * @param a the first vector.
 * @param b the second vector.
 */
BVR_H_FUNC void vec2_add(vec2 result, vec2 const a, vec2 const b){
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
}

/**
 * @brief subtract a vector 2 from another.
 * @param result the vector that will receive the result of a - b.
 * @param a the first vector.
 * @param b the second vector.
 */
BVR_H_FUNC void vec2_sub(vec2 result, vec2 const a, vec2 const b){
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
}

/**
 * @brief scale a vector 2 by a scalar value.
 * @param result the vector that will receive the scaled result.
 * @param a the vector to scale.
 * @param s the scale factor.
 */
BVR_H_FUNC void vec2_scale(vec2 result, vec2 const a, float const s){
    result[0] = a[0] * s;
    result[1] = a[1] * s;
}

/**
 * @brief compute the dot product of two vector 2.
 * @param a the first vector.
 * @param b the second vector.
 * @returns the dot product of a and b.
 */
BVR_H_FUNC float vec2_dot(vec2 const a, vec2 const b){
    return a[0] * b[0] + a[1] * b[1];
}

/**
 * @brief compute the 2D cross product (z component) of two vector 2.
 * @param a the first vector.
 * @param b the second vector.
 * @returns the scalar cross product of a and b.
 */
BVR_H_FUNC float vec2_cross(vec2 const a, vec2 const b){
    return a[0] * b[1] - a[1] * b[0];
}

/**
 * @brief compute the length (magnitude) of a vector 2.
 * @param v the vector.
 * @returns the length of v.
 */
BVR_H_FUNC float vec2_len(vec2 const v){
    return sqrtf(vec2_dot(v, v));
}

/**
 * @brief normalize a vector 2 to a unit length vector.
 * @param result the vector that will receive the normalized result.
 * @param v the vector to normalize.
 */
BVR_H_FUNC void vec2_norm(vec2 result, vec2 const v){
    vec2_scale(result, v, 1.0f / vec2_len(v));
}

/**
 * @brief copy the values of a vector 2 into another.
 * @param result the vector that will receive the copied values.
 * @param a the vector to copy.
 */
BVR_H_FUNC void vec2_copy(vec2 result, vec2 const a){
    result[0] = a[0];
    result[1] = a[1];
}

/**
 * @brief add two vector 3 together.
 * @param result the vector that will receive the result of a + b.
 * @param a the first vector.
 * @param b the second vector.
 */
BVR_H_FUNC void vec3_add(vec3 result, vec3 const a, vec3 const b){
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
}

/**
 * @brief subtract a vector 3 from another.
 * @param result the vector that will receive the result of a - b.
 * @param a the first vector.
 * @param b the second vector.
 */
BVR_H_FUNC void vec3_sub(vec3 result, vec3 const a, vec3 const b){
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
}

/**
 * @brief scale a vector 3 by a scalar value.
 * @param result the vector that will receive the scaled result.
 * @param a the vector to scale.
 * @param s the scale factor.
 */
BVR_H_FUNC void vec3_scale(vec3 result, vec3 const a, float const s){
    result[0] = a[0] * s;
    result[1] = a[1] * s;
    result[2] = a[2] * s;
}

/**
 * @brief compute the dot product of two vector 3.
 * @param a the first vector.
 * @param b the second vector.
 * @returns the dot product of a and b.
 */
BVR_H_FUNC float vec3_dot(vec3 const a, vec3 const b){
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

/**
 * @brief compute the length (magnitude) of a vector 3.
 * @param v the vector.
 * @returns the length of v.
 */
BVR_H_FUNC float vec3_len(vec3 const v){
    return sqrtf(vec3_dot(v, v));
}

/**
 * @brief normalize a vector 3 to a unit length vector.
 * @param result the vector that will receive the normalized result.
 * @param v the vector to normalize.
 */
BVR_H_FUNC void vec3_norm(vec3 result, vec3 const v){
    vec3_scale(result, v, 1.0f / vec3_len(v));
}

/**
 * @brief copy the values of a vector 3 into another.
 * @param result the vector that will receive the copied values.
 * @param a the vector to copy.
 */
BVR_H_FUNC void vec3_copy(vec3 result, vec3 const a){
    result[0] = a[0];
    result[1] = a[1];
    result[2] = a[2];
}                             

/**
 * @brief compute the cross product of two vector 3.
 * @param result the vector that will receive the cross product of a and b.
 * @param a the first vector.
 * @param b the second vector.
 */
BVR_H_FUNC void vec3_mul_cross(vec3 result, vec3 const a, vec3 const b){
    result[0] = a[1]*b[2] - a[2]*b[1];
	result[1] = a[2]*b[0] - a[0]*b[2];
	result[2] = a[0]*b[1] - a[1]*b[0];
}

/**
 * @brief add two vector 4 together.
 * @param result the vector that will receive the result of a + b.
 * @param a the first vector.
 * @param b the second vector.
 */
BVR_H_FUNC void vec4_add(vec4 result, vec4 const a, vec4 const b){
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
    result[3] = a[3] + b[3];
}

/**
 * @brief subtract a vector 4 from another.
 * @param result the vector that will receive the result of a - b.
 * @param a the first vector.
 * @param b the second vector.
 */
BVR_H_FUNC void vec4_sub(vec4 result, vec4 const a, vec4 const b){
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
    result[3] = a[3] - b[3];
}

/**
 * @brief scale a vector 4 by a scalar value.
 * @param result the vector that will receive the scaled result.
 * @param a the vector to scale.
 * @param s the scale factor.
 */
BVR_H_FUNC void vec4_scale(vec4 result, vec4 const a, float const s){
    result[0] = a[0] * s;
    result[1] = a[1] * s;
    result[2] = a[2] * s;
    result[3] = a[3] * s;
}

/**
 * @brief compute the dot product of two vector 4.
 * @param a the first vector.
 * @param b the second vector.
 * @returns the dot product of a and b.
 */
BVR_H_FUNC float vec4_dot(vec4 const a, vec4 const b){
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

/**
 * @brief compute the length (magnitude) of a vector 4.
 * @param v the vector.
 * @returns the length of v.
 */
BVR_H_FUNC float vec4_len(vec4 const v){
    return sqrtf(vec4_dot(v, v));
}

/**
 * @brief normalize a vector 4 to a unit length vector.
 * @param result the vector that will receive the normalized result.
 * @param v the vector to normalize.
 */
BVR_H_FUNC void vec4_norm(vec4 result, vec4 const v){
    vec4_scale(result, v, 1.0f / vec4_len(v));
}

/**
 * @brief copy the values of a vector 4 into another.
 * @param result the vector that will receive the copied values.
 * @param a the vector to copy.
 */
BVR_H_FUNC void vec4_copy(vec4 result, vec4 const a){
    result[0] = a[0];
    result[1] = a[1];
    result[2] = a[2];
    result[3] = a[3];
}                             

/**
 * @brief compute the cross product of two vector 4 (xyz components), setting w to 1.
 * @param result the vector that will receive the cross product of a and b.
 * @param a the first vector.
 * @param b the second vector.
 */
BVR_H_FUNC void vec4_mul_cross(vec4 result, vec4 const a, vec4 const b){
    result[0] = a[1]*b[2] - a[2]*b[1];
	result[1] = a[2]*b[0] - a[0]*b[2];
	result[2] = a[0]*b[1] - a[1]*b[0];
    result[3] = 1.0f;
}

/**
 * @brief build an orthographic projection matrix.
 * @param result the matrix that will receive the orthographic projection.
 * @param left the left clipping plane.
 * @param right the right clipping plane.
 * @param bottom the bottom clipping plane.
 * @param top the top clipping plane.
 * @param near the near clipping plane.
 * @param far the far clipping plane.
 */
BVR_H_FUNC void mat4_ortho(mat4x4 result, float left, float right, float bottom, float top, float near, float far){
    float rl = 1.0f / (right - left);
    float tb = 1.0f / (top - bottom);
    float fn = 1.0f - (far - near);

    result[0][0] = 2.0f * rl;
    result[0][1] = 0.0f;
    result[0][2] = 0.0f;
    result[0][3] = 0.0f;

    result[1][0] = 0.0f;
    result[1][1] = 2.0f * tb;
    result[1][2] = 0.0f;
    result[1][3] = 0.0f;

    result[2][0] = 0.0f;
    result[2][1] = 0.0f;
    result[2][2] = -2.0f * fn;
    result[2][3] = 0.0f;
    
    result[3][0] = -(right + left) * rl;
    result[3][1] = -(top + bottom) * tb;
    result[3][2] = -(far + near) * fn;
    result[3][3] = 1.0f;
}


/**
 * @brief copy a matrix to another
 * @param result the matrix that will be copied to.
 * @param mat the matrix where the values will came from.
 */
BVR_H_FUNC void mat4_copy(mat4x4 result, mat4x4 mat){
    vec4_copy(result[0], mat[0]);
    vec4_copy(result[1], mat[1]);
    vec4_copy(result[2], mat[2]);
    vec4_copy(result[3], mat[3]);
}

/**
 * @brief multiply a 4x4 matrix by a vector 4.
 * @param result the vector that will receive the result of mat * vec.
 * @param mat the 4x4 matrix.
 * @param vec the vector to multiply.
 */
BVR_H_FUNC void mat4_mul_vec4(vec4 result, mat4x4 const mat, vec4 const vec){
    result[0] = (mat[0][0] + mat[0][1] + mat[0][2] + mat[0][3]) * vec[0];
    result[1] = (mat[1][0] + mat[1][1] + mat[1][2] + mat[1][3]) * vec[1];
    result[2] = (mat[2][0] + mat[2][1] + mat[2][2] + mat[2][3]) * vec[2];
    result[3] = (mat[3][0] + mat[3][1] + mat[3][2] + mat[3][3]) * vec[3];
}

/**
 * @brief multiply two 4x4 matrices together.
 * @param result the matrix that will receive the result of a * b.
 * @param a the first matrix.
 * @param b the second matrix.
 */
BVR_H_FUNC void mat4_mul(mat4x4 result, mat4x4 const a, mat4x4 const b){
    mat4x4 temp;
    temp[0][0] = a[0][0]*b[0][0] + a[1][0]*b[0][1] + a[2][0]*b[0][2] + a[3][0]*b[0][3];
    temp[0][1] = a[0][1]*b[0][0] + a[1][1]*b[0][1] + a[2][1]*b[0][2] + a[3][1]*b[0][3];
    temp[0][2] = a[0][2]*b[0][0] + a[1][2]*b[0][1] + a[2][2]*b[0][2] + a[3][2]*b[0][3];
    temp[0][3] = a[0][3]*b[0][0] + a[1][3]*b[0][1] + a[2][3]*b[0][2] + a[3][3]*b[0][3];
    
    temp[1][0] = a[0][0]*b[1][0] + a[1][0]*b[1][1] + a[2][0]*b[1][2] + a[3][0]*b[1][3];
    temp[1][1] = a[0][1]*b[1][0] + a[1][1]*b[1][1] + a[2][1]*b[1][2] + a[3][1]*b[1][3];
    temp[1][2] = a[0][2]*b[1][0] + a[1][2]*b[1][1] + a[2][2]*b[1][2] + a[3][2]*b[1][3];
    temp[1][3] = a[0][3]*b[1][0] + a[1][3]*b[1][1] + a[2][3]*b[1][2] + a[3][3]*b[1][3];

    temp[2][0] = a[0][0]*b[2][0] + a[1][0]*b[2][1] + a[2][0]*b[2][2] + a[3][0]*b[2][3];
    temp[2][1] = a[0][1]*b[2][0] + a[1][1]*b[2][1] + a[2][1]*b[2][2] + a[3][1]*b[2][3];
    temp[2][2] = a[0][2]*b[2][0] + a[1][2]*b[2][1] + a[2][2]*b[2][2] + a[3][2]*b[2][3];
    temp[2][3] = a[0][3]*b[2][0] + a[1][3]*b[2][1] + a[2][3]*b[2][2] + a[3][3]*b[2][3];

    temp[3][0] = a[0][0]*b[3][0] + a[1][0]*b[3][1] + a[2][0]*b[3][2] + a[3][0]*b[3][3];
    temp[3][1] = a[0][1]*b[3][0] + a[1][1]*b[3][1] + a[2][1]*b[3][2] + a[3][1]*b[3][3];
    temp[3][2] = a[0][2]*b[3][0] + a[1][2]*b[3][1] + a[2][2]*b[3][2] + a[3][2]*b[3][3];
    temp[3][3] = a[0][3]*b[3][0] + a[1][3]*b[3][1] + a[2][3]*b[3][2] + a[3][3]*b[3][3];

    memcpy(result, temp, sizeof(mat4x4));
}

/**
 * @brief compute the inverse of a 4x4 matrix.
 * @param result the matrix that will receive the inverse of mat.
 * @param mat the matrix to invert.
 */
BVR_H_FUNC void mat4_inv(mat4x4 result, mat4x4 const mat){
    float s[6];
	float c[6];
	s[0] = mat[0][0]*mat[1][1] - mat[1][0]*mat[0][1];
	s[1] = mat[0][0]*mat[1][2] - mat[1][0]*mat[0][2];
	s[2] = mat[0][0]*mat[1][3] - mat[1][0]*mat[0][3];
	s[3] = mat[0][1]*mat[1][2] - mat[1][1]*mat[0][2];
	s[4] = mat[0][1]*mat[1][3] - mat[1][1]*mat[0][3];
	s[5] = mat[0][2]*mat[1][3] - mat[1][2]*mat[0][3];

	c[0] = mat[2][0]*mat[3][1] - mat[3][0]*mat[2][1];
	c[1] = mat[2][0]*mat[3][2] - mat[3][0]*mat[2][2];
	c[2] = mat[2][0]*mat[3][3] - mat[3][0]*mat[2][3];
	c[3] = mat[2][1]*mat[3][2] - mat[3][1]*mat[2][2];
	c[4] = mat[2][1]*mat[3][3] - mat[3][1]*mat[2][3];
	c[5] = mat[2][2]*mat[3][3] - mat[3][2]*mat[2][3];
	
	float idet = 1.0f/( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );
	
	result[0][0] = ( mat[1][1] * c[5] - mat[1][2] * c[4] + mat[1][3] * c[3]) * idet;
	result[0][1] = (-mat[0][1] * c[5] + mat[0][2] * c[4] - mat[0][3] * c[3]) * idet;
	result[0][2] = ( mat[3][1] * s[5] - mat[3][2] * s[4] + mat[3][3] * s[3]) * idet;
	result[0][3] = (-mat[2][1] * s[5] + mat[2][2] * s[4] - mat[2][3] * s[3]) * idet;

	result[1][0] = (-mat[1][0] * c[5] + mat[1][2] * c[2] - mat[1][3] * c[1]) * idet;
	result[1][1] = ( mat[0][0] * c[5] - mat[0][2] * c[2] + mat[0][3] * c[1]) * idet;
	result[1][2] = (-mat[3][0] * s[5] + mat[3][2] * s[2] - mat[3][3] * s[1]) * idet;
	result[1][3] = ( mat[2][0] * s[5] - mat[2][2] * s[2] + mat[2][3] * s[1]) * idet;

	result[2][0] = ( mat[1][0] * c[4] - mat[1][1] * c[2] + mat[1][3] * c[0]) * idet;
	result[2][1] = (-mat[0][0] * c[4] + mat[0][1] * c[2] - mat[0][3] * c[0]) * idet;
	result[2][2] = ( mat[3][0] * s[4] - mat[3][1] * s[2] + mat[3][3] * s[0]) * idet;
	result[2][3] = (-mat[2][0] * s[4] + mat[2][1] * s[2] - mat[2][3] * s[0]) * idet;

	result[3][0] = (-mat[1][0] * c[3] + mat[1][1] * c[1] - mat[1][2] * c[0]) * idet;
	result[3][1] = ( mat[0][0] * c[3] - mat[0][1] * c[1] + mat[0][2] * c[0]) * idet;
	result[3][2] = (-mat[3][0] * s[3] + mat[3][1] * s[1] - mat[3][2] * s[0]) * idet;
	result[3][3] = ( mat[2][0] * s[3] - mat[2][1] * s[1] + mat[2][2] * s[0]) * idet;
}

/**
 * @brief build a rotation matrix from a quaternion.
 * @param mat the matrix that will receive the rotation matrix.
 * @param quat the quaternion to convert.
 */
BVR_H_FUNC void mat4_from_quat(mat4x4 mat, quat const quat)
{
	float a = quat[3];
	float b = quat[0];
	float c = quat[1];
	float d = quat[2];
	float a2 = a*a;
	float b2 = b*b;
	float c2 = c*c;
	float d2 = d*d;
	
	mat[0][0] = a2 + b2 - c2 - d2;
	mat[0][1] = 2.f*(b*c + a*d);
	mat[0][2] = 2.f*(b*d - a*c);
	mat[0][3] = 0.f;

	mat[1][0] = 2*(b*c - a*d);
	mat[1][1] = a2 - b2 + c2 - d2;
	mat[1][2] = 2.f*(c*d + a*b);
	mat[1][3] = 0.f;

	mat[2][0] = 2.f*(b*d + a*c);
	mat[2][1] = 2.f*(c*d - a*b);
	mat[2][2] = a2 - b2 - c2 + d2;
	mat[2][3] = 0.f;

	mat[3][0] = mat[3][1] = mat[3][2] = 0.f;
	mat[3][3] = 1.f;
}

/**
 * @brief build a rotation matrix from Euler angles, expressed in degrees.
 * @param dest the matrix that will receive the rotation matrix.
 * @param angles the Euler angles (x, y, z) in degrees.
 */
BVR_H_FUNC void mat4_rotate(mat4x4 dest, vec3 angles)
{
    float cx, cy, cz, sx, sy, sz, czsx, cxcz, sysz;
    sx = sinf(deg_to_rad(angles[0]));
    cx = cosf(deg_to_rad(angles[0]));
    sy = sinf(deg_to_rad(angles[1]));
    cy = cosf(deg_to_rad(angles[1]));
    sz = sinf(deg_to_rad(angles[2]));
    cz = cosf(deg_to_rad(angles[2]));

    czsx = cz * sx;
    cxcz = cx * cz;
    sysz = sy * sz;

    dest[0][0] = cy * cz;
    dest[0][1] = czsx * sy + cx * sz;
    dest[0][2] = -cxcz * sy + sx * sz;
    dest[1][0] = -cy * sz;
    dest[1][1] = cxcz - sx * sysz;
    dest[1][2] = czsx + cx * sysz;
    dest[2][0] = sy;
    dest[2][1] = -cy * sx;
    dest[2][2] = cx * cy;
    dest[0][3] = 0.0f;
    dest[1][3] = 0.0f;
    dest[2][3] = 0.0f;
    dest[3][0] = 0.0f;
    dest[3][1] = 0.0f;
    dest[3][2] = 0.0f;
    dest[3][3] = 1.0f;
}

/**
 * @brief build a quaternion representing a rotation of a given angle around an axis.
 * @param quat the quaternion that will receive the result.
 * @param angle the rotation angle, in radians.
 * @param axis the rotation axis (will be normalized internally).
 */
BVR_H_FUNC void quat_rotate(quat quat, float angle, vec3 const axis){
    vec3 axis_normalized;
    vec3_norm(axis_normalized, axis);
    float s = sinf(angle / 2.0f);
    float c = sinf(angle / 2.0f);
    vec3_scale(quat, axis_normalized, s);
    quat[3] = c;
}

/**
 * @brief build a quaternion from roll, pitch and yaw Euler angles.
 * @param quat the quaternion that will receive the result.
 * @param roll the roll angle, in radians.
 * @param pitch the pitch angle, in radians.
 * @param yaw the yaw angle, in radians.
 */
BVR_H_FUNC void quat_euler(quat quat, float roll, float pitch, float yaw){
    float cr = cos(roll * 0.5);
    float sr = sin(roll * 0.5);
    float cp = cos(pitch * 0.5);
    float sp = sin(pitch * 0.5);
    float cy = cos(yaw * 0.5);
    float sy = sin(yaw * 0.5);

    quat[3] = cr * cp * cy + sr * sp * sy;
    quat[0] = sr * cp * cy - cr * sp * sy;
    quat[1] = cr * sp * cy + sr * cp * sy;
    quat[2] = cr * cp * sy - sr * sp * cy;
}

/**
 * @brief convert a quaternion back into roll, pitch and yaw Euler angles.
 * @param euler the vector that will receive the resulting Euler angles (roll, pitch, yaw), in radians.
 * @param quat the quaternion to convert.
 */
BVR_H_FUNC void euler_quat(vec3 euler, quat quat){
    // roll
    float sinr = 2 * (quat[3] * quat[0] + quat[1] * quat[2]);
    float cosr = 1 - 2 * (quat[0] * quat[0] + quat[1] * quat[1]);

    // pitch
    float sinp = sqrtf(1 + 2 * (quat[3] * quat[1] - quat[0] * quat[2]));
    float cosp = sqrtf(1 - 2 * (quat[3] * quat[1] - quat[0] * quat[2]));

    // yaw
    float siny = 2 * (quat[3] * quat[2] + quat[0] * quat[1]);
    float cosy = 1 - 2 * (quat[1] * quat[1] + quat[2] * quat[2]);

    euler[0] = atan2f(sinr, cosr);
    euler[1] = 2 * atan2f(sinp, cosp) - BVR_PI / 2.0f;
    euler[2] = atan2f(siny, cosy);
}