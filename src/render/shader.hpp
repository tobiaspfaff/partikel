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

template<class T>
struct ShaderArgument
{
	ShaderArgument() {}
	ShaderArgument(int program, int index) : program(program), handle(index) {}
	void set(const T& value);

	int program = -1;
	int handle = -1;
};

class ShaderProgram 
{
public:
	ShaderProgram(const std::string& vertShader,
				  const std::string& geomShader,
		          const std::string& fragShader);
	~ShaderProgram();

	template <class T> ShaderArgument<T> arg(const char* name);
	int uniformIndex(const char* name);
	void use();
	
	unsigned int handle;
	VertexShader* vs;
	FragmentShader* fs;
	GeometryShader* gs = nullptr;
};



// ------------------------------------
// IMPLEMENTATION
// ------------------------------------

template<class T>
ShaderArgument<T> ShaderProgram::arg(const char* name)
{
	return ShaderArgument<T> (handle, uniformIndex(name));
}



#endif