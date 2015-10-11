// Shader

#include "render/shader.hpp"
#include "tools/log.hpp"
#include <fstream>
#include <GL/glew.h>

using namespace std;

static unsigned int shaderLoad(const string& filename, GLenum type) 
{
	unsigned int handle = glCreateShader(type);
	
	// load file into buffer
	ifstream file(filename.c_str(), ios::in|ios::binary|ios::ate);
  	if (!file.is_open()) 
  	{
  		fatalError("Failed to load shader "+filename);
  	}
  	streampos size = file.tellg();
    char* buffer = new char [size];
    file.seekg(0, ios::beg);
    file.read(buffer, size);
    file.close();

    glShaderSource(handle, 1, &buffer, nullptr);
    glCompileShader(handle);
	
    delete[] buffer;

    return handle;
}

VertexShader::VertexShader(const string& filename) 
{
	handle = shaderLoad(filename, GL_VERTEX_SHADER);
}

FragmentShader::FragmentShader(const string& filename) 
{
	handle = shaderLoad(filename, GL_FRAGMENT_SHADER);
}

ShaderProgram::ShaderProgram(shared_ptr<VertexShader> vs, shared_ptr<FragmentShader> fs) :
	vs(vs), fs(fs)
{
	handle = glCreateProgram();
	glAttachShader(handle, vs->handle);
	glAttachShader(handle, fs->handle);
	
	glLinkProgram(handle);
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(handle);
}

void ShaderProgram::use() 
{
	glUseProgram(handle);
}