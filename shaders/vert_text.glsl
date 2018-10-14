#version 330
// SHADER SOURCE:
// https://github.com/libgdx/libgdx/blob/master/gdx/src/com/badlogic/gdx/graphics/g2d/DistanceFieldFont.java

uniform mat4 u_projTrans;

in vec4 position; // in ndc
in vec4 color;
in vec2 texcoord0;

out vec4 v_color;
out vec2 v_texCoords;

void main() {
	v_color = color;
	v_texCoords = texcoord0;
	gl_Position = u_projTrans * position;
}