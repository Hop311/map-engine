
const char *const SHADER_FRAG = R"(

#version 330 core

in vec2 uv_frag;

out vec4 colour_out;

uniform sampler2D provinces_tex, terrain_tex;

void main() {
	colour_out = 0.5f * (texture(provinces_tex, uv_frag) + texture(terrain_tex, uv_frag));
}

)";
