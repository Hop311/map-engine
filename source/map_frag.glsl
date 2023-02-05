
const char *const SHADER_FRAG = R"(

#version 330 core

in vec2 uv_frag;

out vec4 colour_out;

uniform sampler2D colormap_tex, terrain_tex, texturesheet_tex;
uniform vec2 map_dims;

const float block_size = 8.0f;
const float sheet_size = 8.0f;

vec2 map_size = vec2(5616, 2160);
float xx = 1 / map_size.x;
float yy = 1 / map_size.y;
vec2 pix = vec2(xx, yy);

vec4 get_terrain(vec2 uv_frag, vec2 corner, vec2 block_offset) {
	uv_frag += corner * pix * 0.5;
	float terrain_type = floor(texture(terrain_tex, uv_frag).r * 256.0f);
	vec2 block_pos = vec2(mod(terrain_type, sheet_size), floor(terrain_type / sheet_size)) / sheet_size;
	vec4 terrain_col = texture(texturesheet_tex, block_pos + block_offset);
	return terrain_col = mix(terrain_col, vec4(0.0f, 0.0f, 0.0f, 1.0f), step(64.0f, terrain_type));
}

vec4 get_terrain_mix(vec2 tex_coord) {
	// Pixel size on map texture
	vec2 scaling = mod(tex_coord + 0.5 * pix, pix) / pix;

	vec2 offset = mod(uv_frag, block_size / map_dims) * map_dims / (block_size * sheet_size);

	vec4 colourlu = get_terrain(tex_coord, vec2(-1, -1), offset);
	vec4 colourld = get_terrain(tex_coord, vec2(-1, +1), offset);
	vec4 colourru = get_terrain(tex_coord, vec2(+1, -1), offset);
	vec4 colourrd = get_terrain(tex_coord, vec2(+1, +1), offset);

	vec4 colour_u = mix(colourlu, colourru, scaling.x);
	vec4 colour_d = mix(colourld, colourrd, scaling.x);
	return mix(colour_u, colour_d, scaling.y);
}

void main() {
	vec4 terrain_col = get_terrain_mix(uv_frag);
	colour_out = 0.5f * (terrain_col + texture(colormap_tex, uv_frag));
}

)";
