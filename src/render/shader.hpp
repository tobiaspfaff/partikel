// Shader

#ifndef RENDER_SHADER_HPP
#define RENDER_SHADER_HPP

#include <string>
#include <memory>

class Shader 
{	
public:
	unsigned int handle;
};

class VertexShader : public Shader 
{
public:
	VertexShader(const std::string& filename);	
};

class FragmentShader : public Shader 
{
public:
	FragmentShader(const std::string& filename);	
};

class ShaderProgram 
{
public:
	ShaderProgram(std::shared_ptr<VertexShader> vs, std::shared_ptr<FragmentShader> fs);
	~ShaderProgram();
	void use();

	unsigned int handle;
	std::shared_ptr<VertexShader> vs;
	std::shared_ptr<FragmentShader> fs;
};

#endif