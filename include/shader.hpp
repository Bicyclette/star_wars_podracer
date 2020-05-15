#ifndef _SHADER_HPP_
#define _SHADER_HPP_

#include <GL/glew.h>
#include <iostream>
#include <string>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
	public:

		Shader(const std::string & vertex_shader_file, const std::string & fragment_shader_file, const std::string & geometry_shader_file = "../shaders/default/geometry.glsl");
		GLuint get_id() const;
		void set_int(const std::string & name, int v) const;
		void set_float(const std::string & name, float v) const;
		void set_vec3f(const std::string & name, glm::vec3 v) const;
		void set_Matrix(const std::string & name, glm::mat4 m) const;
		void set_texture(const std::string & texture_path, int tex_unit, const std::string & uniform_name, bool flip);
		void use() const;

	private:

		void compile(const char * vertex_shader_code, const char * fragment_shader_code, const char * geometry_shader_code);
		
		GLuint id;
};

#endif
