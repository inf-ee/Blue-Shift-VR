#pragma once

// Common defines and types for VR - Max Makes Mods - 2019-04-07

#include <map>
#include <cstdint>

#include "extdll.h"
#include "vector.h"

class CBasePlayer;

// How far in front of the hand the tripmine is placed (CTripmine::PrimaryAttack and UpdateGhost)
constexpr const float VR_TRIPMINE_PLACEMENT_DISTANCE = 32.f;

#include "VRStuff.h"
#include "vr/VRShared.h"

extern GlobalXenMounds gGlobalXenMounds;
extern VRLevelChangeData g_vrLevelChangeData;
