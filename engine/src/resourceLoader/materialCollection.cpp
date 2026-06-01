
#include "materialCollection.hpp"
#include "imgLoadFunction.hpp"

MaterialData* MaterialCollection::loadMaterial(const std::string& filePath)
{
	if (m_AllMaterialPaths.find(filePath) != m_AllMaterialPaths.end())
	{
		unsigned int index = m_AllMaterialPaths.at(filePath);
		auto iterator = std::next(m_AllMaterials.begin(), index);
		return &(*iterator);
	}
	auto iterator = std::next(m_AllMaterials.begin(), insertMaterial(filePath));
	return &(*iterator);
}

unsigned int MaterialCollection::insertMaterial(const std::string& filePath)
{
	unsigned int newIndex = m_AllMaterials.size();
	m_AllMaterials.emplace_back(loadFile(filePath));
	m_AllMaterialPaths[filePath] = newIndex;
	return newIndex;
}

MaterialData MaterialCollection::loadFile(const std::string& filePath)
{
	MaterialData md;
	loadPngToMaterialData(filePath, md);
	return md;
}
