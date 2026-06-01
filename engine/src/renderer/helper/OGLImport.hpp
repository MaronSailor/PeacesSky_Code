#pragma once

#include <Gl/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"


#ifndef DEBUG_OGL
#define DEBUG_OGL 1
#endif

#define ASSERT(x) if (!(x)) __debugbreak();

#if DEBUG_OGL
#define checkOGL(OGLfunc)\
	oglFlushErrors();\
	OGLfunc;\
	ASSERT(oglShowErrors());
#else
#define checkOGL(OGLfunc) OGLfunc;
#endif


void oglFlushErrors();

bool oglShowErrors();