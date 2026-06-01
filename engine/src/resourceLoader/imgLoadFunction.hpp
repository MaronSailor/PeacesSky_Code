#pragma once

#include <stb_image.h>
#include <iostream>

inline void loadPngToMaterialData(const std::string& filePath, MaterialData& materialData)
{
	stbi_set_flip_vertically_on_load(1);
	unsigned char* localBuffer = stbi_load(filePath.c_str(), &materialData.textureWidth,
		&materialData.textureHeight, &materialData.textureBPP, 4);

	if (!localBuffer) {
		std::cout << "stbi_load failed for " << filePath << ": " << stbi_failure_reason() << "\n";
		materialData.textureWidth = materialData.textureHeight = materialData.textureBPP = 0;
		materialData.textureData.clear();
		return;
	}

	size_t byteLength = (size_t)materialData.textureWidth
		* (size_t)materialData.textureHeight
		* 4u;

	materialData.textureData.clear();
	materialData.textureData.resize(byteLength);
	memcpy(materialData.textureData.data(), localBuffer, byteLength);
	if (localBuffer) stbi_image_free(localBuffer);
}