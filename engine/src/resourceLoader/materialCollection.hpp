#pragma once
#include "materialData.hpp"

#include <unordered_map>
#include <string>
#include <list>

class MaterialCollection
{
	std::list<MaterialData> m_AllMaterials;
	std::unordered_map<std::string, unsigned int> m_AllMaterialPaths;

public:
	MaterialData* loadMaterial(const std::string& filePath);
	unsigned int insertMaterial(const std::string& filePath);
	MaterialData loadFile(const std::string& filePath);
};