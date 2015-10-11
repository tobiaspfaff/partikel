// Vertex buffers

#include "render/vertexArray.hpp"

using namespace std;

// Vertex array

VertexArray* VertexArray::bound = nullptr;

VertexArray::VertexArray()
{
	glGenVertexArrays(1, &handle);
}

VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &handle);
}

void VertexArray::bind() 
{
	if (this != bound) 
	{
		glBindVertexArray(handle);
		bound = this;
	}
}

// Vertex buffer

VertexBufferBase* VertexBufferBase::bound = nullptr;

VertexBufferBase::VertexBufferBase()
{
	glGenBuffers(1, &handle);
}

VertexBufferBase::~VertexBufferBase()
{
	glDeleteBuffers(1, &handle);
}

void VertexBufferBase::bind() 
{
	if (this != bound) 
	{
		glBindBuffer(GL_ARRAY_BUFFER, handle);
		bound = this;
	}
}  

