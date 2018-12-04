#include "cube_reflect_program.hpp"

#include "compile_program.hpp"
#include "gl_errors.hpp"

CubeReflectProgram::CubeReflectProgram() {
	program =  compile_program_file("shaders/vert_cube_reflect.glsl", "shaders/frag_cube_reflect.glsl");

	object_to_clip_mat4 = glGetUniformLocation(program, "object_to_clip");
	object_to_light_mat4x3 = glGetUniformLocation(program, "object_to_light");
	normal_to_light_mat3 = glGetUniformLocation(program, "normal_to_light");
	eye_vec3 = glGetUniformLocation(program, "eye");

	glUseProgram(program);

	GLuint diffuse_tex_samplerCube = glGetUniformLocation(program, "diffuse_tex");
	glUniform1i(diffuse_tex_samplerCube, 0);

	GLuint reflect_tex_samplerCube = glGetUniformLocation(program, "reflect_tex");
	glUniform1i(reflect_tex_samplerCube, 1);

	glUseProgram(0);

	GL_ERRORS();
}

Load< CubeReflectProgram > cube_reflect_program(LoadTagInit, [](){
	return new CubeReflectProgram();
});
