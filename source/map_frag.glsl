
const char *const SHADER_FRAG = R"(

#version 330 core

in vec2 uv_frag;

out vec4 colour_out;

uniform sampler2D colormap_tex, terrain_tex, texturesheet_tex;
uniform vec2 terrain_dims;

const float block_size = 8.0f;
const float sheet_size = 8.0f;
const vec4 water_component = vec4(0.0f, 0.0f, 0.0f, 1.0f);

vec2 pixel_dims = 1.0f / terrain_dims;
vec2 half_pixel_dims = 0.5f * pixel_dims;
vec2 block_offset = mod(uv_frag, block_size / terrain_dims) * terrain_dims / (block_size * sheet_size);

vec4 get_terrain(vec2 corner) {
	float terrain_type = floor(texture(terrain_tex, uv_frag + half_pixel_dims * corner).r * 256.0f);
	vec2 block_pos = vec2(mod(terrain_type, sheet_size), floor(terrain_type / sheet_size)) / sheet_size;
	vec4 terrain_col = texture(texturesheet_tex, block_pos + block_offset);
	return mix(terrain_col, water_component, step(sheet_size * sheet_size, terrain_type));
}

void main(void) {
	vec2 pixel_offset = mod(uv_frag + half_pixel_dims, pixel_dims) * terrain_dims;
	vec4 terrain_col = mix(
		mix(get_terrain(vec2(-1, -1)), get_terrain(vec2(+1, -1)), pixel_offset.x),
		mix(get_terrain(vec2(-1, +1)), get_terrain(vec2(+1, +1)), pixel_offset.x),
		pixel_offset.y);
	colour_out = 0.5f * (terrain_col + texture(colormap_tex, uv_frag));
}

)";
