#pragma once

// Per-game data and hooks used by the shared server VR code.
// Every branch has all the VRGameServer_<game>.cpp files; the build compiles the one for its game
// (VR_GAME in linux/Makefile.hldll, the ClCompile entry in projects/vs2019/hldll.vcxproj).
// Game data (map names, entity names, per-map hacks) belongs there, not in the shared VR files.

class CBaseEntity;
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

// Texture under a trigger_push that marks it as a xen jump pad (VRSpecialEntities.cpp), also used by the VR teleporter.
const char* GetXenMoundTextureName();

// Called when a trigger_push pushed an entity that touches it (the non-instant kind of push).
void OnTriggerPushTouched(CBaseEntity* pTriggerPush, CBaseEntity* pOther);

// Called after a trigger_teleport teleported the player.
void OnPlayerTeleported(CBasePlayer* pPlayer, CBaseEntity* pTrigger);

// Game name the server reports before the game rules exist (GetGameDescription in client.cpp).
const char* GetGameDescription();

// Save version marker (VRServerHooks.cpp): a savegame loads only if its magic and version match.
// Increment the version every time the savedata of entities is changed.
int GetSaveMagic();
int GetSaveVersion();
// Console message for a savegame that was rejected, with a newline at the end.
const char* GetSaveIncompatibleMessage();
// Map the server changes to after a savegame was rejected (the engine can't cancel a load).
const char* GetRestoreFailedFallbackMap();
} // namespace VRGame
