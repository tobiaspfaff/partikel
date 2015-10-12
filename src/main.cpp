#include <iostream>
#include <memory>
#include <vector>
#include <GL/glew.h>
#include "render/colors.hpp"
#include "render/init.hpp"
#include "tools/vectors.hpp"
#include "render/shader.hpp"
#include "render/texture.hpp"
#include "render/vertexArray.hpp"

using namespace std;

extern const char* git_version_short;

struct VertexStruct {
	Vec2 pos;
	Vec4 color;
};

int main() 
{
	cout << "Partikel " << git_version_short << endl;

	auto window = make_unique<GLWindow>("Partikel", 1000, 1000);

	auto program = make_unique<ShaderProgram>(
		make_shared<VertexShader>("triangle_test.vs"),
		make_shared<FragmentShader>("triangle_test.fs"),
		make_shared<GeometryShader>("triangle_test.gs") );

	auto vao = make_unique<SingleVertexArray<VertexStruct> >();
	vao->defineAttrib(0, GL_FLOAT, 2, offsetof(VertexStruct, pos));
    vao->defineAttrib(1, GL_FLOAT, 4, offsetof(VertexStruct, color));
    
	Vec2 screen(1920, 1080);

	program->use();
	program->setUniform(program->uniform("scale"), Vec2(2/screen.x, 2/screen.y));
	vao->bind();

	auto tex = make_unique<Texture>("star02.png");

	const int N = 10000;
	vector<VertexStruct> data(N);
	
	float time = 0;
	while(window->poll()) 
	{
		const float freq_time = 0.02;
		const float freq_part = 0.01;
		for (int i=0; i<N; i++) {			
			const float phi = freq_time * time + i * freq_part;
			Vec2 circle = Vec2(sin(phi), cos(phi));
			float mod = fmod(i+time,1000.0) / 1000.0f;
			circle *= (float)sin(mod * M_PI * 2);
			data[i].pos = elmult(screen, (circle + Vec2(1))) * 0.5f;		
			Vec3 rgb = hsv2rgb(Vec3(mod, 1, 1));
			data[i].color = Vec4(rgb.x, rgb.y, rgb.z, i/(float)N);
		}
		vao->buffer.setData(&data[0], N);

		window->clearBuffer();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_POINTS, 0, N);
		window->swap();

		time += 1;
	}

	return 0;	
}