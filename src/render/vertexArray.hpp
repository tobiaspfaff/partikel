// Vertex buffers

#ifndef RENDER_VERTEXARRAY_HPP
#define RENDER_VERTEXARRAY_HPP

#include <string>
#include <memory>

class VertexArray
{
public:
	VertexArray();
	~VertexArray();
	void bind();

	unsigned int handle;
};

#endif