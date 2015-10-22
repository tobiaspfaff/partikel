// OpenGL paths on various OSes

#ifndef RENDER_OPENGL_HPP
#define RENDER_OPENGL_HPP

#if defined (__APPLE__)
	#include <GL/glew.h>
	#include "OpenGL/OpenGL.h"
#elif defined (_WIN32)
	//#define GLEW_STATIC
	#include <GL/glew.h>
#else // Linux

#endif

#endif