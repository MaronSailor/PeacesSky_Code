#pragma once
#include "../../engineCore/core.h"
#include "../../resourceLoader/vecStructs.hpp"

struct CUSTOMENGINE_API Transform
{
	Vec3 location {0.0f, 0.0f, 0.0f};
	Quaternion rotation {1.0f, 0.0f, 0.0f, 0.0f };
	Vec3 scale { 1.0f, 1.0f, 1.0f };
};