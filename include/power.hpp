#ifndef _POWER_HPP_
#define _POWER_HPP_

#include <iostream>
#include <vector>
#include <map>
#include <random>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <array>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"
#include "animation.hpp"
#include "object.hpp"
#include "joint.hpp"

#define MAX_BOLT_NODES 256

struct BoltNode
{
	glm::vec3 position;
	glm::vec2 texCoords;

	BoltNode(glm::vec3 pos, glm::vec2 tCoords)
	{
		position = pos;
		texCoords = tCoords;
	}
};

class Power
{
	public:

		Power(std::vector<glm::vec3> co_left, std::vector<glm::vec3> co_right);
		float get_random(int choice);
		void draw(Shader* power_shader);

	private:

		void create_bolt(int jitter);

		GLuint VAO;
		GLuint VBO;
		struct BoltNode* start;
		struct BoltNode* end;
		float jitter1;
		float jitter2;
		float thickness;
		std::vector<float> offset_x;
		std::vector<struct BoltNode> bolt;
		std::mt19937 rng;
		std::uniform_real_distribution<float> dis1;
		std::uniform_real_distribution<float> dis2;
		std::uniform_real_distribution<float> dis3;
};

#endif
