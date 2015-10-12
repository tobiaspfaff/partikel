// Texture and image functions

#ifndef RENDER_TEXTURE_HPP
#define RENDER_TEXTURE_HPP

#include <string>

class Texture {
public:
	Texture(const std::string& filename);
	void bind();

	int width {0};
	int height {0};
	int ncomp {0};
	unsigned handle;
};

#endif