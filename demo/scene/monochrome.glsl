#version 400

#ifdef _VERTEX_

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

in V_DATA vertex;

uniform vec3 bvr_color;

void main() {
	gl_FragColor = vec4(bvr_color * gl_FragCoord.z, 1.0);
}

#endif