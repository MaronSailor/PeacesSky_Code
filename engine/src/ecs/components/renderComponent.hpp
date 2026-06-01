#pragma once
#include "transformComponent.hpp"

#include "../../resourceLoader/resourceLoader.hpp"

#include <iostream>

class CUSTOMENGINE_API RenderComponent
{
	unsigned int* m_pObjTransformIndex = nullptr;

	CustomEngine::ResourceLoaderUnit* m_Loader = nullptr;

	MeshData* m_Mesh = nullptr;
	MaterialData* m_Material = nullptr;

public:
	bool isVisible = true;

	void setMesh(std::string filePath)
	{
		if (m_Loader) m_Mesh = m_Loader->getMesh(filePath);
	}

	void setMeshData(MeshData* mesh)
	{
		m_Mesh = mesh;
	}

	void setMaterial(std::string filepath)
	{
		if (m_Loader) m_Material = m_Loader->getMaterial(filepath);
	}

	void setResourceLoader(CustomEngine::ResourceLoaderUnit* loader)
	{
		m_Loader = loader;
	}

	void setTransformIndex(unsigned int& transformIndex)
	{
		m_pObjTransformIndex = &transformIndex;
	}

	unsigned int getTransformIndex()
	{
		return *m_pObjTransformIndex;
	}

	MeshData& getMeshData()
	{
		return *m_Mesh;
	}

	MaterialData& getMaterialData()
	{
		return *m_Material;
	}

	~RenderComponent()
	{

	}
};