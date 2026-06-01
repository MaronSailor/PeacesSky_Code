#pragma once
#include "meshData.hpp"

#include <unordered_map>
#include <string>
#include <list>

class MeshCollection
{
	std::list<MeshData> m_AllMeshes;
	std::unordered_map<std::string, unsigned int> m_AllMeshPaths;

public:
	MeshData* loadMesh(const std::string& filePath);
	unsigned int insertMesh(const std::string& filePath);
	MeshData loadFile(const std::string& filepath);
};