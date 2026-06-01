
#include "meshCollection.hpp"
#include "objLoadFunction.hpp"


MeshData* MeshCollection::loadMesh(const std::string& filePath)
{
	if (m_AllMeshPaths.find(filePath) != m_AllMeshPaths.end())
	{
		unsigned int index = m_AllMeshPaths.at(filePath);
		auto iterator = std::next(m_AllMeshes.begin(), index);
		return &(*iterator);
	}
	auto iterator = std::next(m_AllMeshes.begin(), insertMesh(filePath));
	return &(*iterator);
}

unsigned int MeshCollection::insertMesh(const std::string& filePath)
{
	unsigned int newIndex = m_AllMeshes.size();
	m_AllMeshes.emplace_back(loadFile(filePath));
	m_AllMeshPaths[filePath] = newIndex;
	return newIndex;
}

MeshData MeshCollection::loadFile(const std::string& filePath)
{
	MeshData md;
	loadObjToMeshData(md, filePath.c_str(), NULL, true);
	return md;
}