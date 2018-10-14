// SHADER SOURCE:
// https://github.com/libgdx/libgdx/blob/master/gdx/src/com/badlogic/gdx/graphics/g2d/DistanceFieldFont.java

attribute vec4 position;
attribute vec4 color;
attribute vec2 texcoord0;

uniform mat4 u_projTrans;
varying vec4 v_color;
varying vec2 v_texCoords;

void main() {
	v_color = color;
	v_texCoords = texcoord0;
	gl_Position =  u_projTrans * position;
}