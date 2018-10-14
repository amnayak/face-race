#version 330
uniform mat4 object_to_clip;
uniform mat4x3 object_to_light;
uniform mat3 normal_to_light;
uniform mat4 light_to_spot;

layout(location=0) in vec4 Position;
in vec3 Normal;
in vec4 Color;
in vec2 TexCoord;

out vec3 position;
out vec3 normal;
out vec4 color;
out vec2 texCoord;
out vec4 spotPosition;

void main() {
	gl_Position = object_to_clip * Position;
	position = object_to_light * Position;
	spotPosition = light_to_spot * vec4(position, 1.0);
	normal = normal_to_light * Normal;
	color = Color;
	texCoord = TexCoord;
}