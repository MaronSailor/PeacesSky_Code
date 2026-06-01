#include "renderer.hpp"

namespace CustomEngine
{
	RenderUnit* RenderUnit::createRenderer()
	{
		return new Renderer();
	}
}