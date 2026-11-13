// VR weapon pose functions of the client-side CBasePlayer, and client stubs of the VR methods of server classes
// (the SDK's own stubs are in hl/hl_baseentity.cpp)

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "cl_dll.h"
#include "vr/VRHelper.h"
#include "vr/VRRenderer.h"

Vector CBasePlayer::GetAimAngles()
{
	return UTIL_VecToAngles(GetAutoaimVector());
}
const Vector CBasePlayer::GetWeaponPosition()
{
	return gVRRenderer.GetHelper()->GetWeaponPosition();
}
const Vector CBasePlayer::GetWeaponAngles()
{
	return gVRRenderer.GetHelper()->GetWeaponAngles();
}
const Vector CBasePlayer::GetWeaponViewAngles()
{
	Vector angles = GetWeaponAngles();
	angles.x = -angles.x;
	return angles;
}
bool CBasePlayer::IsWeaponUnderWater()
{
	extern cl_enginefunc_t gEngfuncs;
	float weaponPos[3];
	GetWeaponPosition().CopyToArray(weaponPos);
	return gEngfuncs.PM_PointContents(weaponPos, nullptr) == CONTENTS_WATER;
}
bool CBasePlayer::VRCanAttack()
{
	return gVRRenderer.GetHelper()->CanAttack();
}

Vector CBasePlayer::GetGunPosition(void)
{
	return gVRRenderer.GetHelper()->GetGunPosition();
}
Vector CBasePlayer::GetAutoaimVector(float flDelta)
{
	return gVRRenderer.GetHelper()->GetAutoaimVector();
}

// Stubs
void CBaseEntity::UpdateOnRemove() {}
bool CGrenade::IsDraggable() { return false; }
void CGrenade::HandleDragStart() {}
void CGrenade::HandleDragStop() {}
void CGrenade::HandleDragUpdate(const Vector& origin, const Vector& velocity, const Vector& angles) {}
void CGrenade::BaseBalled(CBaseEntity* pPlayer, const Vector& velocity) {}
