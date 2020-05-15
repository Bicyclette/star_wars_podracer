#version 330 core

out vec4 frag_color;

in GS_OUT
{
	vec2 texCoords;
	vec3 frag_norm;
	flat int shadows;
	vec4 frag_pos_sunlightSpace_env;
	vec4 frag_pos_sunlightSpace_pod;
	float visibility;
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

uniform int platform_attenuation;
uniform Material material;
uniform Sun sun;
uniform int cast_shadows;
uniform sampler2D envShadowMap;
uniform sampler2D podShadowMap;

float env_shadowFactor(vec4 frag)
{
	vec3 frag_ndc = frag.xyz / frag.w;
	frag_ndc = (frag_ndc + 1.0) / 2.0;
	float closest_frag = texture(envShadowMap, frag_ndc.xy).r;
	float currentDepth = frag_ndc.z;
	
	float shadow = 0.0;
    float bias = 0.00085;

	vec2 texelSize = 1.0 / textureSize(envShadowMap, 0);

	for(int x = -2; x <= 2; x++)
	{
		for(int y = -2; y <= 2; y++)
		{
			float pcfDepth = texture(envShadowMap, frag_ndc.xy + vec2(x, y) * texelSize).r;
			shadow += ((currentDepth - bias) > pcfDepth) ? 1.0 : 0.0;
		}
	}

	shadow = shadow /= 25.0;

	if(currentDepth > 1.0)
		shadow = 0.0;

	return shadow;
}

float pod_shadowFactor(vec4 frag)
{
	vec3 frag_ndc = frag.xyz / frag.w;
	frag_ndc = (frag_ndc + 1.0) / 2.0;
	float closest_frag = texture(podShadowMap, frag_ndc.xy).r;
	float currentDepth = frag_ndc.z;
	
	float shadow = 0.0;
	float bias = 0.0002;

	vec2 texelSize = 1.0 / textureSize(podShadowMap, 0);

	for(int x = -2; x <= 2; x++)
	{
		for(int y = -2; y <= 2; y++)
		{
			float pcfDepth = texture(podShadowMap, frag_ndc.xy + vec2(x, y) * texelSize).r;
			shadow += ((currentDepth - bias) > pcfDepth) ? 1.0 : 0.0;
		}
	}

	shadow = shadow /= 25.0;

	if(currentDepth > 1.0)
		shadow = 0.0;

	return shadow;
}

const float exposure = 0.2;

void main()
{
	if(fs_in.shadows == 0)
	{
		// color
		vec4 sky = vec4(0.243, 0.396, 0.549, 1.0);
		vec4 tatooine_color = vec4(1.0, 0.635, 0.188, 1.0);
		vec4 base_color = texture(material.diffuse_1, fs_in.texCoords);

		// ambient
		vec3 ambient = sun.color * 0.03;

		// diffuse
		float diff_str = max(dot(-sun.dir, fs_in.frag_norm), 0);
		vec3 diffuse = diff_str * sun.color * 0.65;

		// final color
		if(cast_shadows == 1)
		{
			float env_shadow = env_shadowFactor(fs_in.frag_pos_sunlightSpace_env);
			float pod_shadow = pod_shadowFactor(fs_in.frag_pos_sunlightSpace_pod);
            float shadow = (env_shadow > pod_shadow) ? env_shadow : pod_shadow;
			vec3 color = (((1.0 - shadow) * diffuse) + ambient) * vec3(base_color);
			if(platform_attenuation == 1)
				frag_color = vec4(color * 0.85, 1.0);
			else
				frag_color = vec4(color, 1.0);
			frag_color = mix(sky, frag_color, fs_in.visibility);

			// reinhard tone mapping
			//frag_color = vec4(frag_color.rgb / (frag_color.rgb + vec3(1.0)), 1.0);
			
			// exposure tone mapping
			frag_color = vec4(vec3(1.0) - exp(-frag_color.rgb * exposure), 1.0);
		}
		else if(cast_shadows == 0)
		{
			vec3 color = (diffuse + ambient) * vec3(base_color);
			if(platform_attenuation == 1)
				frag_color = vec4(color * 0.85, 1.0);
			else
				frag_color = vec4(color, 1.0);
			frag_color = mix(sky, frag_color, fs_in.visibility);
			
			// reinhard tone mapping
			//frag_color = vec4(frag_color.rgb / (frag_color.rgb + vec3(1.0)), 1.0);
			
			// exposure tone mapping
			frag_color = vec4(vec3(1.0) - exp(-frag_color.rgb * exposure), 1.0);
		}
	}
}

