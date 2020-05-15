#include "shader.hpp"
#include "stb_image.hpp"

Shader::Shader(const std::string & vertex_shader_file, const std::string & fragment_shader_file, const std::string & geometry_shader_file)
{
	int vShader_codeLength;
	int fShader_codeLength;
	int gShader_codeLength;

	char* vShaderCode;
	char* fShaderCode;
	char* gShaderCode;

	std::fstream v_shader_stream, f_shader_stream, g_shader_stream;
	v_shader_stream.open(vertex_shader_file, std::fstream::in);
	g_shader_stream.open(geometry_shader_file, std::fstream::in);
	f_shader_stream.open(fragment_shader_file, std::fstream::in);

	// compute code lengths
	v_shader_stream.seekg(0, v_shader_stream.end);
	vShader_codeLength = v_shader_stream.tellg();
	v_shader_stream.seekg(0, v_shader_stream.beg);
	
	g_shader_stream.seekg(0, g_shader_stream.end);
	gShader_codeLength = g_shader_stream.tellg();
	g_shader_stream.seekg(0, g_shader_stream.beg);
	
	f_shader_stream.seekg(0, f_shader_stream.end);
	fShader_codeLength = f_shader_stream.tellg();
	f_shader_stream.seekg(0, f_shader_stream.beg);

	// create arrays
	vShaderCode = new char[vShader_codeLength+1];
	gShaderCode = new char[gShader_codeLength+1];
	fShaderCode = new char[fShader_codeLength+1];

	vShaderCode[vShader_codeLength] = '\0';
	gShaderCode[gShader_codeLength] = '\0';
	fShaderCode[fShader_codeLength] = '\0';

	v_shader_stream.read(vShaderCode, vShader_codeLength);
	g_shader_stream.read(gShaderCode, gShader_codeLength);
	f_shader_stream.read(fShaderCode, fShader_codeLength);

	if(!v_shader_stream)
		std::cerr << "Error while trying to read the vertex shader file !" << std::endl;
	if(!g_shader_stream)
		std::cerr << "Error while trying to read the geometry shader file !" << std::endl;
	if(!f_shader_stream)
		std::cerr << "Error while trying to read the fragment shader file !" << std::endl;

	// Now compile the shaders, create the shader program and link
	compile(vShaderCode, fShaderCode, gShaderCode);

	delete[](vShaderCode);
	delete[](gShaderCode);
	delete[](fShaderCode);

	v_shader_stream.close();
	g_shader_stream.close();
	f_shader_stream.close();
}

void Shader::compile(const char * vertex_shader_code, const char * fragment_shader_code, const char * geometry_shader_code)
{
	GLuint vertex_shader, fragment_shader, geometry_shader, shader_program;
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	shader_program = glCreateProgram();

	glShaderSource(vertex_shader, 1, &vertex_shader_code, nullptr);	
	glCompileShader(vertex_shader);
	
	glShaderSource(geometry_shader, 1, &geometry_shader_code, nullptr);	
	glCompileShader(geometry_shader);
	
	glShaderSource(fragment_shader, 1, &fragment_shader_code, nullptr);	
	glCompileShader(fragment_shader);

	// Check for errors
	int success;
	int logLength;
	char* log;

	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE)
	{
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logLength);
		log = new char[logLength];
		glGetShaderInfoLog(vertex_shader, logLength, nullptr, log);
		std::cerr << "Error while compiling the vertex shader : " << log << std::endl;
		delete[](log);
		glDeleteShader(vertex_shader);
	}
	
	glGetShaderiv(geometry_shader, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE)
	{
		glGetShaderiv(geometry_shader, GL_INFO_LOG_LENGTH, &logLength);
		log = new char[logLength];
		glGetShaderInfoLog(geometry_shader, logLength, nullptr, log);
		std::cerr << "Error while compiling the geometry shader : " << log << std::endl;
		delete[](log);
		glDeleteShader(geometry_shader);
	}
	
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE)
	{
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logLength);
		log = new char[logLength];
		glGetShaderInfoLog(fragment_shader, logLength, nullptr, log);
		std::cerr << "Error while compiling the fragment shader : " << log << std::endl;
		delete[](log);
		glDeleteShader(fragment_shader);
	}

	// Final step
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, geometry_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if(success == GL_FALSE)
	{
		glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &logLength);
		log = new char[logLength];
		glGetProgramInfoLog(shader_program, logLength, nullptr, log);
		std::cerr << "Error while linking shaders into a program : " << log << std::endl;
		delete[](log);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}

	glDetachShader(shader_program, vertex_shader);
	glDetachShader(shader_program, geometry_shader);
	glDetachShader(shader_program, fragment_shader);

	id = shader_program;
}

GLuint Shader::get_id() const { return id; }

void Shader::set_int(const std::string & name, int v) const
{
	glUniform1i(glGetUniformLocation(id, name.c_str()), v);
}

void Shader::set_float(const std::string & name, float v) const
{
	glUniform1f(glGetUniformLocation(id, name.c_str()), v);
}

void Shader::set_vec3f(const std::string & name, glm::vec3 v) const
{
	glUniform3f(glGetUniformLocation(id, name.c_str()), v.x, v.y, v.z);
}

void Shader::set_Matrix(const std::string & name, glm::mat4 m) const
{
	glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::use() const { glUseProgram(id);}

void Shader::set_texture(const std::string & texture_path, int tex_unit, const std::string & uniform_name, bool flip)
{
	GLuint tex;
	GLenum format;
	int width, height, channels;
	stbi_set_flip_vertically_on_load(flip);
	unsigned char* data = stbi_load(texture_path.c_str(), &width, &height, &channels, 0);

	if(data)
	{
		if(channels == 3)
			format = GL_RGB;
		else if(channels == 4)
			format = GL_RGBA;

		glActiveTexture(GL_TEXTURE0 + tex_unit);
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		set_int(uniform_name.c_str(), tex_unit);
	}
	else
	{
		std::cerr << "Error while trying to load a texture !" << std::endl;
	}

	stbi_image_free(data);
}
