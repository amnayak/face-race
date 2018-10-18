#include "compile_program.hpp"

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>

static GLuint compile_shader(GLenum type, std::string const &source) {
	GLuint shader = glCreateShader(type);
	GLchar const *str = source.c_str();
	GLint length = GLint(source.size());
	glShaderSource(shader, 1, &str, &length);
	glCompileShader(shader);
	GLint compile_status = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE) {
		std::cerr << "Failed to compile shader." << std::endl;
		GLint info_log_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector< GLchar > info_log(info_log_length, 0);
		GLsizei length = 0;
		glGetShaderInfoLog(shader, GLint(info_log.size()), &length, &info_log[0]);
		std::cerr << "Info log: " << std::string(info_log.begin(), info_log.begin() + length);
		glDeleteShader(shader);
		throw std::runtime_error("Failed to compile shader.");
	}
	return shader;
}

GLuint compile_program(
	std::string const &vertex_shader_source,
	std::string const &fragment_shader_source
	) {

	GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
	GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	//shaders are reference counted so this makes sure they are freed after program is deleted:
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	//link the shader program and throw errors if linking fails:
	glLinkProgram(program);
	GLint link_status = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE) {
		std::cerr << "Failed to link shader program." << std::endl;
		GLint info_log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector< GLchar > info_log(info_log_length, 0);
		GLsizei length = 0;
		glGetProgramInfoLog(program, GLint(info_log.size()), &length, &info_log[0]);
		std::cerr << "Info log: " << std::string(info_log.begin(), info_log.begin() + length);
		throw std::runtime_error("failed to link program");
	}

	return program;
}

GLuint compile_program_file(
	std::string const &vertex_shader_source,
	std::string const &fragment_shader_source
	) {

	auto read_file = [](std::string const &src) {
		// http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
		std::ifstream in(src, std::ios::in | std::ios::binary);
		if (in)
		{
			std::string contents;
			in.seekg(0, std::ios::end);
			contents.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&contents[0], contents.size());
			in.close();
			return contents;
		}

		std::cerr << "ERROR compiling program: Can't find file " << src << std::endl;
		return std::string("");
	};

	
	std::string vertex_program = read_file(vertex_shader_source);
	std::string frag_program = read_file(fragment_shader_source);

	return compile_program(vertex_program, frag_program);
}
