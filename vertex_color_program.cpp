#include "vertex_color_program.hpp"

#include "compile_program.hpp"

VertexColorProgram::VertexColorProgram() {
	program = compile_program_file("shaders/vert_vertex_colored.glsl", "shaders/frag_vertex_colored.glsl");

	object_to_clip_mat4 = glGetUniformLocation(program, "object_to_clip");
	object_to_light_mat4x3 = glGetUniformLocation(program, "object_to_light");
	normal_to_light_mat3 = glGetUniformLocation(program, "normal_to_light");

	sun_direction_vec3 = glGetUniformLocation(program, "sun_direction");
	sun_color_vec3 = glGetUniformLocation(program, "sun_color");
	sky_direction_vec3 = glGetUniformLocation(program, "sky_direction");
	sky_color_vec3 = glGetUniformLocation(program, "sky_color");
}

Load< VertexColorProgram > vertex_color_program(LoadTagInit, [](){
	return new VertexColorProgram();
});
