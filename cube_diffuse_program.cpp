#include "cube_diffuse_program.hpp"

#include "compile_program.hpp"
#include "gl_errors.hpp"

CubeDiffuseProgram::CubeDiffuseProgram() {
	program = compile_program_file("shaders/vert_cube_diffuse.glsl", "shaders/frag_cube_diffuse.glsl");

	object_to_clip_mat4 = glGetUniformLocation(program, "object_to_clip");
	normal_to_light_mat3 = glGetUniformLocation(program, "normal_to_light");

	glUseProgram(program);

	GLuint tex_samplerCube = glGetUniformLocation(program, "tex");
	glUniform1i(tex_samplerCube, 0);

	glUseProgram(0);

	GL_ERRORS();
}

Load< CubeDiffuseProgram > cube_diffuse_program(LoadTagInit, [](){
	return new CubeDiffuseProgram();
});
