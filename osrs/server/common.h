#pragma once
#include <cstdint>

enum class GameMsg : uint32_t
{
	Server_GetStatus,
	Server_GetPing,

	Client_Accepted,
	Client_AssignID,
	Client_RegisterWithServer,
	Client_UnregisterWithServer,

	Game_AddPlayer,
	Game_RemovePlayer,
	Game_UpdatePlayer,
};

struct sPlayerDescription
{
	uint32_t nUniqueID = 0;
	uint32_t nAvatarID = 0;

	uint32_t nOre = 0;
	float fMiningSpeed = 1.0f;

	//olc::vf2d vPos;
	//olc::vf2d vVel;
};