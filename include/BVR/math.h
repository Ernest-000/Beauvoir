#pragma once

#include <BVR/config.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>

typedef float vec2[2];
typedef float vec3[3];
typedef float vec4[4];
typedef float quat[4];

typedef vec4 mat4x4[4];

typedef struct bvr_transform_s {
    vec3 position;
    vec3 scale;
    quat rotation;

    mat4x4 matrix;
} bvr_transform_t;

struct bvr_bounds_s {
    vec2 coords;
    float width;
    float height;
};

#define BVR_SCALE_VEC3(vec, a) vec[0] = a; vec[1] = a; vec[2] = a;
#define BVR_SCALE_VEC4(vec, a) vec[0] = a; vec[1] = a; vec[2] = a; vec[3] = a;

#define BVR_IDENTITY_VEC3(vec) BVR_SCALE_VEC3(vec, 0.0f);
#define BVR_IDENTITY_VEC4(vec) BVR_SCALE_VEC4(vec, 0.0f);
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
    BVR_SCALE_VEC3(tr.scale, scalev)                         

BVR_H_FUNC float flerp(float a, float b, float t){
    return a + t * (b - a);
}

BVR_H_FUNC float rad_to_deg(float rad) { 
    return rad * 180 / M_PI; 
}

BVR_H_FUNC float deg_to_rad(float deg) { 
    return deg * M_PI / 180; 
}

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

BVR_H_FUNC float clamp(float d, float min, float max) {
  const float t = d < min ? min : d;
  return t > max ? max : t;
}

BVR_H_FUNC void vec2_add(vec2 result, vec2 const a, vec2 const b){
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
}

BVR_H_FUNC void vec2_sub(vec2 result, vec2 const a, vec2 const b){
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
}

BVR_H_FUNC void vec2_scale(vec2 result, vec2 const a, float const s){
    result[0] = a[0] * s;
    result[1] = a[1] * s;
}

BVR_H_FUNC float vec2_dot(vec2 const a, vec2 const b){
    return a[0] * b[0] + a[1] * b[1];
}

BVR_H_FUNC float vec2_cross(vec2 const a, vec2 const b){
    return a[0] * b[1] - a[1] * b[0];
}

BVR_H_FUNC float vec2_len(vec2 const v){
    return sqrtf(vec2_dot(v, v));
}

BVR_H_FUNC void vec2_norm(vec2 result, vec2 const v){
    vec2_scale(result, v, 1.0f / vec2_len(v));
}

BVR_H_FUNC void vec2_copy(vec2 result, vec2 const a){
    result[0] = a[0];
    result[1] = a[1];
}

BVR_H_FUNC void vec3_add(vec3 result, vec3 const a, vec3 const b){
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
}

BVR_H_FUNC void vec3_sub(vec3 result, vec3 const a, vec3 const b){
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
}

BVR_H_FUNC void vec3_scale(vec3 result, vec3 const a, float const s){
    result[0] = a[0] * s;
    result[1] = a[1] * s;
    result[2] = a[2] * s;
}

BVR_H_FUNC float vec3_dot(vec3 const a, vec3 const b){
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

BVR_H_FUNC float vec3_len(vec3 const v){
    return sqrtf(vec3_dot(v, v));
}

BVR_H_FUNC void vec3_norm(vec3 result, vec3 const v){
    vec3_scale(result, v, 1.0f / vec3_len(v));
}

BVR_H_FUNC void vec3_copy(vec3 result, vec3 const a){
    result[0] = a[0];
    result[1] = a[1];
    result[2] = a[2];
}                             

BVR_H_FUNC void vec3_mul_cross(vec3 result, vec3 const a, vec3 const b){
    result[0] = a[1]*b[2] - a[2]*b[1];
	result[1] = a[2]*b[0] - a[0]*b[2];
	result[2] = a[0]*b[1] - a[1]*b[0];
}

BVR_H_FUNC void vec4_add(vec4 result, vec4 const a, vec4 const b){
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
    result[3] = a[3] + b[3];
}

BVR_H_FUNC void vec4_sub(vec4 result, vec4 const a, vec4 const b){
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
    result[3] = a[3] - b[3];
}

BVR_H_FUNC void vec4_scale(vec4 result, vec4 const a, float const s){
    result[0] = a[0] * s;
    result[1] = a[1] * s;
    result[2] = a[2] * s;
    result[3] = a[3] * s;
}

BVR_H_FUNC float vec4_dot(vec4 const a, vec4 const b){
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

BVR_H_FUNC float vec4_len(vec4 const v){
    return sqrtf(vec4_dot(v, v));
}

BVR_H_FUNC void vec4_norm(vec4 result, vec4 const v){
    vec4_scale(result, v, 1.0f / vec4_len(v));
}

BVR_H_FUNC void vec4_copy(vec4 result, vec4 const a){
    result[0] = a[0];
    result[1] = a[1];
    result[2] = a[2];
    result[3] = a[3];
}                             

BVR_H_FUNC void vec4_mul_cross(vec4 result, vec4 const a, vec4 const b){
    result[0] = a[1]*b[2] - a[2]*b[1];
	result[1] = a[2]*b[0] - a[0]*b[2];
	result[2] = a[0]*b[1] - a[1]*b[0];
    result[3] = 1.0f;
}

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

BVR_H_FUNC void mat4_mul_vec4(vec4 result, mat4x4 const mat, vec4 const vec){
    result[0] = (mat[0][0] + mat[0][1] + mat[0][2] + mat[0][3]) * vec[0];
    result[1] = (mat[1][0] + mat[1][1] + mat[1][2] + mat[1][3]) * vec[1];
    result[2] = (mat[2][0] + mat[2][1] + mat[2][2] + mat[2][3]) * vec[2];
    result[3] = (mat[3][0] + mat[3][1] + mat[3][2] + mat[3][3]) * vec[3];
}

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

BVR_H_FUNC void quat_rotate(quat quat, float angle, vec3 const axis){
    vec3 axis_normalized;
    vec3_norm(axis_normalized, axis);
    float s = sinf(angle / 2.0f);
    float c = sinf(angle / 2.0f);
    vec3_scale(quat, axis_normalized, s);
    quat[3] = c;
}

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
    euler[1] = 2 * atan2f(sinp, cosp) - M_PI / 2.0f;
    euler[2] = atan2f(siny, cosy);
}