#pragma once

// Per-game data and hooks used by the shared server VR code.
// Every branch has all the VRGameServer_<game>.cpp files; the build compiles the one for its game
// (VR_GAME in linux/Makefile.hldll, the ClCompile entry in projects/vs2019/hldll.vcxproj).
// Game data (map names, entity names, per-map hacks) belongs there, not in the shared VR files.

class CBasePlayer;

// How a brush entity crushes a player who is stuck in it (VRCrushEntityHandler.cpp)
enum class VRCrushMode
{
	NEVER,
	ALWAYS,
	WHEN_MOVING,
	WHEN_MOVING_DOWN,
	WHEN_MOVING_UP,
	HEALING_EXPLOIT
};

namespace VRGame
{
// Crush mode of a brush model ("*63") on a map ("maps/c1a0c.bsp"). VRCrushMode::NEVER if not listed.
VRCrushMode GetCrushMode(const char* mapName, const char* brushModelName);

// Targetname of the intro ride train the player must stay in on this map, nullptr if the map has none.
const char* GetIntroTrainRideTargetName(const char* mapName);

// True if the brush entity with this targetname must never be used as the player's ground entity on this map.
bool IsExcludedGroundEntity(const char* mapName, const char* targetName);

// Called at the end of the player's restore from a savegame.
void OnPlayerRestored(CBasePlayer* pPlayer);
} // namespace VRGame
