#version 310 es

#undef lowp
#undef mediump
#undef highp

#ifdef _VERTEX_

precision mediump float;

layout(location=0) in vec3 in_position;
layout(location=1) in vec2 in_uvs;

uniform mat4 bvr_transform;

layout(std140) uniform bvr_camera {
	mat4 bvr_projection;
	mat4 bvr_view;
};

out V_DATA vertex;

void main() {
	gl_Position = bvr_projection * bvr_view * bvr_transform * vec4(in_position, 1.0);
	
	vertex.uvs = in_uvs;
}

#endif

#ifdef _FRAGMENT_

precision mediump float;

uniform mediump vec3 bcolor;

in V_DATA vertex;

layout(location=0) out vec4 o_Color;

void main() {
	o_Color = vec4(1.0, 1.0, 1.0, 1.0);
}

#endif