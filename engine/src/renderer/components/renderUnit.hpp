#include <vector>
#include "../../ecs/engineSceneInfo.hpp"
#include "../../sceneManager/scene.hpp"

namespace CustomEngine
{
	class RenderUnit
	{	
	public:
		static RenderUnit* createRenderer();
		
		virtual ~RenderUnit() = default;

		virtual void renderScene(EngineSceneInfo* sceneInfo) = 0;

		virtual void setWindowAspect(unsigned int width, unsigned int height) = 0;

		virtual void setOutputFbo(unsigned int fbo) {}
	};
}