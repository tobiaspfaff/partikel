// Create GL window and context

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "render/window.hpp"
#include "tools/log.hpp"

using namespace std;

void error_callback(int error, const char* description) 
{
 	fatalError(description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

GLWindow::GLWindow(const string& name, int width, int height) 
{
	// Initialize GLFW
	if (!glfwInit()) 
	{
		fatalError("Can't initialize GLFW");
	}
	glfwSetErrorCallback(error_callback);

	// Use OpenGL 4.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	// Indicate we only want the newest core profile
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create the window and context
	window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
	if(!window) 
	{
		fatalError("Can't create GLFW window");
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;

	// Makes sure all extensions will be exposed in GLEW and initialize GLEW.
	glewExperimental = GL_TRUE;
	glewInit();
}

void GLWindow::clearBuffer() 
{
	static const GLfloat green[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glClearBufferfv(GL_COLOR, 0, green);
}

void GLWindow::swap() 
{
	glfwSwapBuffers(window);    
}

bool GLWindow::poll()
{
	glfwPollEvents();
	return !glfwWindowShouldClose(window);
}
