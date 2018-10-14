// SHADER SOURCE:
// https://github.com/libgdx/libgdx/blob/master/gdx/src/com/badlogic/gdx/graphics/g2d/DistanceFieldFont.java

uniform sampler2D u_texture;
uniform float u_smoothing;
varying vec4 v_color;
varying vec2 v_texCoords;

void main() {
	if (u_smoothing > 0.0) {
		float smoothing = 0.25 / u_smoothing;
		float distance = texture2D(u_texture, v_texCoords).a;
		float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
		gl_FragColor = vec4(v_color.rgb, alpha * v_color.a);
	} else {
		gl_FragColor = v_color * texture2D(u_texture, v_texCoords);
	}
}