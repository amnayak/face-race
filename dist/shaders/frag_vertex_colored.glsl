#version 330

uniform vec3 sun_direction;
uniform vec3 sun_color;
uniform vec3 sky_direction;
uniform vec3 sky_color;

in vec3 position;
in vec3 normal;
in vec4 color;
out vec4 fragColor;

void main() {
       vec3 total_light = vec3(0.0, 0.0, 0.0);
       vec3 n = normalize(normal);
       { //sky (hemisphere) light:
               vec3 l = sky_direction;
               float nl = 0.5 + 0.5 * dot(n,l);
               total_light += nl * sky_color;
       }
       { //sun (directional) light:
               vec3 l = sun_direction;
               float nl = max(0.0, dot(n,l));
               total_light += nl * sun_color;
       }
       fragColor = vec4(color.rgb * total_light, color.a);
}