// Vertex buffers

#include "render/vertexArray.hpp"

using namespace std;

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
	glBindVertexArray(handle);
}
  
