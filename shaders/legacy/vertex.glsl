#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoords_in;
layout (location = 2) in vec3 vertex_normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
out vec2 texCoords;
out vec3 normal;
out vec3 frag_pos;

void main()
{
	mat3 normal_matrix = mat3(transpose(inverse(model)));
	normal = normal_matrix * vertex_normal;
	gl_Position = proj * view * model * vec4(pos, 1.0);
	frag_pos = vec3(model * vec4(pos, 1.0));
	texCoords = texCoords_in;
}
