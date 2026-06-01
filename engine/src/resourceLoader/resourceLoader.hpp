#pragma once
#include "meshCollection.hpp"
#include "materialCollection.hpp"
#include "resourceLoaderUnit.hpp"

namespace CustomEngine
{
	class ResourceLoader : public ResourceLoaderUnit
	{
		MeshCollection m_Meshes;
		MaterialCollection m_Materials;

	public:
		ResourceLoader();
		virtual ~ResourceLoader();

		MeshData* getMesh(const std::string& filePath) override;
		MaterialData* getMaterial(const std::string& filePath) override;
	};
}