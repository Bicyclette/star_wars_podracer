#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 tex;

out VS_OUT
{
	vec2 texCoords;
	vec3 frag_norm;
	vec3 frag_pos;
	flat int shadows;
	vec4 frag_pos_sunlightSpace_env;
	vec4 frag_pos_sunlightSpace_pod;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform int shadowPass;
uniform mat4 sunlightSpaceMatrix_env;
uniform mat4 sunlightSpaceMatrix_pod;

uniform int process_pod_shadowPass;

void main()
{
	mat4 norm_mat;
	vs_out.texCoords = tex;
	vs_out.shadows = shadowPass;
	
	norm_mat = inverse(transpose(model));
	vs_out.frag_norm = vec3(norm_mat * vec4(normal, 1.0));
	vs_out.frag_pos = vec3(model * vec4(pos, 1.0));
	vs_out.frag_pos_sunlightSpace_env = sunlightSpaceMatrix_env * model * vec4(pos, 1.0);
	vs_out.frag_pos_sunlightSpace_pod = sunlightSpaceMatrix_pod * model * vec4(pos, 1.0);
		
	if(shadowPass == 0)
	{
		gl_Position = proj * view * model * vec4(pos, 1.0);
	}
	else if(shadowPass == 1)
	{
        if(process_pod_shadowPass == 1)
			gl_Position = sunlightSpaceMatrix_pod * model * vec4(pos, 1.0);
        else
			gl_Position = sunlightSpaceMatrix_env * model * vec4(pos, 1.0);
	}
}

