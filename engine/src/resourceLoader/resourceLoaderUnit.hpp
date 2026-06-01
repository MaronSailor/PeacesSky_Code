#pragma once
#include "../engineCore/core.h"
#include <string>
#include "../resourceLoader/meshData.hpp"
#include "../resourceLoader/materialData.hpp"

namespace CustomEngine
{
	class CUSTOMENGINE_API ResourceLoaderUnit
	{
	public:
		virtual ~ResourceLoaderUnit() = default;

		static ResourceLoaderUnit* create();

		virtual MeshData* getMesh(const std::string& filePath) = 0;
		virtual MaterialData* getMaterial(const std::string& filePath) = 0;
	};
}