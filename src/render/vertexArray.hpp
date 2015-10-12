// Vertex buffers

#ifndef RENDER_VERTEXARRAY_HPP
#define RENDER_VERTEXARRAY_HPP

#include <GL/glew.h>
#include <string>
#include <memory>

class VertexBufferBase
{
public:
	VertexBufferBase();
	~VertexBufferBase();
	void bind();

	unsigned int handle;	

protected:
	static VertexBufferBase* bound;	
};

template<class T>
class VertexBuffer : public VertexBufferBase
{
public:
	VertexBuffer() : VertexBufferBase() {}
	void setData(const T* data, int num);
};

class VertexArray
{
public:
	VertexArray();
	~VertexArray();
	void bind();

	unsigned int handle;

protected:
	static VertexArray* bound;	
};

template<class T>
class SingleVertexArray : public VertexArray
{
public:
	void defineAttrib(int index, GLenum type, int elements, size_t offset);

	VertexBuffer<T> buffer;	
};


// ------------------------------------
// IMPLEMENTATION
// ------------------------------------

template<class T>
void VertexBuffer<T>::setData(const T* data, int num) 
{
	bind();
	glBufferData(GL_ARRAY_BUFFER, num * sizeof(T), nullptr, GL_STREAM_DRAW); // orphaning
	glBufferData(GL_ARRAY_BUFFER, num * sizeof(T), data, GL_STREAM_DRAW);
}

template<class T>
void SingleVertexArray<T>::defineAttrib(int index, GLenum type, int elements, size_t offset) 
{
	bind();
	buffer.bind();
	glVertexAttribPointer(index, elements, type, GL_FALSE, sizeof(T), (void*) offset);
	glEnableVertexAttribArray(index);
}

#endif