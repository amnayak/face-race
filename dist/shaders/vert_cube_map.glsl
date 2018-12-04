#version 330

uniform mat4 object_to_clip;
layout(location=0) in vec4 Position; //note: layout keyword used to make sure that the location-0 attribute is always bound to something
in vec3 TexCoord;
out vec3 texCoord;
void main() {
   gl_Position = object_to_clip * Position;
   texCoord = TexCoord;
}