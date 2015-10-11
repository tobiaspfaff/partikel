#include <iostream>
#include <memory>
#include <GL/glew.h>
#include "render/init.hpp"
#include "render/shader.hpp"

using namespace std;

extern const char* git_version_short;

int main() 
{
	cout << "Partikel " << git_version_short << endl;

	auto window = make_unique<GLWindow>("Partikel");

	auto program = make_unique<ShaderProgram>(
		make_shared<VertexShader>("triangle_test.vs"),
		make_shared<FragmentShader>("triangle_test.fs") );

	program->use();

	while(window->poll()) 
	{
		window->clearBuffer();
		glDrawArrays(GL_TRIANGLES, 0, 3);
		window->swap();
	}

	return 0;	
}