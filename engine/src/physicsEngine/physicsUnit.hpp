#pragma once

#include "../ecs/components/physicsComponent.hpp"
#include "../ecs/engineSceneInfo.hpp"

class PhysicsUnit
{
public:
	static PhysicsUnit* createPhysicsUnit();

	virtual ~PhysicsUnit() = default;

	virtual void calculateScenePhysics(EngineSceneInfo* sceneInfo) = 0;

	virtual void startAsyncCalculation(EngineSceneInfo* sceneInfo) { calculateScenePhysics(sceneInfo); }
	virtual void waitForCalculation() {}
};