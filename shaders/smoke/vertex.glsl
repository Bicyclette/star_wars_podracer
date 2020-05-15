#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in float lifeTime;
layout (location = 2) in vec3 direction;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out VS_OUT
{
	float lifeTime;
	flat int depth;
} vs_out;

uniform int depthPass;

void main()
{
	if(depthPass == 1)
		vs_out.depth = 1;
	else
		vs_out.depth = 0;

	gl_Position = proj * view * model * vec4(pos, 1.0);
	vs_out.lifeTime = lifeTime;
}

