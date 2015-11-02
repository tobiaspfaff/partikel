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

ShaderProgram::ShaderProgram(shared_ptr<VertexShader> vs, 
							 shared_ptr<FragmentShader> fs,
							 shared_ptr<GeometryShader> gs) :
	vs(vs), fs(fs), gs(gs)
{
	handle = glCreateProgram();
	glAttachShader(handle, vs->handle);
	glAttachShader(handle, fs->handle);
	if (gs)
	{
		glAttachShader(handle, gs->handle);	
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

void ShaderProgram::setUniform(int idx, int v)
{
	glProgramUniform1i(handle, idx, v);
}

void ShaderProgram::setUniform(int idx, float v)
{
	glProgramUniform1f(handle, idx, v);
}

void ShaderProgram::setUniform(int idx, const Vec2& v)
{
	glProgramUniform2f(handle, idx, v.x, v.y);
}

void ShaderProgram::setUniform(int idx, const Vec3& v)
{
	glProgramUniform3f(handle, idx, v.x, v.y, v.z);
}

void ShaderProgram::setUniform(int idx, const Vec4& v)
{
	glProgramUniform4f(handle, idx, v.x, v.y, v.z, v.w);
}

void ShaderProgram::setUniform(int idx, const Vec2i& v)
{
	glProgramUniform2i(handle, idx, v.x, v.y);
}

void ShaderProgram::setUniform(int idx, const Vec3i& v)
{
	glProgramUniform3i(handle, idx, v.x, v.y, v.z);
}

void ShaderProgram::setUniform(int idx, const Vec4i& v)
{
	glProgramUniform4i(handle, idx, v.x, v.y, v.z, v.w);
}

int ShaderProgram::uniform(const char* name) 
{
	return glGetUniformLocation(handle, name);
}