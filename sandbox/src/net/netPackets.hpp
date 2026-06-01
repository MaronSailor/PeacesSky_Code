#pragma once

#include <cstdint>

enum class NetMessageType : uint32_t
{
	PlayerState = 1,
	BlockRemoveRequest = 2,
	BlockRemoved = 3,
	WorldSeed = 4,
	BlockPlaced = 5,
	BallSpawned = 6,
	VersionHandshake = 7,
	GameSettings = 8
};

struct NetMessagePacket
{
	uint32_t messageType = static_cast<uint32_t>(NetMessageType::PlayerState);

	// PlayerState payload | WorldSeed: px=platformSize, py=platformOffsetBlocks
	float px = 0.0f;
	float py = 0.0f;
	float pz = 0.0f; // FlyMode	

	float tmr = 0.0f; 

	float rw = 1.0f;
	float rx = 0.0f;
	float ry = 0.0f;
	float rz = 0.0f;

	// Block payload (center position) or Ball direction
	float bx = 0.0f;
	float by = 0.0f;
	float bz = 0.0f;

	// WorldSeed payload OR BlockPlaced blockType OR BallSpawned teamIndex OR VersionHandshake hash
	uint32_t seed = 0;
};

static_assert(sizeof(NetMessagePacket) == 52, "NetMessagePacket size must be 52 bytes.");