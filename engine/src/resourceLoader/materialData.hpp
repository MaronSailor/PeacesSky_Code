#pragma once
#include <vector>

struct MaterialData
{
	std::vector<unsigned char> textureData;
	int textureWidth, textureHeight, textureBPP;
};