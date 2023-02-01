#include "Graphics.hpp"

#include "Logger.hpp"
#include "GLTools.hpp"
#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <SOIL2.h>

const glm::vec3 RED{ 1.0f, 0.0f, 0.0f };
const glm::vec3 GREEN{ 0.0f, 1.0f, 0.0f };
const glm::vec3 BLUE{ 0.0f, 0.0f, 1.0f };
const glm::vec3 YELLOW{ 1.0f, 1.0f, 0.0f };
const glm::vec3 CYAN{ 0.0f, 1.0f, 1.0f };
const glm::vec3 MAGENTA{ 1.0f, 0.0f, 1.0f };
const glm::vec3 BLACK{};
const glm::vec3 WHITE{ 1.0f };
const glm::vec3 GREY{ 0.5f };

struct vertex_t {
	glm::vec3 pos;
	glm::vec2 colour;
};

const float MAP_SIZE = 5.0f, MAP_HEIGHT = -2.5f;
const vertex_t MAP[4] = {
	{ { -MAP_SIZE, MAP_HEIGHT, -MAP_SIZE }, { 0.0f, 1.0f } },
	{ { -MAP_SIZE, MAP_HEIGHT,  MAP_SIZE }, { 0.0f, 0.0f } },
	{ {  MAP_SIZE, MAP_HEIGHT, -MAP_SIZE }, { 1.0f, 1.0f } },
	{ {  MAP_SIZE, MAP_HEIGHT,  MAP_SIZE }, { 1.0f, 0.0f } }
};
const vertex_t VERT_DATA[] = {
	MAP[0], MAP[1], MAP[2], MAP[3]
};
const char *const SHADER_VERT = R"(
#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec2 uv_in;

out vec2 uv_frag;

uniform mat4 view;
uniform mat4 proj;

void main() {
	gl_Position = proj * view * vec4(position_in, 1.0f);
	uv_frag = uv_in;
}
)";
const char *const SHADER_FRAG = R"(
#version 330 core

in vec2 uv_frag;

out vec4 colour_out;

uniform sampler2D tex;

void main() {
	colour_out = texture(tex, uv_frag);
}
)";

#define MAP_DIR R"(C:\Users\hop31\Documents\Workspace\map-engine\map\)"

static GLuint program = 0, tex = 0, vao = 0, vbo = 0;
static GLint view_uniform = 0, proj_uniform = 0, tex_uniform = 0;
static glm::mat4 proj;

bool Graphics::init() {
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		logger("Failed to initialize GLEW.");
		return false;
	}

	enable_gl_debug_output();

	glClearColor(0.0f, 0.5f, 1.0f, 1.0f);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Load shaders
	int ret = load_program(program, SHADER_VERT, nullptr, SHADER_FRAG);
	if (ret) {
		logger("Failed to load shaders.");
		return false;
	}
	view_uniform = glGetUniformLocation(program, "view");
	proj_uniform = glGetUniformLocation(program, "proj");
	tex_uniform = glGetUniformLocation(program, "tex");

	// Load images
	tex = SOIL_load_OGL_texture(MAP_DIR "provinces.bmp",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	if (!tex) {
		logger("Failed to load province texture.");
		glDeleteProgram(program);
		return false;
	}
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Generate tris buffer and vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)sizeof(glm::vec3));
	glEnableVertexAttribArray(1);

	glUseProgram(program);
	glUniform1i(tex_uniform, 0);

	glBufferData(GL_ARRAY_BUFFER, sizeof(VERT_DATA), VERT_DATA, GL_STATIC_DRAW);

	logger("Successfully initialised graphics.");
	return true;
}

void Graphics::deinit() {
	glDeleteProgram(program);
	glDeleteTextures(1, &tex);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	logger("Successfully deinitialised graphics.");
}

void Graphics::render(const Camera *camera) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniformMatrix4fv(proj_uniform, 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(view_uniform, 1, GL_FALSE, &camera->getMatrix()[0][0]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(VERT_DATA) / sizeof(vertex_t));
}

void Graphics::resize(glm::ivec2 dims) {
	glViewport(0, 0, dims.x, dims.y);
	proj = glm::perspective(glm::radians(70.0f), (float)dims.x / (float)dims.y, 0.1f, 100.0f);
}
