#version 400

#ifdef _VERTEX_

layout(location=0) in vec3 in_position;
layout(location=1) in vec2 in_uvs;
layout(location=1) in vec3 in_norm;

uniform mat4 bvr_transform;
uniform mat4 bvr_local;

layout(std140) uniform bvr_camera {
	mat4 bvr_projection;
	mat4 bvr_view;
};

layout(std140) uniform bvr_global_illumination {
	vec4 bvr_light_position;
	vec4 bvr_light_direction;
	vec4 bvr_light_color;
};

out V_DATA vertex;
out V_LIGHT light;

void main() {
	gl_Position = bvr_projection * bvr_view * bvr_transform * bvr_local * vec4(in_position, 1.0);
	
	vertex.position = vec3(bvr_transform * vec4(in_position, 1.0));
	vertex.uvs = in_uvs;
    vertex.normals = mat3(transpose(inverse(bvr_transform * bvr_local))) * in_norm;

	light.position = bvr_light_position;
	light.direction = bvr_light_direction;
	light.color = bvr_light_color;
}

#endif

#ifdef _FRAGMENT_

in V_DATA vertex;
in V_LIGHT light;

uniform sampler2D bvr_texture;

void main() {
	vec4 tex = vec4(1.0, 1.0, 1.0, 1.0); // texture(bvr_texture, vec2(vertex.uvs));

	gl_FragColor = tex * calc_light(tex, light, vertex);
}

#endif