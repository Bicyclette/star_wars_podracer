#version 330 core

in GS_OUT
{
	vec2 tCoords;
} fs_in;

uniform sampler2D env_texture;
uniform sampler2D smoke_texture;

uniform sampler2D envDepth_texture;
uniform sampler2D smokeDepth_texture;

uniform sampler2D envMotionBlur;
uniform sampler2D smokeMotionBlur;

uniform sampler2D podracer_texture;
uniform sampler2D podracer_bright_texture;
uniform sampler2D smoke_bright_texture;
uniform sampler2D depthPod_texture;

uniform sampler2D gaussian_blur_smoke_bright_texture;

uniform int check_render_pass;
uniform int r;

out vec4 color;

float linearizeDepth(float depth)
{
	float z = (depth * 2.0) - 1.0;
	return (2.0 * 1000.0) / (1001.0 - z * 999.0);
}

void mix_render_passes()
{
	float env_depth = linearizeDepth(texture(envDepth_texture, fs_in.tCoords).r);
	env_depth /= 1000.0;
	float smoke_depth = linearizeDepth(texture(smokeDepth_texture, fs_in.tCoords).r);
	smoke_depth /= 1000.0;
	float pod_depth = linearizeDepth(texture(depthPod_texture, fs_in.tCoords).r);
	pod_depth /= 1000.0;

	vec4 env_color = texture(envMotionBlur, fs_in.tCoords);
	vec4 smoke_color = texture(smokeMotionBlur, fs_in.tCoords) + texture(gaussian_blur_smoke_bright_texture, fs_in.tCoords);
	vec4 pod_color = texture(podracer_texture, fs_in.tCoords);

	if((pod_depth < smoke_depth) && (pod_depth < env_depth))
	{
		color = mix(env_color, pod_color, pod_color.a);
	}
	else if((smoke_depth < pod_depth) && (smoke_depth < env_depth) && (pod_depth < env_depth))
	{
		color = mix(pod_color, smoke_color, smoke_color.a);
	}
	else if((smoke_depth < pod_depth) && (smoke_depth < env_depth) && (pod_depth > env_depth))
	{
		color = mix(env_color, smoke_color, smoke_color.a);
	}
	else
	{
		color = env_color;
	}
}

void main()
{
	if(check_render_pass == 1)
	{
		if(r == 0)
			color = texture(env_texture, fs_in.tCoords);
	
		else if(r == 1)
			color = texture(smoke_texture, fs_in.tCoords);
	
		else if(r == 2)
			color = texture(podracer_texture, fs_in.tCoords);
	
		else if(r == 3)
			color = texture(podracer_bright_texture, fs_in.tCoords);
	
		else if(r == 4)
			color = texture(smoke_bright_texture, fs_in.tCoords);
	
		else if(r == 5)
			color = texture(gaussian_blur_smoke_bright_texture, fs_in.tCoords);

		else if(r == 6)
		{
			float env_depth = linearizeDepth(texture(envDepth_texture, fs_in.tCoords).r);
			env_depth /= 1000.0;
			color = vec4(vec3(env_depth), 1.0);
		}

		else if(r == 7)
		{
			float smoke_depth = linearizeDepth(texture(smokeDepth_texture, fs_in.tCoords).r);
			smoke_depth /= 1000.0;
			color = vec4(vec3(smoke_depth), 1.0);
		}

		else if(r == 8)
		{
			float pod_depth = linearizeDepth(texture(depthPod_texture, fs_in.tCoords).r);
			pod_depth /= 1000.0;
			color = vec4(vec3(pod_depth), 1.0);
		}

		else if(r == 9)
			color = texture(envMotionBlur, fs_in.tCoords);
	
		else if(r == 10)
			color = texture(smokeMotionBlur, fs_in.tCoords);
		else if(r == 11)
		{
			mix_render_passes();
		}
	}
	else
	{
		mix_render_passes();
	}
}

