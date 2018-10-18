#version 330
// SHADER SOURCE:
// https://github.com/libgdx/libgdx/blob/master/gdx/src/com/badlogic/gdx/graphics/g2d/DistanceFieldFont.java

uniform sampler2D u_texture;
uniform float u_smoothing;

in vec4 v_color;
in vec2 v_texCoords;

out vec4 fragColor;

void main() {
	if (u_smoothing > 0.0) {
		float smoothing = 0.25 / u_smoothing;
		float dist = texture(u_texture, v_texCoords).a;
		float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, dist);
		fragColor = vec4(v_color.rgb, alpha * v_color.a);
	} else {
		fragColor = v_color * texture(u_texture, v_texCoords).a;
	}
}