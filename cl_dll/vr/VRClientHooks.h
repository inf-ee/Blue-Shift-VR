#pragma once

// The VR functions and globals that SDK client files use (the hooks), other than the methods of gVRRenderer.
// Only forward declarations and standard headers, so this header parses on its own from any SDK file.

#include <unordered_set>

class Vector;
struct cl_entity_s;

// cdll_int.cpp: replaces entries of the engine function table (VREngineFuncSwaps.cpp)
void VRSwapEngineFuncs();

// hud.cpp: hooks the VR user messages (VRHudMessages.cpp)
void VRHookHudMessages();

// hl_weapons.cpp, weapons_shared.cpp: true while the weapon may not be used, e.g. at a mounted gun (VRHudAmmo.cpp)
bool GetHUDWeaponBlocked();

// Weapon code: controller recoil feedback (VRInput.cpp)
void VRRegisterRecoil(float intensity);

// ev_common.cpp: position and aim of the VR weapon (VRRenderer.cpp)
Vector VRGlobalGetGunPosition();
void VRGlobalGetGunAim(Vector& forward, Vector& right, Vector& up, Vector& angles);

// entity.cpp: the temp entities of the current frame. Used by the studio model renderer to filter out temp ents
// when applying scale (as scale is used as a timer there) - Max Makes Mods, 2019-05-26
extern std::unordered_set<cl_entity_s*> g_curFrameTempEnts;
