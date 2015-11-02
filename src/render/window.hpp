// Create GL window and context

#ifndef RENDER_INIT_HPP
#define RENDER_INIT_HPP

#include <string>
#include <vector>
#include <functional>

struct GLFWwindow;

class GLWindow 
{
	GLFWwindow* window;
public:
	GLWindow(const std::string& name, int width, int height);
	void clearBuffer();
	void swap();
	bool poll(); // return false if closed
	void setTitle(const std::string& title);

	std::vector<std::function<bool(int,int)> > keyHandlers;
};

#endif