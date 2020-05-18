/**
 * \file
 * Somewhere over the rainbow, skies are blue
 * \author Mathias Velo
 */

#include "skybox.hpp"

// cube (front, right, back, left, top, bottom) => seen from (0,0,0) inside the cube
float cube[] = {
	-1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, 1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, -1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, 1.0f};

Skybox::Skybox(const std::string& vShader, const std::string& fShader, const std::string& gShader, const std::vector<std::string>& texture_paths)
{
	// create shader
	s = new Shader(vShader.c_str(), fShader.c_str(), gShader.c_str());

	// CubeMap
	glGenTextures(1, &cubeMapID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapID);

	int width, height, channels;
	GLenum format = GL_RGB;

	int tex_count = texture_paths.size();
	for(int i = 0; i < tex_count; i++)
	{
		unsigned char* img_data = stbi_load(texture_paths.at(i).c_str(), &width, &height, &channels, 0);
		if(img_data != nullptr)
		{
			if(channels == 3)
				format = GL_RGB;
			if(channels == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, img_data);
			stbi_image_free(img_data);
		}
		else
		{
			std::cerr << "Error: failed loading texture " << texture_paths.at(i)  << " for cubemap !" << std::endl;
			stbi_image_free(img_data);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	// VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), &cube, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);

	// Unbind VAO
	glBindVertexArray(0);
	// Unbind cubemap texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

Skybox::~Skybox()
{
	delete(s);
}

void Skybox::draw(glm::mat4 viewMatrix, glm::mat4 projMatrix)
{
	glBindVertexArray(VAO);
	glActiveTexture(GL_TEXTURE0);
	s->use();
	s->set_int("skybox", 0);
	s->set_Matrix("view", viewMatrix);
	s->set_Matrix("proj", projMatrix);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapID);
	glActiveTexture(GL_TEXTURE0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}
