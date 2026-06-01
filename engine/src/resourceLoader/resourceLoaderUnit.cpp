#include "resourceLoaderUnit.hpp"
#include "resourceLoader.hpp"

namespace CustomEngine
{
	ResourceLoaderUnit* ResourceLoaderUnit::create()
	{
		return new ResourceLoader();
	}
}