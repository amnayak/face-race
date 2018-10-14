#version 330

in vec3 color;
//"uniform vec4 color;

out vec4 fragColor;

void main() {
	fragColor = vec4(color, 1.0);
}