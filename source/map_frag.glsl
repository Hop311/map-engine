
const char *const SHADER_FRAG = R"(

#version 330 core

in vec2 uv_frag;

out vec4 colour_out;

uniform sampler2D colormap_tex, terrain_tex, texturesheet_tex;
uniform vec2 map_dims;

const float block_size = 16.0f;
const float sheet_size = 8.0f;

void main() {
	float terrain_type = floor(texture(terrain_tex, vec2(uv_frag.x, 1.0f - uv_frag.y)).r * 256.0f);
	vec2 block_pos = vec2(mod(terrain_type, sheet_size), floor(terrain_type / sheet_size)) / sheet_size;

	vec2 block_dims = block_size / map_dims;
	vec2 offset = mod(uv_frag , block_dims) / block_dims / sheet_size;

	vec4 terrain_col = texture(texturesheet_tex, block_pos + offset);
	terrain_col = mix(terrain_col, vec4(0.0f, 0.0f, 0.0f, 1.0f), step(64.0f, terrain_type));

	colour_out = 0.5f * (terrain_col + texture(colormap_tex, uv_frag));
}

)";