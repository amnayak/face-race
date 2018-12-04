#version 330
uniform mat4 object_to_clip;
uniform mat3 normal_to_light;
layout(location=0) in vec4 Position; //note: layout keyword used to make sure that the location-0 attribute is always bound to something
in vec3 Normal;
in vec4 Color;
out vec3 normal;
out vec4 color;
void main() {
	gl_Position = object_to_clip * Position;
	normal = normal_to_light * Normal;
	color = Color;
}
