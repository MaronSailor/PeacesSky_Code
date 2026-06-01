#pragma once

#include <cstdint>

#define GAME_VERSION_STRING "0.2.6"

namespace GameVersion
{
	constexpr uint32_t fnv1a(const char* str, uint32_t hash = 2166136261u)
	{
		return (*str == '\0') ? hash : fnv1a(str + 1, (hash ^ static_cast<uint32_t>(*str)) * 16777619u);
	}

	constexpr uint32_t VERSION_HASH = fnv1a(GAME_VERSION_STRING);
}