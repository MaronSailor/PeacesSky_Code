#pragma once
#include "../ecs/ecs.hpp"
#include "../resourceLoader/resourceLoader.hpp"
#include "../ecs/components/cameraComponent.hpp"
#include "../ecs/components/lightSourceComponent.hpp"

struct Scene
{
	ECS ecs;
	std::vector<Entity> entities;
	Camera sceneCamera;
	Camera* activeCamera = &sceneCamera;

	uint32_t addEntity()
	{
		uint32_t index = entities.size();
		entities.emplace_back(ecs);
		return index;
	}

	void removeEntity(uint32_t index)
	{
		if (index >= entities.size()) return;
		entities.erase(entities.begin() + index);
	}
};