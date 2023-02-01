#include "Graphics.hpp"

#include "Logger.hpp"
#include "GLTools.hpp"
#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

const glm::vec3 RED{ 1.0f, 0.0f, 0.0f };
const glm::vec3 GREEN{ 0.0f, 1.0f, 0.0f };
const glm::vec3 BLUE{ 0.0f, 0.0f, 1.0f };
const glm::vec3 YELLOW{ 1.0f, 1.0f, 0.0f };
const glm::vec3 CYAN{ 0.0f, 1.0f, 1.0f };
const glm::vec3 MAGENTA{ 1.0f, 0.0f, 1.0f };
const glm::vec3 BLACK{};
const glm::vec3 WHITE{ 1.0f };
const glm::vec3 GREY{ 0.5f };

const float FLOOR_SIZE = 5.0f, FLOOR_HEIGHT = -2.5f;
const float CUBE_SIZE = 1.0f;

struct vertex_t {
	glm::vec3 pos, colour;
};

const vertex_t FLOOR[4] = {
	{ { -FLOOR_SIZE, FLOOR_HEIGHT, -FLOOR_SIZE }, GREY },
	{ { -FLOOR_SIZE, FLOOR_HEIGHT,  FLOOR_SIZE }, GREY },
	{ {  FLOOR_SIZE, FLOOR_HEIGHT,  FLOOR_SIZE }, GREY },
	{ {  FLOOR_SIZE, FLOOR_HEIGHT, -FLOOR_SIZE }, GREY }
};
const vertex_t CUBE[8] = {
	{ { -CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE }, RED },
	{ { -CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE }, GREEN },
	{ {  CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE }, BLUE },
	{ {  CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE }, YELLOW },
	{ { -CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE }, CYAN },
	{ { -CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE }, MAGENTA },
	{ {  CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE }, BLACK },
	{ {  CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE }, WHITE },
};

const vertex_t VERT_DATA[] = {
	FLOOR[0], FLOOR[1], FLOOR[2], FLOOR[0], FLOOR[2], FLOOR[3],
	CUBE[0], CUBE[1], CUBE[2], CUBE[0], CUBE[2], CUBE[3],
	CUBE[4], CUBE[6], CUBE[5], CUBE[4], CUBE[7], CUBE[6],
	CUBE[0], CUBE[4], CUBE[5], CUBE[0], CUBE[5], CUBE[1],
	CUBE[1], CUBE[5], CUBE[6], CUBE[1], CUBE[6], CUBE[2],
	CUBE[2], CUBE[6], CUBE[7], CUBE[2], CUBE[7], CUBE[3],
	CUBE[3], CUBE[7], CUBE[4], CUBE[3], CUBE[4], CUBE[0]
};
const char *const SHADER_VERT = R"(
#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 colour_in;

out vec3 colour_frag;

uniform mat4 view;
uniform mat4 proj;

void main() {
	gl_Position = proj * view * vec4(position_in, 1.0f);
	colour_frag = colour_in;
}
)";
const char *const SHADER_FRAG = R"(
#version 330 core

in vec3 colour_frag;

out vec4 colour_out;

void main() {
	colour_out = vec4(colour_frag, 1.0f);
}
)";

GLuint program = 0, vao = 0, vbo = 0;
GLint view_uniform = 0, proj_uniform = 0;
glm::mat4 proj;

bool Graphics::init() {
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		logger("Failed to initialize GLEW.");
		return false;
	}

	enable_gl_debug_output();

	glClearColor(0.0f, 0.5f, 1.0f, 1.0f);
	//glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Loading shaders
	int ret = load_program(program, SHADER_VERT, nullptr, SHADER_FRAG);
	if (ret) {
		logger("Failed to load shaders.");
		return false;
	}
	view_uniform = glGetUniformLocation(program, "view");
	proj_uniform = glGetUniformLocation(program, "proj");

	// Generate tris buffer and vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)sizeof(glm::vec3));
	glEnableVertexAttribArray(1);

	glUseProgram(program);

	glBufferData(GL_ARRAY_BUFFER, sizeof(VERT_DATA), VERT_DATA, GL_STATIC_DRAW);

	logger("Successfully initialised graphics.");
	return true;
}

void Graphics::deinit() {
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	logger("Successfully deinitialised graphics.");
}

void Graphics::render(const Camera *camera) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniformMatrix4fv(proj_uniform, 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(view_uniform, 1, GL_FALSE, &camera->getMatrix()[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, sizeof(VERT_DATA) / sizeof(vertex_t));
}

void Graphics::resize(glm::ivec2 dims) {
	glViewport(0, 0, dims.x, dims.y);
	proj = glm::perspective(glm::radians(70.0f), (float)dims.x / (float)dims.y, 0.1f, 100.0f);
}
