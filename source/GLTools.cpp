#include "GLTools.hpp"

#include "Logger.hpp"

#include "SOIL2.h"

#include <vector>

static const char *debug_type_name(GLenum type) {
	switch (type) {
#define F(X) case GL_DEBUG_TYPE_##X: return #X;
		F(ERROR) F(DEPRECATED_BEHAVIOR) F(UNDEFINED_BEHAVIOR) F(PORTABILITY)
		F(PERFORMANCE) F(MARKER) F(PUSH_GROUP) F(POP_GROUP) F(OTHER)
#undef F
	default: return "UNKNOWN";
	}
}
static const char *debug_severity_name(GLenum severity) {
	switch (severity) {
#define F(X) case GL_DEBUG_SEVERITY_##X: return #X;
		F(HIGH) F(MEDIUM) F(LOW) F(NOTIFICATION)
#undef F
	default: return "UNKNOWN";
	}
}
static const char *debug_source_name(GLenum source) {
	switch (source) {
#define F(X) case GL_DEBUG_SOURCE_##X: return #X;
		F(API) F(WINDOW_SYSTEM) F(SHADER_COMPILER) F(THIRD_PARTY) F(APPLICATION) F(OTHER)
#undef F
	default: return "UNKNOWN";
	}
}
static const void *DEBUG_ID = (void *)0xDEB06;
static void GLAPIENTRY gl_debug_output(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user_param) {
	if (user_param != DEBUG_ID)
		logger("Unexpected user_param: ", user_param, " (was set as ", DEBUG_ID, ").");
	logger("[", debug_type_name(type), "][", debug_severity_name(severity), "][", debug_source_name(source), "] id = ", id, ", length = ", length, ":\n\n", message, "\n");
}
void enable_gl_debug_output() {
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(gl_debug_output, DEBUG_ID);
}

static const char *shader_type_name(GLenum type) {
	switch (type) {
#define F(X) case GL_##X##_SHADER: return #X;
		F(VERTEX) F(GEOMETRY) F(FRAGMENT)
#undef F
	default: return "UNKNOWN";
	}
}
int load_shader(GLenum shader_type, GLuint &shader, const char *source) {
	GLint success = GL_FALSE, info_log_length = 0;
	GLuint shader_id = glCreateShader(shader_type);
	glShaderSource(shader_id, 1, &source, nullptr);
	glCompileShader(shader_id);
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		std::vector<char> info_log(info_log_length);
		glGetShaderInfoLog(shader_id, info_log_length, nullptr, info_log.data());
		logger(shader_type_name(shader_type), " shader info log (length ", info_log_length, "):\n\n", info_log.data(), "\n");
	}
	if (success) {
		shader = shader_id;
		return 0;
	} else {
		logger(shader_type_name(shader_type), " shader compilation failed.");
		glDeleteShader(shader_id);
		shader = 0;
		return -1;
	}
}

int load_program(GLuint &program, const char *vertex_shader, const char *geometry_shader, const char *fragment_shader) {
	GLuint vertex_shader_id = 0, geometry_shader_id = 0, fragment_shader_id = 0;
	GLint ret = 0;
	if (vertex_shader) ret |= load_shader(GL_VERTEX_SHADER, vertex_shader_id, vertex_shader);
	if (geometry_shader) ret |= load_shader(GL_GEOMETRY_SHADER, geometry_shader_id, geometry_shader);
	if (fragment_shader) ret |= load_shader(GL_FRAGMENT_SHADER, fragment_shader_id, fragment_shader);
	if (ret) {
		program = 0;
		return ret;
	}
	GLuint program_id = glCreateProgram();
	if (vertex_shader_id) glAttachShader(program_id, vertex_shader_id);
	if (geometry_shader_id) glAttachShader(program_id, geometry_shader_id);
	if (fragment_shader_id) glAttachShader(program_id, fragment_shader_id);
	glLinkProgram(program_id);
	glGetProgramiv(program_id, GL_LINK_STATUS, &ret);
	int info_log_length = 0;
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		std::vector<char> info_log(info_log_length);
		glGetProgramInfoLog(program_id, info_log_length, nullptr, info_log.data());
		logger("Program info log (length ", info_log_length, "):\n\n", info_log.data(), "\n");
	}
	glDeleteShader(vertex_shader_id);
	glDeleteShader(geometry_shader_id);
	glDeleteShader(fragment_shader_id);
	if (ret) {
		program = program_id;
		return 0;
	} else {
		logger("Program linking failed.");
		glDeleteShader(program_id);
		program = 0;
		return -1;
	}
}

int load_texture(const char *filepath, GLuint &tex_id, GLint &width, GLint &height, GLint min_filter, GLint mag_filter) {
	width = 0;
	height = 0;
	tex_id = SOIL_load_OGL_texture(filepath, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	if (!tex_id) {
		logger("Failed to load texture ", filepath);
		return -1;
	}
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (width <= 0 || height <= 0) {
		logger("Invalid texture dims ", width, " x ", height, " for ", filepath);
		glDeleteTextures(1, &tex_id);
		tex_id = 0;
		width = 0;
		height = 0;
		return -1;
	}
	return 0;
}

const size_t BMP_HEADER = 54;
int load_bmp_texture_unpaletted(const char *filepath, GLuint &tex_id, GLint &width, GLint &height, GLint min_filter, GLint mag_filter) {
	tex_id = 0;
	width = 0;
	height = 0;
	FILE *file = nullptr;
	int ret = fopen_s(&file, filepath, "rb");
	if (ret || !file) {
		logger("Failed to open ", filepath, " with code ", ret);
		return -1;
	}
	uint8_t header[BMP_HEADER];
	if (fread(header, BMP_HEADER, 1, file) != 1) {
		logger("Failed to read header of ", filepath);
		fclose(file);
		return -1;
	}
	const uint32_t pixel_offset = *(uint32_t *)&header[10];
	width = *(int32_t *)&header[18];
	height = *(int32_t *)&header[22];
	if (width <= 0 || height <= 0) {
		logger("Invalid BMP texture dims ", width, " x ", height, " for ", filepath);
		width = 0;
		height = 0;
		fclose(file);
		return -1;
	}
	const int32_t size = width * height;
	ret = fseek(file, pixel_offset, SEEK_SET);
	if (ret) {
		logger("Failed to get to pixel data of ", filepath, " (at ", pixel_offset, " bytes in)");
		fclose(file);
		return -1;
	}
	uint8_t *pixels = new uint8_t[size];
	if (fread(pixels, size, 1, file) != 1) {
		logger("Failed to read pixels (", size, " bytes at offset ", pixel_offset, ")");
		delete[] pixels;
		fclose(file);
		return -1;
	}
	fclose(file);
	glGenTextures(1, &tex_id);
	if (!tex_id) {
		logger("Failed to generated texture ID for ", filepath);
		delete[] pixels;
		return -1;
	}
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
	glBindTexture(GL_TEXTURE_2D, 0);

	delete[] pixels;
	return 0;
}
