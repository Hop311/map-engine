#pragma once

#include <GL/glew.h>

void enable_gl_debug_output(void);
int load_shader(GLenum shader_type, GLuint &shader, const char *source);
int load_program(GLuint &program, const char *vertex_shader, const char *geometry_shader, const char *fragment_shader);

int load_texture(const char *filepath, GLuint &tex_id, GLint &width, GLint &height, GLint min_filter, GLint mag_filter, unsigned soil_flags);
int load_bmp_texture_unpaletted(const char *filepath, GLuint &tex_id, GLint &width, GLint &height, GLint min_filter, GLint mag_filter, unsigned soil_flags);
