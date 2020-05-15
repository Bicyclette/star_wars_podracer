#version 330 core

out vec4 frag_color;

struct Material
{
	sampler2D diffuse_1;
    vec3 base_color;
    int has_textures;
    float shininess;
};

struct Sun
{
	vec3 dir;
	vec3 color;
};

uniform Material material;
uniform Sun sun;
uniform vec3 view_pos;
uniform sampler2D envShadowMap;
uniform sampler2D podShadowMap;

in GS_OUT
{
	vec2 texCoords;
	vec3 frag_norm;
	vec3 frag_pos;
	flat int shadows;
	vec4 frag_pos_sunlightSpace_env;
	vec4 frag_pos_sunlightSpace_pod;
} fs_in;

float env_shadowFactor(vec4 frag)
{
	vec3 frag_ndc = frag.xyz / frag.w;
	frag_ndc = (frag_ndc + 1.0) / 2.0;
	float closest_frag = texture(envShadowMap, frag_ndc.xy).r;
	float currentDepth = frag_ndc.z;
	
	float shadow = 0.0;
	float bias = 0.000005;

	shadow = ((currentDepth - bias) > closest_frag) ? 1.0 : 0.0;

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
	float bias = 0.00025;

	shadow = ((currentDepth - bias) > closest_frag) ? 1.0 : 0.0;

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

const float specs_exposure = 0.0625;
const float race_exposure = 0.125;

uniform int race;

void main()
{
	if(fs_in.shadows == 0)
	{
		// color
		vec4 base_color;
        if(material.has_textures == 1)
        {
            base_color = texture(material.diffuse_1, fs_in.texCoords);
        }
        else
        {
            base_color = vec4(material.base_color, 1.0);
        }

		// ambient
		vec3 ambient = sun.color * 0.125;

		// diffuse
		float diff_str = max(dot(-sun.dir, fs_in.frag_norm), 0.0);
		vec3 diffuse = diff_str * sun.color * 0.8;

		// specular
		vec3 view_dir = normalize(view_pos - fs_in.frag_pos);
		vec3 reflect_dir = reflect(sun.dir, fs_in.frag_norm);
        float spec_str = pow(max(dot(view_dir, reflect_dir), 0.0), 6);
		vec3 specular = sun.color * spec_str * 0.055;

		// final color
        float env_shadow = env_shadowFactor(fs_in.frag_pos_sunlightSpace_env);
        float pod_shadow = pod_shadowFactor(fs_in.frag_pos_sunlightSpace_pod);
        float shadow = (env_shadow > pod_shadow) ? env_shadow : pod_shadow;
        float ratio = max(1.0 - shadow, 0.0);
        vec3 color = ((ratio * (diffuse + specular)) + ambient) * vec3(base_color);
		frag_color = vec4(color, 1.0);

		// reinhard tone mapping
		//frag_color = vec4(frag_color.rgb / (frag_color.rgb + vec3(1.0)), 1.0);
		
		// exposure tone mapping
        if(race == 1)
		    frag_color = vec4(vec3(1.0) - exp(-frag_color.rgb * race_exposure), 1.0);
        else
		    frag_color = vec4(vec3(1.0) - exp(-frag_color.rgb * specs_exposure), 1.0);
	}
}

