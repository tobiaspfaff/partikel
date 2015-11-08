// Shader

#include "render/opengl.hpp"
#include "render/shader.hpp"
#include "tools/log.hpp"
#include <fstream>
#include <iostream>

using namespace std;

static unsigned int shaderLoad(const string& filename, GLenum type, const string& name) 
{
	string filePath = "shader/" + filename;
	unsigned int handle = glCreateShader(type);
	
	// load file into buffer
	ifstream file(filePath.c_str(), ios::in|ios::binary|ios::ate);
  	if (!file.is_open()) 
  	{
  		fatalError("Failed to load shader "+filename);
  	}
  	size_t size = file.tellg();
    char* buffer = new char [size+1];
    file.seekg(0, ios::beg);
    file.read(buffer, size);
    file.close();
    buffer[size] = '\0';
    
    glShaderSource(handle, 1, (const GLchar**)&buffer, nullptr);
    glCompileShader(handle);
    int params = -1;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &params);
    if (params != GL_TRUE) 
    {
    	string logBuffer(2048, '\0');
		int actualLength = 0;
		glGetShaderInfoLog (handle, logBuffer.size(), &actualLength, &logBuffer[0]);
		fatalError(name + " shader '" + filename +"' compile failed:\n" + logBuffer);
    }
	
    delete[] buffer;

    return handle;
}

VertexShader::VertexShader(const string& filename) 
{
	handle = shaderLoad(filename, GL_VERTEX_SHADER, "Vertex");
}

FragmentShader::FragmentShader(const string& filename) 
{
	handle = shaderLoad(filename, GL_FRAGMENT_SHADER, "Fragment");
}

GeometryShader::GeometryShader(const string& filename) 
{
	handle = shaderLoad(filename, GL_GEOMETRY_SHADER, "Geometry");
}

ShaderProgram::ShaderProgram(const string& vertShader, 
							 const string& geomShader,
							 const string& fragShader)
{
	handle = glCreateProgram();
	
	VertexShader vs(vertShader);
	glAttachShader(handle, vs.handle);

	FragmentShader fs(fragShader);
	glAttachShader(handle, fs.handle);

	if (!geomShader.empty())
	{
		GeometryShader gs(geomShader);
		glAttachShader(handle, gs.handle);	
	}
	glLinkProgram(handle);

	int params = -1;
    glGetProgramiv(handle, GL_LINK_STATUS, &params);
    if (params != GL_TRUE) 
    {
    	string logBuffer(2048, '\0');
		int actualLength = 0;
		glGetProgramInfoLog(handle, logBuffer.size(), &actualLength, &logBuffer[0]);
		fatalError("Vertex program link failed:\n" + logBuffer);
    }
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(handle);
}

void ShaderProgram::use() 
{
	glUseProgram(handle);
}

int ShaderProgram::uniformIndex(const char* name)
{
	return glGetUniformLocation(handle, name);
}

template<>
void ShaderArgument<int>::set(const int& v)
{
	glProgramUniform1i(program, handle, v);
};

template<>
void ShaderArgument<float>::set(const float& v)
{
	glProgramUniform1f(program, handle, v);
};

template<>
void ShaderArgument<Vec2>::set(const Vec2& v)
{
	glProgramUniform2f(program, handle, v.x, v.y);
};
