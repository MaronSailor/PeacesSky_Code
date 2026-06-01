#include "physicsUnit.hpp"

#include "physicsEngine.hpp"

PhysicsUnit* PhysicsUnit::createPhysicsUnit()
{
	return new PhysicsEngine();
}