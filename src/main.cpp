#include <iostream>
#include <memory>
#include <GL/glew.h>
#include "render/init.hpp"
#include "render/shader.hpp"
#include "render/vertexArray.hpp"

using namespace std;

extern const char* git_version_short;

int main() 
{
	cout << "Partikel " << git_version_short << endl;

	auto window = make_unique<GLWindow>("Partikel");

	auto program = make_unique<ShaderProgram>(
		make_shared<VertexShader>("triangle_test.vs"),
		make_shared<FragmentShader>("triangle_test.fs") );

	auto vao = make_unique<VertexArray>();

	program->use();
	vao->bind();

	while(window->poll()) 
	{
		window->clearBuffer();
		glDrawArrays(GL_TRIANGLES, 0, 3);
		window->swap();
	}

	return 0;	
}