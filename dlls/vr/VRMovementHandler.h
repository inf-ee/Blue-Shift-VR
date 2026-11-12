#pragma once

#include "extdll.h"
#include "vector.h"

class CBaseEntity;

class VRMovementHandler
{
public:
	static Vector DoMovement(const Vector& from, const Vector& to, CBaseEntity* pMovingEntityForTouch = nullptr, bool allowZ = false);
};
