#version 330

uniform vec2 res;
uniform samplerCube tex;
in vec3 texCoord;
out vec4 fragColor;

void main() {
   vec3 col = texture(tex, texCoord, 6.5f).rgb;
   fragColor = vec4(pow(col.r,0.45), pow(col.g,0.45), pow(col.b,0.45), 1.0);
}