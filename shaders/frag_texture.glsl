#version 330

uniform vec3 sun_direction;
uniform vec3 sun_color;
uniform vec3 spot_position;
uniform vec3 spot_direction;
uniform vec3 spot_color;
uniform vec2 spot_outer_inner;

uniform sampler2D tex;
uniform sampler2DShadow spot_depth_tex;
uniform samplerCube ibl;

in vec3 position;
in vec3 normal;
in vec4 color;
in vec2 texCoord;
in vec4 spotPosition;

out vec4 fragColor;

void main() {
	vec3 total_light = vec3(0.0, 0.0, 0.0);
	vec3 n = normalize(normal);
	{ //sky (IBL) light:
		vec3 light = texture(ibl, normal).rgb;
		vec3 col = color.rgb * light;
		vec3 contrib = vec3(pow(col.r,0.45), pow(col.g,0.45), pow(col.b,0.45));
		total_light += 0.8 * contrib;
	}
	{ //sun (directional) light:
		// vec3 l = sun_direction;
		// float nl = max(0.0, dot(n,l));
		// total_light += nl * sun_color;
	}
	{ //spot (point with fov + shadow map) light:
		vec3 l = normalize(spot_position - position);
		float nl = max(0.0, dot(n,l));
		// TODO: look up shadow map
		float d = dot(l,-spot_direction);
		float amt = smoothstep(spot_outer_inner.x, spot_outer_inner.y, d);
		float shadow = textureProj(spot_depth_tex, spotPosition);
		total_light += 0.4 * shadow * nl * amt * spot_color;
		//fragColor = vec4(s,s,s, 1.0); //DEBUG: just show shadow
	}
	fragColor = texture(tex, texCoord) * vec4(color.rgb * total_light, color.a);
}