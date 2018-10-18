#include "depth_program.hpp"

#include "compile_program.hpp"

DepthProgram::DepthProgram() {
	program = compile_program_file("shaders/vert_depth.glsl", "shaders/frag_depth.glsl");

	object_to_clip_mat4 = glGetUniformLocation(program, "object_to_clip");
}

Load< DepthProgram > depth_program(LoadTagInit, [](){
	return new DepthProgram();
});
