// Create GL window and context

#ifndef RENDER_INIT_HPP
#define RENDER_INIT_HPP

#include <string>
#include <vector>
#include <functional>

struct GLFWwindow;

class GLWindow 
{
	void updateTitle();

	static const int framesToAverage = 25;	
	GLFWwindow* window;
	int curFrame = 0;
	std::string title = "Initializing Partikel...";
	double fps = 0;
public:
	GLWindow(int width, int height);
	void clearBuffer();
	void swap();
	bool poll(); // return false if closed
	void setTitle(const std::string& title);

	std::vector<std::function<bool(int,int)> > keyHandlers;
};

#endif