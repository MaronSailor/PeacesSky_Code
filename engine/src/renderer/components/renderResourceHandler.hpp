#include "texture.hpp"
#include "vertexBuffer.hpp"
#include "indexBuffer.hpp"
#include "vertexArray.hpp"
#include "vertexBufferLayout.hpp"
#include "shader.hpp"

#include "../../resourceLoader/meshData.hpp"
#include "../../resourceLoader/materialData.hpp"
#include "../../ecs/components/renderComponent.hpp"
#include "shadowMap.hpp"

#include <unordered_map>

class RenderResourceHandler
{
	std::vector<Texture> m_Textures;
	std::vector<IndexBuffer> m_IndexBuffers;
	std::vector<VertexBuffer> m_VertexBuffers;
	std::vector<Shader> m_Shaders;

	VertexBufferLayout m_Layout;
	VertexArray m_VertexArray;

	ShadowMap m_ShadowMap;

	std::unordered_map<MaterialData*, unsigned int> m_TextureTable;
	std::unordered_map<MeshData*, unsigned int> m_MeshTable;
	std::unordered_map<MeshData*, uint32_t> m_MeshRevisions;

	void updateMesh(MeshData& mesh, unsigned int index);

public:
	RenderResourceHandler();

	~RenderResourceHandler();

	unsigned int bindComponent(RenderComponent& component, Shader& shader);
	void bindTexture(RenderComponent& component, Shader& shader);
	unsigned int bindQuad();
	unsigned int createMaterial(MaterialData& material);
	unsigned int createMesh(MeshData& mesh);
	Shader& getShader(unsigned int index);
	ShadowMap& getShadowMap();
	unsigned int loadShader(std::string vertexShaderPath, std::string fragmentShaderPath);
};