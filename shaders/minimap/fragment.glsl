#version 330 core

out vec4 frag_color;

in GS_OUT
{
	vec2 texCoords;
	vec3 frag_norm;
} fs_in;

struct Material
{
	sampler2D diffuse_1;
};

struct Sun
{
	vec3 dir;
	vec3 color;
};

uniform Material material;
uniform Sun sun;

void main()
{
	// color
	vec4 tatooine_color = vec4(1.0, 0.635, 0.188, 1.0);
	vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 base_color = texture(material.diffuse_1, fs_in.texCoords);

	// ambient
	vec3 ambient = sun.color * 0.1;

	// diffuse
	float diff_str = max(dot(-sun.dir, fs_in.frag_norm), 0);
	vec3 diffuse = diff_str * sun.color * 0.65;

	// final color
	vec3 color = (diffuse + ambient) * vec3(base_color);
	frag_color = vec4(color, 1.0);
}

