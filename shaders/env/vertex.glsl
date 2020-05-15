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
	flat int shadows;
	vec4 frag_pos_sunlightSpace_env;
	vec4 frag_pos_sunlightSpace_pod;
	float visibility;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform int animation;
uniform int shadowPass;
uniform mat4 sunlightSpaceMatrix_env;
uniform mat4 sunlightSpaceMatrix_pod;

uniform int process_pod_shadowPass;

const float fog_density = 0.0025f;
const float gradient = 1.5f;

void main()
{
	mat4 norm_mat = inverse(transpose(model));
	vs_out.texCoords = vertex_texCoords;
	vs_out.frag_norm = vec3(norm_mat * vec4(vertex_normal, 1.0));
	vs_out.shadows = shadowPass;
	vs_out.frag_pos_sunlightSpace_env = sunlightSpaceMatrix_env * model * vec4(pos, 1.0);
	vs_out.frag_pos_sunlightSpace_pod = sunlightSpaceMatrix_pod * model * vec4(pos, 1.0);
	vs_out.visibility = 1.0f;

	float dist_to_camera;

	if(shadowPass == 0)
	{
		dist_to_camera = length(vec3(view * model * vec4(pos, 1.0)));
		vs_out.visibility = exp(-pow(dist_to_camera * fog_density, gradient));
		if(animation == 0)
		{
			gl_Position = proj * view * model * vec4(pos, 1.0);
		}
		else if(animation == 1)
		{
			gl_Position = proj * view * model * vec4(pos, 1.0);
		}
	}
	else if(shadowPass == 1)
	{
        if(process_pod_shadowPass == 1)
		    gl_Position = sunlightSpaceMatrix_pod * model * vec4(pos, 1.0);
        else
		    gl_Position = sunlightSpaceMatrix_env * model * vec4(pos, 1.0);
	}
}

