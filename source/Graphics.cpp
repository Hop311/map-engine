#include "Graphics.hpp"

#include "Logger.hpp"
#include "GLTools.hpp"
#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <SOIL2.h>

#include "map_vert.glsl"
#include "map_frag.glsl"

#define MAP_DIR R"(C:\Program Files (x86)\Steam\steamapps\common\Victoria 2\map)"

typedef int (*load_texture_func_t)(const char *filepath, GLuint &tex_id, GLint &width, GLint &height, GLint min_filter, GLint mag_filter, unsigned soil_flags);
struct Texture {
	const char *filepath, *uniform;
	load_texture_func_t load_texture_func;
	GLuint filter, soil_flags, id;
	glm::ivec2 dims;
	float aspect_ratio;
};
enum Assets : int {
	TERRAIN, TEXTURESHEET, COLOURMAP, COLORMAP_WATER, ASSET_COUNT
};
static Texture textures[ASSET_COUNT] = {
	{ MAP_DIR "/terrain.bmp", "terrain_tex", load_bmp_texture_unpaletted, GL_NEAREST, 0, 0, { 0, 0 }, 0.0f },
	{ MAP_DIR "/terrain/texturesheet.tga", "texturesheet_tex", load_texture, GL_LINEAR, 0, 0, { 0, 0 }, 0.0f },
	{ MAP_DIR "/terrain/colormap.dds", "colormap_tex", load_texture, GL_LINEAR, SOIL_FLAG_INVERT_Y, 0, { 0, 0 }, 0.0f },
	{ MAP_DIR "/terrain/colormap_water.dds", "colormap_water_tex", load_texture, GL_LINEAR, SOIL_FLAG_INVERT_Y, 0, { 0, 0 }, 0.0f },
};
const float MAP_SIZE = 20.0f, MAP_HEIGHT = -2.5f, TILE_SIZE = 1.0f;
static int rows, indicies_per_row;

typedef glm::vec2 vertex_t;
static GLuint program, vao, vbo;
static glm::mat4 model, proj;
static struct {
	struct { GLint model, view, proj, draw_3D; } vert;
	struct { GLint textures[ASSET_COUNT], terrain_dims; } frag;
} uniforms;
static bool draw_3D = true;

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
	uniforms.vert.model = glGetUniformLocation(program, "model");
	uniforms.vert.view = glGetUniformLocation(program, "view");
	uniforms.vert.proj = glGetUniformLocation(program, "proj");
	uniforms.vert.draw_3D = glGetUniformLocation(program, "draw_3D");
	for (int idx = 0; idx < ASSET_COUNT; ++idx)
		uniforms.frag.textures[idx] = glGetUniformLocation(program, textures[idx].uniform);
	uniforms.frag.terrain_dims = glGetUniformLocation(program, "terrain_dims");

	// Load images
	for (int idx = 0; idx < ASSET_COUNT; ++idx) {
		Texture &tex = textures[idx];
		if (tex.load_texture_func(tex.filepath, tex.id, tex.dims.x,
			tex.dims.y, tex.filter, tex.filter, tex.soil_flags)) {
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
	glUniformMatrix4fv(uniforms.vert.model, 1, GL_FALSE, &model[0][0]);
	for (int idx = 0; idx < ASSET_COUNT; ++idx) {
		glActiveTexture(GL_TEXTURE0 + idx);
		glBindTexture(GL_TEXTURE_2D, textures[idx].id);
		glUniform1i(uniforms.frag.textures[idx], idx);
	}
	const glm::vec2 map_dims{ (float)textures[TERRAIN].dims.x, (float)textures[TERRAIN].dims.y };
	glUniform2f(uniforms.frag.terrain_dims, map_dims.x, map_dims.y);

	const glm::vec2 tile_count{ ceil(map_dims / TILE_SIZE + 0.5f) };
	const glm::vec2 tile_dims{ 1.0f / tile_count };
	const glm::ivec2 tile_counti{ (int)tile_count.x, (int)tile_count.y };
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
	glUniformMatrix4fv(uniforms.vert.proj, 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(uniforms.vert.view, 1, GL_FALSE, &camera->getMatrix()[0][0]);
	glUniform1i(uniforms.vert.draw_3D, draw_3D);
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
