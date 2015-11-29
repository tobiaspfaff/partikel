#include "render/displayParticle.hpp"
#include "render/window.hpp"
#include <sstream>
#include <GLFW/glfw3.h>

using namespace std;

DisplayParticle::DisplayParticle(CLQueue& queue, const Domain& domain, GLWindow& window) :
	window(window), domainMin(toVec2(domain.offset)), domainMax(toVec2(domain.offset) + toVec2(domain.size)*domain.dx), queue(queue)
{
	// display buffers
	vaPart = make_unique<SingleVertexArray>();
	vaPart->buffer.setSize(1024);
	clPart = make_unique<CLVertexBuffer<float> >(queue, *vaPart);

	// shaders
	particleShader = make_unique<ShaderProgram>("particle.vert", "billboard_uv.geom", "tex_shade.frag");
	uniformScale = particleShader->arg<Vec2>("scale");
	uniformBillboardSize = particleShader->arg<float>("billboard_size");

	// register key handler
	window.keyHandlers.push_back(bind(&DisplayParticle::keyHandler, this, placeholders::_1, placeholders::_2));
}

void DisplayParticle::compute()
{
	if (curSystem < 0 && !displayPartList.empty())
	{
		curSystem = 0;
		changePart();
	}
	
	if (curSystem >= 0)
	{
		DynamicParticles* part = displayPartList[curSystem].part;
		
		if (part->size > 0)
		{
			size_t offset = part->size * sizeof(float);
			clPart->grow(part->size * 2);
			clPart->acquire();
			clTest(clEnqueueCopyBuffer(queue.handle, part->px.handle, clPart->handle, 0, 0, offset, 0, 0, 0), "buffer copy x");
			clTest(clEnqueueCopyBuffer(queue.handle, part->py.handle, clPart->handle, 0, offset, offset, 0, 0, 0), "buffer copy y");
			clPart->release();
		}
	}
}

void DisplayParticle::render()
{	
	// draw real grid
	if (curSystem >= 0)
	{
		DynamicParticles* part = displayPartList[curSystem].part;
		vaPart->bind();
		vaPart->defineAttrib(0, GL_FLOAT, 1, 0, 0);
		vaPart->defineAttrib(1, GL_FLOAT, 1, 0, sizeof(float) * part->size);
		particleShader->use();
		uniformBillboardSize.set(radius);
		uniformScale.set(Vec2(2.0f / domainMax.x, 2.0f / domainMax.y));
		glDrawArrays(GL_POINTS, 0, (GLsizei)part->size);
	}
}

bool DisplayParticle::keyHandler(int key, int mods)
{
	switch (key)
	{
	case GLFW_KEY_TAB:
		curSystem = (curSystem + 1) % displayPartList.size();
		changePart();
		return true;
	default:
		return false;
	}
}

void DisplayParticle::attach(DynamicParticles* part, const string& name)
{
	displayPartList.push_back({ part, name });
}

void DisplayParticle::changePart()
{
	string name = (curSystem >= 0) ? displayPartList[curSystem].name : "";
	
	// title text
	stringstream str;
	str << "Particle '" << name << "'"; 
	window.setTitle(str.str());
}