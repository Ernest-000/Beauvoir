#version 400

#ifdef _VERTEX_

layout(location=0) in int i_vertex;

uniform mat4 bvr_transform;
uniform vec4 bvr_grid_size;

layout(std140) uniform bvr_camera {
	mat4 bvr_projection;
	mat4 bvr_view;
};

out V_DATA {
	vec2 uvs;
} vertex;

void main() {	
	vec2 position = vec2(0.0);

	position.x = mod(gl_VertexID / 2, bvr_grid_size.x);
	position.y = mod(gl_VertexID, 2);

	// add rows
	position.y += floor(gl_VertexID / (bvr_grid_size.x * 2));

	vertex.uvs = position;

	// size the grid to tile's dimensions
	position *= bvr_grid_size.zw;

	gl_Position = bvr_projection * bvr_view * bvr_transform * vec4(position.x, 0, position.y, 1.0);
}

#endif

#ifdef _FRAGMENT_

in V_DATA {
	vec2 uvs;
} vertex;

uniform sampler2DArray bvr_texture;

void main() {
	vec4 tex = texture(bvr_texture, vec3(vertex.uvs, 12));
	gl_FragColor = tex;
}

#endif