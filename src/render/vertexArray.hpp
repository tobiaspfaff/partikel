// Vertex buffers

#ifndef RENDER_VERTEXARRAY_HPP
#define RENDER_VERTEXARRAY_HPP

#include <GL/glew.h>
#include <string>
#include <memory>

class VertexBuffer
{
public:
	VertexBuffer();
	virtual ~VertexBuffer();
	void bind();
	void setData(void* data, size_t size);
	inline void setSize(size_t nsize) { setData(nullptr, nsize); };

	unsigned int handle;	
	size_t size = 0;

protected:
	static VertexBuffer* bound;	
};

class VertexArray
{
public:
	VertexArray();
	virtual ~VertexArray();
	void bind();

	unsigned int handle;

protected:
	static VertexArray* bound;	
};

class SingleVertexArray : public VertexArray
{
public:
	void defineAttrib(int index, GLenum type, int elements, int stride, size_t offset);

	VertexBuffer buffer;	
};

#endif