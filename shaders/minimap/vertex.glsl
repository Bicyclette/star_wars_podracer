#version 330 core

layout (location=0) in vec3 pos;
layout (location=1) in vec3 vertex_normal;
layout (location=2) in vec2 vertex_texCoords;
layout (location=3) in vec2 vertex_bonesID;
layout (location=4) in vec2 vertex_bonesWeight;

out VS_OUT
{
	vec2 texCoords;
	vec3 frag_norm;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	mat4 norm_mat = inverse(transpose(model));
	vs_out.texCoords = vertex_texCoords;
	vs_out.frag_norm = vec3(norm_mat * vec4(vertex_normal, 1.0));
	gl_Position = proj * view * model * vec4(pos, 1.0);
}

