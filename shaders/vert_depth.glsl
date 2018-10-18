#version 330

uniform mat4 object_to_clip;

layout(location=0) in vec4 Position;
in vec3 Normal;
in vec4 Color;
in vec2 TexCoord;

out vec3 color;

void main() {
	gl_Position = object_to_clip * Position;
	color = 0.5 + 0.5 * Normal;
}