#pragma once

#include "GL.hpp"

#include <string>
#include <fstream>

//compiles+links an OpenGL shader program from source.
// throws on compilation error.
GLuint compile_program(
	std::string const &vertex_shader_source,
	std::string const &fragment_shader_source);

GLuint compile_program_file(
	std::string const &vertex_shader_source,
	std::string const &fragment_shader_source);