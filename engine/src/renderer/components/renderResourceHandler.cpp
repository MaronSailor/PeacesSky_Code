#include "renderResourceHandler.hpp"
#include "../helper/helperFunctions.hpp"
#include "vertex.hpp"

#include <iostream>

RenderResourceHandler::RenderResourceHandler() : m_ShadowMap(500, 500)
{
	m_Layout.push<float>(3); // n position floats per vertex
	m_Layout.push<float>(2); // n texture coordinate floats per vertex
	m_Layout.push<float>(3); // n normal floats per vertex

	loadShader("../engine/src/renderer/shader/sceneShader.vert", "../engine/src/renderer/shader/sceneShader.frag");
	loadShader("../engine/src/renderer/shader/shadowMapShader.vert", "../engine/src/renderer/shader/shadowMapShader.frag");
	loadShader("../engine/src/renderer/shader/lightingAdditionShader.vert", "../engine/src/renderer/shader/lightingAdditionShader.frag");
	
	Vertex quadVertices[] = {
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{ 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
		{{-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
		{{ 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
	};

	unsigned int quadIndices[] = {
		0, 1, 2,
		2, 1, 3
	};

	m_VertexBuffers.emplace_back((const void*)(&quadVertices), sizeof(Vertex) * 4);
	m_IndexBuffers.emplace_back((const unsigned int*)(&quadIndices), 6);
}

RenderResourceHandler::~RenderResourceHandler()
{
	for (Texture& tex : m_Textures)
	{
		tex.destroy();
	}

	for (IndexBuffer& idx : m_IndexBuffers)
	{
		idx.destroy();
	}

	for (VertexBuffer& vtx : m_VertexBuffers)
	{
		vtx.destroy();
	}

	for (Shader& sdr : m_Shaders)
	{
		sdr.destroy();
	}

	m_ShadowMap.destroy();
}

unsigned int RenderResourceHandler::bindQuad()
{
	m_VertexArray.bind();;
	m_VertexArray.addBuffer(m_VertexBuffers[0], m_Layout);
	m_IndexBuffers[0].bind();
	return m_IndexBuffers[0].getCount();
}

unsigned int RenderResourceHandler::loadShader(std::string vertexShaderPath, std::string fragmentShaderPath)
{
	std::string vertexShader = readTextFile(vertexShaderPath);
	std::string fragmentShader = readTextFile(fragmentShaderPath);
	unsigned int index = m_Shaders.size();
	m_Shaders.emplace_back(vertexShader, fragmentShader);

	return index;
}

unsigned int RenderResourceHandler::createMaterial(MaterialData& material)
{
	unsigned int newIndex = m_Textures.size();
	m_Textures.emplace_back(material);
	m_TextureTable[&material] = newIndex;
	return newIndex;
}

unsigned int RenderResourceHandler::createMesh(MeshData& mesh)
{
	unsigned int newIndex = m_VertexBuffers.size();
	m_VertexBuffers.emplace_back(mesh.vertexBufferData.data(), mesh.vertexBufferData.size() * sizeof(Vertex));
	m_IndexBuffers.emplace_back(mesh.indexBufferData.data(), mesh.indexBufferData.size());
	m_MeshTable[&mesh] = newIndex;
	m_MeshRevisions[&mesh] = mesh.revision;
	return newIndex;
}

void RenderResourceHandler::updateMesh(MeshData& mesh, unsigned int index)
{
	m_VertexBuffers[index].setData(mesh.vertexBufferData.data(), mesh.vertexBufferData.size() * sizeof(Vertex));
	m_IndexBuffers[index].setData(mesh.indexBufferData.data(), static_cast<unsigned int>(mesh.indexBufferData.size()));
}

unsigned int RenderResourceHandler::bindComponent(RenderComponent& component, Shader& shader)
{
	unsigned int index = 0;
	shader.bind();
	m_VertexArray.bind();

	MeshData& mesh = component.getMeshData();

	if (m_MeshTable.find(&mesh) != m_MeshTable.end())
	{
		index = m_MeshTable.at(&mesh);
		if (m_MeshRevisions[&mesh] != mesh.revision)
		{
			updateMesh(mesh, index);
			m_MeshRevisions[&mesh] = mesh.revision;
		}
	}
	else
	{
		index = createMesh(mesh);
	}
	m_VertexArray.addBuffer(m_VertexBuffers[index], m_Layout);
	m_IndexBuffers[index].bind();
	
	return m_IndexBuffers[index].getCount();
}

void RenderResourceHandler::bindTexture(RenderComponent& component, Shader& shader)
{
	unsigned int index = 0;
	shader.bind();
	m_VertexArray.bind();
	if (m_TextureTable.find(&component.getMaterialData()) != m_TextureTable.end())
	{
		index = m_TextureTable.at(&component.getMaterialData());
	}
	else
	{
		index = createMaterial(component.getMaterialData());
	}
	m_Textures[index].bind();
	shader.setUniform1i("u_Texture", 0);
}

Shader& RenderResourceHandler::getShader(unsigned int index)
{
	return m_Shaders[index];
}

ShadowMap& RenderResourceHandler::getShadowMap()
{
	return m_ShadowMap;
}

