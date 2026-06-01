#pragma once
#include "../../resourceLoader/vecStructs.hpp"

struct CUSTOMENGINE_API Camera
{
	float fov = 45.0f;
	float nearP = 0.1f;
	float farP = 10.0f;
	Vec3 location;
	Quaternion rotation;
};