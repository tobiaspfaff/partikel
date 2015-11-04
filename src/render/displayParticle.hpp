// Debug display of simulation particles

#ifndef RENDER_DISPLAYPARTICLE_HPP
#define RENDER_DISPLAYPARTICLE_HPP

#include "sim/particle.hpp"
#include "render/shader.hpp"
#include "compute/computeMain.hpp"

class GLWindow;

struct LineVertex
{
	Vec4 color;
	Vec2 pos;	
};

struct DisplayPartInfo
{
	DynamicParticles* part;
	std::string name;
};

class DisplayParticle
{
public:
	DisplayParticle(CLQueue& queue, const Vec2& domainMin, const Vec2& domainMax, GLWindow& window);

	void attach(DynamicParticles* part, const std::string& name);
	void render();
	void compute();
protected:
	void changePart();
	bool keyHandler(int key, int mods);

	GLWindow& window;
	Vec2 domainMin;
	Vec2 domainMax;
	std::vector<DisplayPartInfo> displayPartList;
	SingleVertexArray<cl_float2> vbPart;
	SingleVertexArray<LineVertex> vbGridLines;
	std::unique_ptr<CLVertexBuffer<cl_float2> > clPart;
	CLKernel fillKernel;
	std::unique_ptr<ShaderProgram> particleShader, lineShader;
	int curSystem = -1;
};

#endif