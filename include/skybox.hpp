#ifndef _SKYBOX_HPP_
#define _SKYBOX_HPP_

#include <iostream>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.hpp"
#include "shader.hpp"

#define VERTICES_COUNT 36

class Skybox
{
	public:

		Skybox(const std::string& vShader, const std::string& fShader, const std::string& gShader, const std::vector<std::string>& texture_paths);
		~Skybox();
		void draw(glm::mat4 viewMatrix, glm::mat4 projMatrix);

	private:

		GLuint cubeMapID;
		GLuint VAO;
		GLuint VBO;
		Shader* s;
};

#endif
