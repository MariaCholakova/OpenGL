#pragma once

#include <GL/glew.h>

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCal(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCal(const char* function, const char* file, int line);