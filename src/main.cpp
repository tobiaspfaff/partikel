#include <iostream>
#include <memory>
#include <vector>
#include <GL/glew.h>
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

struct VertexStruct {
	cl_float4 color;
	cl_float2 pos;	
};

int main() 
{
	const int N = 10000;
	cout << "Partikel " << git_version_short << endl;

	auto window = make_unique<GLWindow>("Partikel", 1000, 1000);

	auto vao = make_unique<SingleVertexArray<VertexStruct> >();
	vao->defineAttrib(0, GL_FLOAT, 4, offsetof(VertexStruct, color));
    vao->defineAttrib(1, GL_FLOAT, 2, offsetof(VertexStruct, pos));
    vector<VertexStruct> data(N);
	vao->buffer.setData(&data[0], N);

	auto cl = make_unique<CLMaster>();
	auto clBuf = make_unique<CLVertexBuffer<VertexStruct> >(*cl, *vao);
	auto queue = cl->gpuQueue;

	CLKernel kernel(*cl, "hello.cl", "hello");
	kernel.setArg(0, clBuf->handle);

	auto program = make_unique<ShaderProgram>(
		make_shared<VertexShader>("triangle_test.vs"),
		make_shared<FragmentShader>("triangle_test.fs"),
		make_shared<GeometryShader>("triangle_test.gs") );
	
	Vec2 screen(1920, 1080);

	program->use();
	program->setUniform(program->uniform("scale"), Vec2(2/screen.x, 2/screen.y));
	vao->bind();

	auto tex = make_unique<Texture>("star02.png");

	float time = 0;
	while(window->poll()) 
	{
		glFinish();
		clBuf->acquire(queue);
		kernel.setArg(1, time);
		kernel.setArg(2, screen);
		kernel.setArg(3, N);
		kernel.enqueue(queue, N);

		/*clBuf->read(cl->gpuQueue, data);
		for (int i=0;i<N;i++)
			cout << "read " << i << ": pos " <<  data[i].pos.s[0] << "," << data[i].pos.s[1] << " color " << data[i].color.s[0] << "," << data[i].color.s[1] << "," << data[i].color.s[2] << "," << data[i].color.s[3] << endl;
*/
		clBuf->release(queue);
		clFinish(queue);

		vao->bind();
		window->clearBuffer();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_POINTS, 0, N);
		window->swap();

		time += 1;
	}

	return 0;	
}