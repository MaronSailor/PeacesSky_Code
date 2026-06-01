#pragma once
#include <vector>
#include <cstdint>

#include "vecStructs.hpp"

struct VertexData
{
	Vec3 position;
	Vec2 texCoords;
	Vec3 normals;
};

struct MeshData
{
	std::vector<VertexData> vertexBufferData;
	std::vector<unsigned int> indexBufferData;
	uint32_t revision = 0;
};