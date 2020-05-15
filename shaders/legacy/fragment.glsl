#version 330 core

out vec4 frag_color;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct SpotLight
{
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float inner_angle;
	float outer_angle;
	float constant;
	float linear;
	float quadratic;
};

uniform Material material;
uniform SpotLight p_light;

uniform vec3 viewer_pos;

in vec2 texCoords;
in vec3 normal;
in vec3 frag_pos;

void main()
{
	float distance = length(p_light.position - frag_pos);
	float decay = 1.0 / (p_light.constant + p_light.linear * distance + p_light.quadratic * (distance * distance));
	vec3 ray = normalize(p_light.position - frag_pos);
	float theta = dot(ray, normalize(p_light.direction));
	
	if(theta > p_light.inner_angle)
	{
		// ambient
		vec3 ambient = vec3(texture(material.diffuse, texCoords)) * p_light.ambient;

		// diffuse
		vec3 norm = normalize(normal);
		float diffuse_strength = max(dot(norm, ray), 0.0);
		vec3 diffuse = diffuse_strength * p_light.diffuse * vec3(texture(material.diffuse, texCoords));
		diffuse *= decay;

		// specular
		vec3 view_vector = normalize(viewer_pos - frag_pos);
		vec3 reflect_vector = reflect(-ray, norm);
		float spec = pow(max(dot(view_vector, reflect_vector), 0.0), material.shininess);
		vec3 specular = spec * vec3(texture(material.specular, texCoords)) * p_light.specular;
		specular *= decay;

		frag_color = vec4(ambient + diffuse + specular, 1.0);
	}
	else if(theta >= p_light.outer_angle && theta <= p_light.inner_angle)
	{
		// smooth
		float s = (abs(theta - p_light.outer_angle) / abs(p_light.inner_angle - p_light.outer_angle));
		
		// ambient
		vec3 ambient = vec3(texture(material.diffuse, texCoords)) * p_light.ambient;

		// diffuse
		vec3 norm = normalize(normal);
		float diffuse_strength = max(dot(norm, ray), 0.0);
		vec3 diffuse = diffuse_strength * p_light.diffuse * vec3(texture(material.diffuse, texCoords));
		diffuse *= decay * s;

		// specular
		vec3 view_vector = normalize(viewer_pos - frag_pos);
		vec3 reflect_vector = reflect(-ray, norm);
		float spec = pow(max(dot(view_vector, reflect_vector), 0.0), material.shininess);
		vec3 specular = spec * vec3(texture(material.specular, texCoords)) * p_light.specular;
		specular *= decay * s;

		frag_color = vec4(ambient + diffuse + specular, 1.0);
	}
	else
	{
		frag_color = vec4(vec3(texture(material.diffuse, texCoords)) * p_light.ambient * decay, 1.0);
	}
}
