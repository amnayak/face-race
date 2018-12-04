#include "cube_program.hpp"

#include "compile_program.hpp"
#include "gl_errors.hpp"

CubeProgram::CubeProgram() {
	program = compile_program_file("shaders/vert_cube_map.glsl", "shaders/frag_cube_map.glsl");
	object_to_clip_mat4 = glGetUniformLocation(program, "object_to_clip");

	glUseProgram(program);

	GLuint tex_samplerCube = glGetUniformLocation(program, "tex");
	glUniform1i(tex_samplerCube, 0);

	glUseProgram(0);

	GL_ERRORS();
}

Load< CubeProgram > cube_program(LoadTagInit, [](){
	return new CubeProgram();
});
