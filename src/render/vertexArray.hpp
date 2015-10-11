// Vertex buffers

#ifndef RENDER_VERTEXARRAY_HPP
#define RENDER_VERTEXARRAY_HPP

#include <string>
#include <memory>

class VertexBuffer {
public:
	VertexBuffer();
	~VertexBuffer();
	void bind();

	unsigned int handle;
};

#endif