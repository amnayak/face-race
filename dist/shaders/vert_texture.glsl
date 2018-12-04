#version 330
uniform mat4 object_to_clip;
uniform mat4x3 object_to_light;
uniform mat3 normal_to_light;
uniform mat3 normal_to_clip;
uniform mat4 light_to_spot;
uniform mat4 world_to_clip;

layout(location=0) in vec4 Position;
in vec3 Normal;
in vec4 Color;
in vec2 TexCoord;

out vec3 position;
out vec3 normal;
out vec3 clipNormal;
out vec4 color;
out vec2 texCoord;
out vec4 spotPosition;

void main() {
	gl_Position = object_to_clip * Position;
	position = object_to_light * Position;
	spotPosition = light_to_spot * vec4(position, 1.0);
	normal = normal_to_light * Normal;
	clipNormal = vec3(world_to_clip * vec4(normal, 0.0));
	color = Color;
	texCoord = TexCoord;
}