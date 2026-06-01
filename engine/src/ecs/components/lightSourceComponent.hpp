#pragma once
#include "../../engineCore/core.h"
#include "../../resourceLoader/vecStructs.hpp"

enum LightType
{
	DirectionalLight,
	Spotlight
};

struct CUSTOMENGINE_API LightSource
{
	LightType type;
	Vec3 location;
	Vec3 rotation;
	Vec4Color color;
};