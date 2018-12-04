#include "texture_program.hpp"

#include "compile_program.hpp"
#include "gl_errors.hpp"

TextureProgram::TextureProgram() {
	program = compile_program_file("shaders/vert_texture.glsl", "shaders/frag_texture.glsl");

	object_to_clip_mat4 = glGetUniformLocation(program, "object_to_clip");
	object_to_light_mat4x3 = glGetUniformLocation(program, "object_to_light");
	normal_to_light_mat3 = glGetUniformLocation(program, "normal_to_light");

	sun_direction_vec3 = glGetUniformLocation(program, "sun_direction");
	sun_color_vec3 = glGetUniformLocation(program, "sun_color");

	spot_position_vec3 = glGetUniformLocation(program, "spot_position");
	spot_direction_vec3 = glGetUniformLocation(program, "spot_direction");
	spot_color_vec3 = glGetUniformLocation(program, "spot_color");
	spot_outer_inner_vec2 = glGetUniformLocation(program, "spot_outer_inner");

	light_to_spot_mat4 = glGetUniformLocation(program, "light_to_spot");

	glUseProgram(program);

	GLuint tex_sampler2D = glGetUniformLocation(program, "tex");
	glUniform1i(tex_sampler2D, 0);

	GLuint spot_depth_tex_sampler2D = glGetUniformLocation(program, "spot_depth_tex");
	glUniform1i(spot_depth_tex_sampler2D, 1);

	GLuint tex_samplerCube = glGetUniformLocation(program, "ibl");
	glUniform1i(tex_samplerCube, 2);

	glUseProgram(0);

	GL_ERRORS();
}

Load< TextureProgram > texture_program(LoadTagInit, [](){
	return new TextureProgram();
});
