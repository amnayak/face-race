#version 330
uniform samplerCube diffuse_tex; //blurry world
uniform samplerCube reflect_tex; //shiny world
uniform vec3 eye; //eye position in lighting space
in vec3 position;
in vec3 normal;
in vec4 color;
out vec4 fragColor;
void main() {
	vec3 n = normalize(normal);
	vec3 v = normalize(eye-position);
	vec3 light = texture(diffuse_tex, n).rgb;
	vec3 col;

//refract:
//"	col = texture(reflect_tex, refract(-v, n, 1.0 / 1.325)).rgb;\n"

//pure mirror:
//"	col = texture(reflect_tex, reflect(-v, n)).rgb;\n"

//reflect + refract:
	float refl = mix(0.06, 1.0, pow(1.0 - max(0.0, dot(n, v)), 5.0)); //schlick's approximation
	col = (1.0 - refl) * texture(reflect_tex, refract(-v, n, 1.0 / 1.5)).rgb;
	col += refl * texture(reflect_tex, reflect(-v, n)).rgb;

//Varying roughness:
//"	col = texture(reflect_tex, reflect(-v, n), round(0.5+0.5*sin(5.0*position.z)) * 4.0).rgb;\n"

//partial mirror: (~"clear coat", but probably could use angular dependence)
//"	col = 0.5 * color.rgb * light;\n"
//"	col += 0.5 * texture(reflect_tex, reflect(-v, n)).rgb;\n"

	fragColor = vec4(pow(col.r,0.45), pow(col.g,0.45), pow(col.b,0.45), color.a);
}