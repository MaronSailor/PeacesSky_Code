#pragma once
#include "components/transformComponent.hpp"
#include "components/renderComponent.hpp"
#include "components/cameraComponent.hpp"
#include "components/lightSourceComponent.hpp"
#include "components/physicsComponent.hpp"

struct EngineSceneInfo
{
	std::vector<Transform>* pTransformComponentStorage = nullptr;
	std::vector<RenderComponent>* pRenderComponentStorage = nullptr;
	Camera* pActiveCamera = nullptr;
	std::vector<LightSource>* pLightComponentStorage = nullptr;
	std::vector<PhysicsComponent>* pPhysicsComponentStorage = nullptr;
	double deltaTime = 0.0;
};