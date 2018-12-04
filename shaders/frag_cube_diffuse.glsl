#version 330
uniform samplerCube tex;
in vec3 normal;
in vec4 color;
out vec4 fragColor;
void main() {
	vec3 light = texture(tex, normal).rgb;
	vec3 col = color.rgb * light;
	fragColor = vec4(pow(col.r,0.45), pow(col.g,0.45), pow(col.b,0.45), color.a);
}