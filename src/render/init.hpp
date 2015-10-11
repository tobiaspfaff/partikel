// Create GL window and context

#ifndef RENDER_INIT_HPP
#define RENDER_INIT_HPP

#include <string>

struct GLFWwindow;

class GLWindow 
{
	GLFWwindow* window;
public:
	GLWindow(const std::string& name);
	void clearBuffer();
	void swap();
	bool poll(); // return false if closed
};

#endif