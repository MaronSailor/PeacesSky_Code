#pragma once
#include "scene.hpp"
#include "../resourceLoader/resourceLoaderUnit.hpp"
#include <vector>
#include <memory>

namespace CustomEngine
{
	class SceneManager
	{
		std::vector<std::unique_ptr<Scene>> m_Scenes;

	public:
		SceneManager()
		{
		}

		unsigned int addScene()
		{
			unsigned int index = static_cast<unsigned int>(m_Scenes.size());
			m_Scenes.emplace_back(std::make_unique<Scene>());
			return index;
		}

		Scene& getScene(unsigned int index)
		{
			return *m_Scenes[index];
		}
	};
}