#pragma once

// Per-game data and hooks used by the shared client VR code.
// Every branch has all the VRGameClient_<game>.cpp files; the build compiles the one for its game
// (VR_GAME in linux/Makefile.hl_cdll, the ClCompile entry in projects/vs2019/hl_cdll.vcxproj).
// Game data (map names, sky names, HUD sheets) belongs there, not in the shared VR files.

namespace VRGame
{
// Name of the VR skybox ("desert") for a map ("maps/c0a0.bsp"), nullptr if the map is not listed.
const char* GetSkyboxName(const char* mapName);
} // namespace VRGame
