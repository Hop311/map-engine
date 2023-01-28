#pragma once

#include <gl/glew.h>

void enable_gl_debug_output();
int load_shader(GLenum shader_type, GLuint &shader, const char *source);
int load_program(GLuint &program, const char *vertex_shader, const char *geometry_shader, const char *fragment_shader);
