// Shader

#ifndef RENDER_SHADER_HPP
#define RENDER_SHADER_HPP

#include <string>
#include <memory>
#include "tools/vectors.hpp"

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

class GeometryShader : public Shader 
{
public:
	GeometryShader(const std::string& filename);	
};

class ShaderProgram 
{
public:
	ShaderProgram(std::shared_ptr<VertexShader> vs, 
				  std::shared_ptr<FragmentShader> fs,
				  std::shared_ptr<GeometryShader> gs = nullptr);
	~ShaderProgram();
	void use();
	int uniform(const char* name);
	void setUniform(int idx, int i);
	void setUniform(int idx, float v);
	void setUniform(int idx, const Vec2& v);
	void setUniform(int idx, const Vec3& v);
	void setUniform(int idx, const Vec4& v);
	void setUniform(int idx, const Vec2i& v);
	void setUniform(int idx, const Vec3i& v);
	void setUniform(int idx, const Vec4i& v);

	unsigned int handle;
	std::shared_ptr<VertexShader> vs;
	std::shared_ptr<FragmentShader> fs;
	std::shared_ptr<GeometryShader> gs;
};

#endif