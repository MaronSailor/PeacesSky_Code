#include "window.hpp"
#include "glfwWindow.hpp"

namespace CustomEngine
{
	Window* Window::Create(const WindowProps& props)
	{
		return new GlfwWindow(props);
	}
}