#ifdef VERTEX

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

uniform mat4 projection;
uniform mat4 view;

uniform sampler2D displacement_map;

out vec2 f_uv;
out vec3 f_normal;

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
}

#endif

#ifdef FRAGMENT

const vec3 SUN_COLOR = vec3(1.0, 1.0, 1.0);
const float AMBIENT_STRENGTH = 0.05;
const float SPECULAR = 0.2;

in vec2 f_uv;
in vec3 f_normal;

uniform mat4 projection;
uniform mat4 view;

uniform sampler2D displacement_map;

out vec4 fragment_color;

void main() {
    vec3 camera_position = -1.0 * vec3(view[3][0], view[3][1], view[3][2]);
    vec3 camera_dir = -1.0 * normalize(vec3(view[0][2], view[1][2], view[2][2]));
    vec3 sun_dir_n = normalize(vec3(1.0, -0.5, 1.0));

    // Specular
    vec3 reflect_dir = reflect(-sun_dir_n, f_normal);
    float specular_intensity = pow(max(dot(reflect_dir, camera_dir), 0.0), 16);
    vec3 specular = SPECULAR * specular_intensity * SUN_COLOR;

    // Ambient
    vec3 ambient = AMBIENT_STRENGTH * SUN_COLOR;

    // Diffuse
    float diffuse_intensity = max(dot(f_normal, -sun_dir_n), 0.0);
    vec3 diffuse = diffuse_intensity * SUN_COLOR;

    vec3 water_color = vec3(0, 0.458, 0.466);
    // vec3 water_color = vec3(1.0, 1.0, 1.0);
    vec3 result = (ambient + (diffuse + specular)) * water_color;

    // fragment_color = vec4(displacement.xyz, 1.0);
    // fragment_color = vec4(f_normal, 1.0);
    fragment_color = vec4(result, 1.0);
}

#endif