#version 330

uniform mat4 u_projTrans;

in vec2 position;
in vec2 texcoord0;

out vec2 v_texCoord;

void main() {
	v_texCoord = texcoord0;
	gl_Position = u_projTrans * vec4(position,0,1);
}