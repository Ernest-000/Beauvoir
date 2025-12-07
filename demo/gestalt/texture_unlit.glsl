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
	gl_Position = bvr_transform * vec4(in_position, 1.0);
	
	vertex.position = vec3(bvr_transform[3].xyz) / 2.0;
	vertex.uvs = in_uvs;
}

#endif

#ifdef _FRAGMENT_

precision mediump float;

in V_DATA vertex;

uniform sampler2D bvr_composite;
uniform sampler2DArray bvr_texture;

uniform int bvr_layer;

void main() {
	L_DATA layer = create_layer(bvr_layer);

	vec4 composite = texture(bvr_composite, vertex.uvs + vertex.position.xy);
	vec4 t_sample = texture(bvr_texture, vec3(vertex.uvs, layer.index));

	gl_FragColor = calc_blending(composite, t_sample, layer);
}

#endif