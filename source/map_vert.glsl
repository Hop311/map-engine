
const char *const SHADER_VERT = R"(

#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec2 uv_in;

out vec2 uv_frag;

uniform mat4 model, view, proj;

void main() {
	gl_Position = proj * view * model * vec4(position_in, 1.0f);
	uv_frag = uv_in;
}

)";
