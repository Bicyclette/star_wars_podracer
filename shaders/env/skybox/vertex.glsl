#version 330 core

layout (location=0) in vec3 pos;

out VS_OUT
{
	vec3 texCoords;
} vs_out;

uniform mat4 view;
uniform mat4 proj;

void main()
{
	vs_out.texCoords = pos;
	mat4 final_view = mat4(mat3(view));
	vec4 orientation = proj * final_view * vec4(pos, 1.0);
	gl_Position = orientation.xyww;
}
