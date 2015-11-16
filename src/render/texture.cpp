// Texture and image functions

#include "render/opengl.hpp"
#include "render/texture.hpp"
#include "tools/log.hpp"
#include "dependencies/stb_image.h"

using namespace std;

Texture::Texture(const string& filename) 
{
	// load file using STBI
	const string filePath = "textures/" + filename;
	unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &ncomp, 0);
	if (!data)
	{
		fatalError("Failed to load image "+filename);
	}
	if (ncomp != 3 && ncomp != 4)
	{
		fatalError("Image " + filename + " has weird number of components. Only RBG and RBGA supported.");
	}

	// Pre-multiply alpha
	if (ncomp == 4 && 0) 
	{
		unsigned char* ptr = data;
		for (int i = 0; i < width*height; i++)
		{
			int alpha = ptr[3];
			ptr[0] = (alpha * (int)ptr[0]) / 255;
			ptr[1] = (alpha * (int)ptr[1]) / 255;
			ptr[2] = (alpha * (int)ptr[2]) / 255;
			ptr += 4;
		}
	}

	// create GL texture
	glGenTextures(1, &handle);
	bind();
	GLenum format = ncomp==3 ? GL_RGB : GL_RGBA;
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);
}

void Texture::bind() 
{
	glBindTexture(GL_TEXTURE_2D, handle);
}