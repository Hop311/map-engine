
const char *const SHADER_VERT = R"(

#version 330 core

layout(location = 0) in vec2 uv_in;

out vec2 uv_frag;

uniform mat4 model, view, proj;
uniform sampler2D terrain_tex;

float heights[64] = float[64](
	0.11f, 0.11f, 0.11f, 0.11f, 0.11f, // arctic forest
	0.1f, 0.1f, 0.1f,                  // dry plains
	0.12f, 0.12f, 0.12f, 0.12f,        // farmland
	0.15f, 0.15f, 0.15f, 0.15f,        // tall forest
	0.15f, 0.15f, 0.15f, 0.15f,        // small hills
	0.13f, 0.13f, 0.13f, 0.13f,        // round forest
	0.16f, 0.16f, 0.16f, 0.16f,        // small mountains
	0.18f, 0.18f, 0.18f, 0.18f,        // tall mountains
	0.12f, 0.12f, 0.12f, 0.12f,        // plains
	0.1f, 0.1f, 0.1f, 0.1f,            // steppe
	0.13f, 0.13f, 0.13f, 0.13f,        // jungle
	0.11f, 0.11f, 0.11f, 0.11f,        // marsh
	0.11f, 0.11f, 0.11f, 0.11f,        // arid
	0.1f, 0.1f, 0.1f, 0.1f,            // desert
	0.18f, 0.18f, 0.18f, 0.18f,        // mountain
	0.2f, 0.2f, 0.2f, 0.2f             // mountain peak
);

void main() {
	float terrain_type = floor(texture(terrain_tex, uv_in).r * 256.0f);
	if (terrain_type < 64.0f) terrain_type = heights[int(terrain_type + 0.5f)];
	else terrain_type = 0.0f;
	gl_Position = proj * view * model * vec4(uv_in.x, terrain_type - 0.05f, uv_in.y, 1.0f);
	uv_frag = uv_in;
}

)";
