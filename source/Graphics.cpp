#include "Graphics.hpp"

#include "Logger.hpp"
#include "GLTools.hpp"
#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <SOIL2.h>

#include "map_vert.glsl"
#include "map_frag.glsl"

#define MAP_DIR R"(C:\Program Files (x86)\Steam\steamapps\common\Victoria 2\map)"

typedef glm::vec2 vertex_t;
const float MAP_SIZE = 20.0f, MAP_HEIGHT = -2.5f, TILE_SIZE = 1.0f;
static int rows, indicies_per_row;

struct Texture {
	const char *filepath;
	GLuint id;
	glm::ivec2 dims;
	float aspect_ratio;
};
enum Assets : int {
	COLOURMAP, TERRAIN, TEXTURESHEET, ASSET_COUNT
};
static const char *const ASSET_PATHS[ASSET_COUNT] = {
	MAP_DIR "/terrain/colormap.dds", MAP_DIR "/terrain.bmp", MAP_DIR "/terrain/texturesheet.tga"
};
static const char *ASSET_UNIFORMS[ASSET_COUNT] = {
	"colormap_tex", "terrain_tex", "texturesheet_tex"
};
typedef int (*load_texture_func)(const char *filepath, GLuint &tex_id, GLint &width, GLint &height, GLint min_filter, GLint mag_filter, unsigned soil_flags);
static load_texture_func ASSET_LOAD_FUNCS[ASSET_COUNT] = {
	load_texture, load_bmp_texture_unpaletted, load_texture
};
static GLint ASSET_FILTERS[ASSET_COUNT] = {
	GL_LINEAR, GL_NEAREST, GL_LINEAR
};
static unsigned ASSET_SOIL_FLAGS[ASSET_COUNT] = {
	SOIL_FLAG_INVERT_Y, 0, 0
};
static Texture textures[ASSET_COUNT];

static GLuint program, vao, vbo;
static glm::mat4 model, proj;
static bool draw_3D = true;
static struct {
	GLint model, view, proj, draw_3D;
} vert_uniforms;
static struct {
	GLint textures[ASSET_COUNT];
	GLint terrain_dims;
} frag_uniforms;

bool Graphics::init(void) {
	if constexpr (ASSET_COUNT <= 0) {
		logger("No assets to load.");
		return false;
	}
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
	vert_uniforms.model = glGetUniformLocation(program, "model");
	vert_uniforms.view = glGetUniformLocation(program, "view");
	vert_uniforms.proj = glGetUniformLocation(program, "proj");
	vert_uniforms.draw_3D = glGetUniformLocation(program, "draw_3D");
	for (int idx = 0; idx < ASSET_COUNT; ++idx)
		frag_uniforms.textures[idx] = glGetUniformLocation(program, ASSET_UNIFORMS[idx]);
	frag_uniforms.terrain_dims = glGetUniformLocation(program, "terrain_dims");

	// Load images
	for (int idx = 0; idx < ASSET_COUNT; ++idx) {
		Texture &tex = textures[idx];
		tex.filepath = ASSET_PATHS[idx];
		if (ASSET_LOAD_FUNCS[idx](tex.filepath, tex.id, tex.dims.x, tex.dims.y,
			ASSET_FILTERS[idx], ASSET_FILTERS[idx], ASSET_SOIL_FLAGS[idx])) {
			ret = 1;
			break;
		}
		tex.aspect_ratio = (float)tex.dims.x / (float)tex.dims.y;
		logger("Loaded ", tex.filepath, " with dims ", tex.dims.x, " x ", tex.dims.y, " (aspect ratio ", tex.aspect_ratio, ").");
	}
	if (ret) {
		for (int idx = 0; idx < ASSET_COUNT; ++idx)
			glDeleteTextures(1, &textures[idx].id);
		glDeleteProgram(program);
		return false;
	}
	model = glm::scale(glm::mat4{1.0f}, {textures[TERRAIN].aspect_ratio * MAP_SIZE, 1.0f, MAP_SIZE});
	model = glm::translate(model, { -0.5f, MAP_HEIGHT, -0.5f });

	// Generate tris buffer and vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)0);
	glEnableVertexAttribArray(0);

	glUseProgram(program);
	glUniformMatrix4fv(vert_uniforms.model, 1, GL_FALSE, &model[0][0]);
	for (int idx = 0; idx < ASSET_COUNT; ++idx) {
		glActiveTexture(GL_TEXTURE0 + idx);
		glBindTexture(GL_TEXTURE_2D, textures[idx].id);
		glUniform1i(frag_uniforms.textures[idx], idx);
	}
	const glm::vec2 map_dims{ (float)textures[TERRAIN].dims.x, (float)textures[TERRAIN].dims.y };
	glUniform2f(frag_uniforms.terrain_dims, map_dims.x, map_dims.y);

	const glm::vec2 tile_count{ ceil(map_dims / TILE_SIZE) };
	const glm::ivec2 tile_counti{ (int)tile_count.x, (int)tile_count.y };
	const glm::vec2 tile_dims{ 1.0f / tile_count };
	indicies_per_row = 2 * (tile_counti.x + 1);
	rows = tile_counti.y;
	vertex_t *verticies = new vertex_t[indicies_per_row * tile_counti.y];
	int pos = 0;
	for (int y = 0; y < tile_counti.y; ++y)
		for (int x = 0; x < tile_counti.x + 1; ++x) {
			verticies[pos++] = { (float)x * tile_dims.x, (float)y * tile_dims.y };
			verticies[pos++] = { (float)x * tile_dims.x, (float)(y + 1) * tile_dims.y };
		}

	glBufferData(GL_ARRAY_BUFFER, indicies_per_row * tile_counti.y * sizeof(vertex_t), verticies, GL_STATIC_DRAW);
	delete[] verticies;

	logger("Successfully initialised graphics.");
	return true;
}

void Graphics::deinit(void) {
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	for (int idx = 0; idx < ASSET_COUNT; ++idx)
		glDeleteTextures(1, &textures[idx].id);
	glDeleteProgram(program);

	logger("Successfully deinitialised graphics.");
}

void Graphics::render(const Camera *camera) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniformMatrix4fv(vert_uniforms.proj, 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(vert_uniforms.view, 1, GL_FALSE, &camera->getMatrix()[0][0]);
	glUniform1i(vert_uniforms.draw_3D, draw_3D);
	for (int y = 0; y < rows; ++y)
		glDrawArrays(GL_TRIANGLE_STRIP, y * indicies_per_row, indicies_per_row);
}

void Graphics::resize(glm::ivec2 dims) {
	glViewport(0, 0, dims.x, dims.y);
	proj = glm::perspective(glm::radians(70.0f), (float)dims.x / (float)dims.y, 0.1f, 100.0f);
}

void Graphics::togggle_draw_3D(void) {
	draw_3D = !draw_3D;
}
