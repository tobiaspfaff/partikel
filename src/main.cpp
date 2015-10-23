#include <iostream>
#include <memory>
#include <vector>
#include "render/opengl.hpp"
#include "render/colors.hpp"
#include "render/window.hpp"
#include "tools/vectors.hpp"
#include "tools/log.hpp"
#include "compute/computeMain.hpp"
#include "render/shader.hpp"
#include "render/texture.hpp"
#include "render/vertexArray.hpp"

using namespace std;

extern const char* git_version_short;

int main() 
{
	cout << "Partikel " << git_version_short << endl;

	auto window = make_unique<GLWindow>("Partikel", 1000, 1000);

	auto vaoGrid = make_unique<SingleVertexArray<cl_float4> >();
	vaoGrid->defineAttrib(0, GL_FLOAT, 4, 0);
    //vao->defineAttrib(1, GL_FLOAT, 2, offsetof(VertexStruct, pos));
    
	// grid
	Vec2i gridSize(100,100);
	vector<cl_float4> data(gridSize.x * gridSize.y);
	for (int i=0,idx=0; i<gridSize.y; i++)
		for (int j=0; j<gridSize.x; j++,idx++)
			data[idx] = {{ (float)i/gridSize.x, (float)j/gridSize.y, 0.0f, 1.0f }};

	vaoGrid->buffer.setData(&data[0], data.size());

	CLQueue cpuQueue, gpuQueue;
	createQueues(cpuQueue, gpuQueue);
	auto& queue = gpuQueue;

	CLKernel kernel(queue, "hello.cl", "hello");
	auto clBuf = make_unique<CLVertexBuffer<cl_float4> >(queue, *vaoGrid);
	kernel.setArg(0, clBuf->handle);
	kernel.setArg(1, 0);
	kernel.setArg(2, gridSize);

	auto program = make_unique<ShaderProgram>(
		make_shared<VertexShader>("triangle_test.vs"),
		make_shared<FragmentShader>("triangle_test.fs"),
		make_shared<GeometryShader>("triangle_test.gs") );
	
	Vec2 screen(1920, 1080);

	program->use();
	program->setUniform(program->uniform("size"), gridSize);
	program->setUniform(program->uniform("scale"), Vec2(2.0f/gridSize.x, 2.0f/gridSize.y));
	vaoGrid->bind();

	auto tex = make_unique<Texture>("star02.png");

	float time = 0;
	while(window->poll()) 
	{
		glFinish();
		clBuf->acquire();
		kernel.setArg(1, time);
		kernel.enqueue(gridSize.x * gridSize.y);

		clBuf->release();
		clFinish(queue.handle);

		vaoGrid->bind();
		window->clearBuffer();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_POINTS, 0, (GLsizei)data.size());
		window->swap();

		time += 0.01;
	}

	return 0;	
}