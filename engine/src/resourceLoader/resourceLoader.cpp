#include "resourceLoader.hpp"

namespace CustomEngine
{
	ResourceLoader::ResourceLoader()
	{
		
	}

	ResourceLoader::~ResourceLoader()
	{
		
	}

	MeshData* ResourceLoader::getMesh(const std::string& filepath)
	{
		return m_Meshes.loadMesh(filepath);
	}

	MaterialData* ResourceLoader::getMaterial(const std::string& filepath)
	{
		return m_Materials.loadMaterial(filepath);
	}
}