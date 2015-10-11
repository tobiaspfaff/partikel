// Create GL window and context

#include "render/init.hpp"
#include "tools/log.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
using namespace std;

GLWindow::GLWindow(const string& name) 
{
	// Initialize GLFW
	if (!glfwInit()) 
	{
		fatalError("Can't initialize GLFW");
	}

	/*
	// Set the error callback, as mentioned above.
	// glfwSetErrorCallback(error_callback);

	// Set up OpenGL options.
	// Use OpenGL verion 4.1,
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	// GLFW_OPENGL_FORWARD_COMPAT specifies whether the OpenGL context should be forward-compatible, i.e. one where all functionality deprecated in the requested version of OpenGL is removed.
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	// Indicate we only want the newest core profile, rather than using backwards compatible and deprecated features.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// Make the window resize-able.
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a window to put our stuff in.
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);

	// If the window fails to be created, print out the error, clean up GLFW and exit the program.
	if(!window) {
	  fprintf(stderr, "Failed to create GLFW window.");
	  glfwTerminate();
	  exit(EXIT_FAILURE);
	}

	// Use the window as the current context (everything that's drawn will be place in this window).
	glfwMakeContextCurrent(window);

	// Set the keyboard callback so that when we press ESC, it knows what to do.
	glfwSetKeyCallback(window, key_callback);

	printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));

	// Makes sure all extensions will be exposed in GLEW and initialize GLEW.
	glewExperimental = GL_TRUE;
	glewInit();
	*/
}