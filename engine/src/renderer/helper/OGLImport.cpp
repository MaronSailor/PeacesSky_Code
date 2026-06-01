#include "OGLImport.hpp"

#include <iostream>

void oglFlushErrors()
{
	while (glGetError() != GL_NO_ERROR);
}

bool oglShowErrors()
{
	while (GLenum error = glGetError())
	{
		std::cout << "[OpenGL Error] (" << error << ")\n";
		return false;
	}
	return true;
}