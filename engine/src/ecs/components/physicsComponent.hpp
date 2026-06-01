#pragma once

#include "../../resourceLoader/vecStructs.hpp"

class PhysicsComponent
{
	unsigned int* m_pObjTransformIndex = nullptr;

public:
	void setTransformIndex(unsigned int& transformIndex)
	{
		m_pObjTransformIndex = &transformIndex;
	}

	const unsigned int getTransformIndex() const
	{
		return *m_pObjTransformIndex;
	}

	bool isStatic = true;
	bool isFalling = false;
	float gravityScale = 1.0f;
	Vec3 collisionDimensions;
	Vec3 locationInertia;
};