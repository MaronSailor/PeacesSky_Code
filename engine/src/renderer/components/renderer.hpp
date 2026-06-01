#include "vertexArray.hpp"
#include "indexBuffer.hpp"
#include "shader.hpp"
#include "renderUnit.hpp"
#include "renderResourceHandler.hpp"
#include "../../sceneManager/scene.hpp"
#include "../../ecs/engineSceneInfo.hpp"

#include "imageBuffer.hpp"

namespace CustomEngine
{
	class Renderer : public RenderUnit
	{
		RenderResourceHandler m_ResourceHandler;
		Vec2 m_WindowAspect;
		ImageBuffer m_ImageBuffer;
		unsigned int m_OutputFbo = 0;

	private:
		void renderSceneObjs(EngineSceneInfo* sceneInfo);
		void renderSceneLighting(EngineSceneInfo* sceneInfo);

	public:
		Renderer();
		virtual ~Renderer();

		static Renderer* createRenderer();

		void draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;

		void renderScene(EngineSceneInfo* sceneInfo) override;
		void setWindowAspect(unsigned int width, unsigned int height) override;
		void setOutputFbo(unsigned int fbo) override { m_OutputFbo = fbo; }
	};
}