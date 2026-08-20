#pragma once

// vertex shader struct
static const char* __ext_s_vdata = "struct V_DATA {\n"
	"   vec3 position;\n"
	"   vec2 uvs;\n"
    "   vec3 normals;\n"
    "};\n";

// light shader struct
static const char* __ext_s_vlight = "struct V_LIGHT {\n"
"	vec4 position;\n"
"	vec4 direction;\n"
"	vec4 color;\n"
"};\n";

static const char* __ext_s_layer = "struct L_DATA {\n"
"	int index;\n"
"	int blend;\n"
"	float opacity;\n"
"};\n";

// light related function(s)
static const char* __ext_f_light = "vec4 calc_light(vec4 color, V_LIGHT light, V_DATA vertex){\n"
"	vec3 l_color;\n"
"	float intensity = light.color.a / 255;\n"
"	float ambiant_intensity = light.position.w / 255;\n"
"	vec3 norm = normalize(vertex.normals);\n"
"	vec3 light_direction = normalize(light.position.xyz - vertex.position);\n"
"	vec3 diffuse = vec3(intensity) * max(dot(norm, light_direction), 0.0);\n"
"	vec3 ambiant = vec3(ambiant_intensity);\n"
"	l_color = diffuse + ambiant;\n"
"	return vec4(l_color, 1.0) * vec4(light.color.rgb, 1.0);\n"
"}\n";

static const char* __ext_f_layer = "L_DATA create_layer(int layer){\n"
"	L_DATA info;\n"
"   info.index = 0xFF & layer;\n"
"   info.blend = (0xFF00 & layer) >> 8;\n"
"   info.opacity = ((0xFF0000 & layer) >> 16) / 255.0;\n"
"	return info;\n"
"}\n"
"vec4 calc_blending(vec4 composite, vec4 pixel, L_DATA layer){\n"
"    vec3 blend = pixel.rgb;\n"
"    float alpha = pixel.a * layer.opacity;\n"
    // normal and passthrough
"    if(layer.blend == 0 || layer.blend == 1){ return mix(composite, pixel, alpha);}"
    // multiply
"    if(layer.blend == 4){blend = composite.rgb * pixel.rgb;}\n"
    // screen
"    else if(layer.blend == 9){ blend = 1.0 - (1.0 - composite.rgb) * (1.0 - pixel.rgb);}\n"
    // overlay
"    else if(layer.blend == 13){\n"
"        blend = mix(2.0 * composite.rgb * pixel.rgb, \n"
"       1.0 - 2.0*(1.0-composite.rgb)*(1.0-pixel.rgb),\n"
"            step(0.5, composite.rgb));\n"
"    }\n"
    // darken
"    else if(layer.blend == 3){blend = min(composite.rgb, pixel.rgb);}\n"
    // lighten
"    else if(layer.blend == 8){blend = max(composite.rgb, pixel.rgb);}\n"
"    return mix(composite, vec4(blend, 1.0), alpha);\n"
"}\n";