#include "render/vertexArray.hpp"

using namespace std;

// Vertex buffer

VertexBuffer* VertexBuffer::bound = nullptr;

VertexBuffer::VertexBuffer()
{
	glGenBuffers(1, &handle);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &handle);
}

void VertexBuffer::bind()
{
	if (this != bound)
	{
		glBindBuffer(GL_ARRAY_BUFFER, handle);
		bound = this;
	}
}

void VertexBuffer::setData(void* data, size_t size)
{
	this->size = size;
	bind();
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW); // orphaning
	if (data)
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STREAM_DRAW);
}

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

void SingleVertexArray::defineAttrib(int index, GLenum type, int elements, int stride, size_t offset)
{
	bind();
	buffer.bind();
	glVertexAttribPointer(index, elements, type, GL_FALSE, stride, (void*)offset);
	glEnableVertexAttribArray(index);
}

