#version 400

#ifdef _VERTEX_

layout(location=0) in int i_vertex;

uniform mat4 bvr_transform;
uniform vec4 bvr_grid_size;

layout(std140) uniform bvr_camera {
	mat4 bvr_projection;
	mat4 bvr_view;
};

out L_DATA {
	vec2 uvs;
	flat int vertex;
} vertex;

void main() {	
	vec3 position = vec3(0.0);

	float vertices_per_row = bvr_grid_size.x * 2.0 + 3.0;

	float clamped_layer = mod(gl_VertexID, vertices_per_row * bvr_grid_size.y);
	float row_index = mod(clamped_layer, vertices_per_row);
	float clamped_index = clamp(row_index - 1.0, 0.0, vertices_per_row - 2);

	position.x = floor(clamped_index / 2);
	position.z = mod(clamped_index, 2);

	// add rows
	position.z += floor(clamped_layer / (vertices_per_row));

	vertex.uvs = position.xz * vec2(1.0, -1.0);
	vertex.vertex = i_vertex;

	// size the grid to tile's dimensions
	position.x *= bvr_grid_size.z;
	position.z *= bvr_grid_size.w;

	position.y = i_vertex & 0xFF;

	gl_Position = bvr_projection * bvr_view * bvr_transform * vec4(position, 1.0);
}

#endif

#ifdef _FRAGMENT_

in L_DATA {
	vec2 uvs;
	flat int vertex;
} vertex;

uniform sampler2DArray bvr_texture;

void main() {
	int texid = (0xFF00 & vertex.vertex) >> 8;
	if(texid == 0) {
		discard;
	}

	vec4 tex = texture(bvr_texture, vec3(vertex.uvs, texid));
	gl_FragColor = vec4(tex.rgb, 1.0);
}

#endif