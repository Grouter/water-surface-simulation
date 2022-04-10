#ifdef VERTEX

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

uniform mat4 projection;
uniform mat4 view;

uniform sampler2D displacement_map;

const vec3 light_position = vec3(0.0, 0.0, 0.0);

out vec2 f_uv;
out vec3 f_normal;
out vec3 light;
out vec3 halfway;

vec3 get_normal(vec2 pos) {
    vec3 off = vec3(-1.0, 0.0, 1.0);
    vec2 size = vec2(2.0, 0.0);
	vec2 texsize = vec2(textureSize(displacement_map, 0));

    float l = texture(displacement_map, pos + off.xy / texsize).g; // 01
	float r = texture(displacement_map, pos + off.zy / texsize).g; // 21
	float u = texture(displacement_map, pos + off.yx / texsize).g; // 10
	float d = texture(displacement_map, pos + off.yz / texsize).g; // 12

	vec3 va = normalize(vec3(size.xy, l - r));
	vec3 vb = normalize(vec3(-size.yx, u - d));

	return normalize(cross(va, vb));
}

void main() {
    f_uv = uv;
    f_normal = get_normal(uv);

    vec4 displacement = texture(displacement_map, uv);
    vec3 diplaced_position = position + displacement.xyz;
    gl_Position = projection * view * vec4(diplaced_position, 1.0);

    light = normalize((view * vec4(light_position, 1.0)).xyz - gl_Position.xyz);
    halfway = light + normalize(-gl_Position.xyz);
}

#endif

#ifdef FRAGMENT

const vec3 SUN_COLOR = vec3(1.0, 1.0, 1.0);
const float AMBIENT_STRENGTH = 0.05;
const float SPECULAR = 0.5;

in vec2 f_uv;
in vec3 f_normal;
in vec3 light;
in vec3 halfway;

uniform mat4 projection;
uniform mat4 view;

// uniform sampler2D debug_map;

out vec4 fragment_color;

void main() {
    vec3 light_n   = normalize(light);
    vec3 halfway_n = normalize(halfway);

    vec3 water_color = vec3(0, 0.458, 0.466);

    // Specular
    float specular_intensity = max(pow(dot(f_normal, halfway_n), 16.0), 0.0);
    vec3 specular = SPECULAR * specular_intensity * SUN_COLOR;

    // Ambient
    vec3 ambient = AMBIENT_STRENGTH * SUN_COLOR;

    // Diffuse
    float diffuse_intensity = max(dot(f_normal, light_n), 0.0);
    vec3 diffuse = diffuse_intensity * SUN_COLOR;

    vec3 result = (ambient + (diffuse + specular)) * water_color;

    // fragment_color = vec4(f_normal, 1.0);
    fragment_color = vec4(result, 1.0);

    // vec4 debug_color = texture(debug_map, f_uv);
    // fragment_color = vec4(debug_color.xy, 0.0, 1.0);
}

#endif
