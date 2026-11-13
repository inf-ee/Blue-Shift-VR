#pragma once

// Per-game data and hooks used by the shared client VR code.
// Every branch has all the VRGameClient_<game>.cpp files; the build compiles the one for its game
// (VR_GAME in linux/Makefile.hl_cdll, the ClCompile entry in projects/vs2019/hl_cdll.vcxproj).
// Game data (map names, sky names, HUD sheets) belongs there, not in the shared VR files.

// The weapon wheel has this many spokes around the bare hand in the centre, each with room for this many weapons
constexpr int VR_WHEEL_NUM_SPOKES = 6;
constexpr int VR_WHEEL_SPOKE_LENGTH = 3;

namespace VRGame
{
// Name of the mod in log lines and error messages.
const char* GetModTitle();

// The liblist.gam the mod's version is read from, relative to the game's main folder.
const char* GetVersionFilePath();

// Weapon id (WEAPON_*) at a place of the weapon wheel, 0 if the place is unused. Spoke 0 points up, the others follow
// clockwise; index 0 is next to the centre. Only the weapons the player has are drawn, one after the other
// without gaps, so a spoke grows as weapons are picked up. A weapon that isn't on the wheel can't be selected with it.
int GetWeaponWheelWeapon(int spoke, int index);

// Name of the VR skybox ("desert") for a map ("maps/c0a0.bsp"), nullptr if the map is not listed.
const char* GetSkyboxName(const char* mapName);
} // namespace VRGame
