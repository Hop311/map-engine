
const char *const SHADER_VERT = R"(

#version 330 core

layout(location = 0) in vec2 uv_in;

out vec2 uv_frag;

uniform mat4 model, view, proj;

void main() {
	gl_Position = proj * view * model * vec4(uv_in.x, 0.0f, uv_in.y, 1.0f);
	uv_frag = uv_in;
}

)";
