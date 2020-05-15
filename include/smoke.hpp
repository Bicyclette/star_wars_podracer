#ifndef _PARTICLE_HPP_
#define _PARTICLE_HPP_

#include <iostream>
#include <vector>
#include <random>
#include <map>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"
#include "joint.hpp"
#include "animation.hpp"
#include "object.hpp"

#define MAX_PARTICLES 240

struct Particle
{
	glm::vec3 position;
	float lifeTime;
	glm::vec3 direction;

	Particle(glm::vec3 pos, glm::vec3 dir)
	{
		position = pos;
		lifeTime = 0.0f;
		direction = dir;
	}
};

class Smoke
{
	public:

		Smoke(std::vector<glm::vec3> sources, glm::vec3 sources_dir);
		void set_init_dir(glm::vec3 dir);
		void draw(double delta, Shader* smoke_shader);

	private:

		GLuint VAO;
		GLuint VBO;
		
		//initial particles sources and their direction
		glm::vec3 s1;
		glm::vec3 s2;
		glm::vec3 s3;
		glm::vec3 direction;
		
		// set of particles
		std::vector<struct Particle> particles;
		std::random_device rd;
		std::mt19937 gen;
		std::uniform_real_distribution<float> dis;

		// smoke attributes
		float velocity;
		float dissolve;
		float jitter;
};

#endif
