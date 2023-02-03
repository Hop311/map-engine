#include "Graphics.hpp"

#include "Logger.hpp"
#include "GLTools.hpp"
#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <SOIL2.h>

#include "map_vert.glsl"
#include "map_frag.glsl"

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

const float MAP_SIZE = 20.0f, MAP_HEIGHT = -2.5f;
const vertex_t MAP[4] = {
	{ { -MAP_SIZE, MAP_HEIGHT, -MAP_SIZE }, { 0.0f, 1.0f } },
	{ { -MAP_SIZE, MAP_HEIGHT,  MAP_SIZE }, { 0.0f, 0.0f } },
	{ {  MAP_SIZE, MAP_HEIGHT, -MAP_SIZE }, { 1.0f, 1.0f } },
	{ {  MAP_SIZE, MAP_HEIGHT,  MAP_SIZE }, { 1.0f, 0.0f } }
};
const vertex_t VERT_DATA[] = {
	MAP[0], MAP[1], MAP[2], MAP[3]
};

struct Texture {
	const char *filepath;
	GLuint id;
	glm::ivec2 dims;
	float aspect_ratio;
};
enum Assets : int {
	PROVINCES, TERRAIN, COLOURMAP, ASSET_COUNT
};
#define MAP_DIR R"(C:\Users\hop31\Documents\Workspace\map-engine\map\)"
static const char *const ASSET_PATHS[ASSET_COUNT] = {
	MAP_DIR "provinces.bmp", MAP_DIR "terrain.bmp", MAP_DIR "terrain/colormap.dds"
};
static struct {
	Texture textures[ASSET_COUNT];
	glm::ivec2 dims;
	float aspect_ratio;
} map;

static GLuint program, vao, vbo;
static GLint model_uniform, view_uniform, proj_uniform, province_tex_uniform, terrain_tex_uniform;
static glm::mat4 model, proj;

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
	model_uniform = glGetUniformLocation(program, "model");
	view_uniform = glGetUniformLocation(program, "view");
	proj_uniform = glGetUniformLocation(program, "proj");
	province_tex_uniform = glGetUniformLocation(program, "province_tex");
	terrain_tex_uniform = glGetUniformLocation(program, "terrain_tex");

	// Load images
	for (int idx = 0; idx < ASSET_COUNT; ++idx) {
		Texture &tex = map.textures[idx];
		tex.filepath = ASSET_PATHS[idx];
		if (load_texture(tex.filepath, tex.id, tex.dims.x, tex.dims.y, GL_NEAREST, GL_NEAREST)) {
			ret = 1;
			break;
		}
		tex.aspect_ratio = (float)tex.dims.x / (float)tex.dims.y;
		logger("Loaded ", tex.filepath, " with dims ", tex.dims.x, " x ", tex.dims.y, " (aspect ratio ", tex.aspect_ratio, ").");
	}
	if (ret) {
		for (int idx = 0; idx < ASSET_COUNT; ++idx)
			glDeleteTextures(1, &map.textures[idx].id);
		glDeleteProgram(program);
		return false;
	}
	map.dims = map.textures[PROVINCES].dims;
	map.aspect_ratio = map.textures[PROVINCES].aspect_ratio;
	model = glm::scale(glm::mat4{ 1.0f }, { map.aspect_ratio, 1.0f, 1.0f });

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
	glUniformMatrix4fv(model_uniform, 1, GL_FALSE, &model[0][0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, map.textures[PROVINCES].id);
	glUniform1i(province_tex_uniform, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, map.textures[TERRAIN].id);
	glUniform1i(terrain_tex_uniform, 1);

	glBufferData(GL_ARRAY_BUFFER, sizeof(VERT_DATA), VERT_DATA, GL_STATIC_DRAW);

	logger("Successfully initialised graphics.");
	return true;
}

void Graphics::deinit() {
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	for (int idx = 0; idx < ASSET_COUNT; ++idx)
		glDeleteTextures(1, &map.textures[idx].id);
	glDeleteProgram(program);

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
