// Half-Life: Blue Shift data and hooks for the server VR code, see VRGameServer.h

#include <cstring>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"

#include "VRGameServer.h"
#include "vr/VRShared.h"

VRCrushMode VRGame::GetCrushMode(const char* mapName, const char* brushModelName)
{
	// No crushing brush entities are listed for Blue Shift maps.
	return VRCrushMode::NEVER;
}

const char* VRGame::GetIntroTrainRideTargetName(const char* mapName)
{
	// Disabled for Blue Shift. The candidates were:
	// 	{ "maps/ba_tram1.bsp", "barney_tram" },
	// 	{ "maps/ba_tram2.bsp", "barney_tram" },
	// 	{ "maps/ba_tram3.bsp", "barney_tram" },
	// 	{ "maps/ba_elevator.bsp", "elevator" }
	return nullptr;
}

bool VRGame::IsExcludedGroundEntity(const char* mapName, const char* targetName)
{
	// ba_elevator: "fake_elevator" is a stationary func_train the player stands in before the ride; when the button is pressed
	// the map teleports the player into the real "elevator" and resets/removes fake_elevator, which would otherwise be read
	// as a huge move and drag the player away.
	return strcmp(mapName, "maps/ba_elevator.bsp") == 0 && strcmp(targetName, "fake_elevator") == 0;
}

void VRGame::OnPlayerRestored(CBasePlayer* pPlayer)
{
}

const char* VRGame::GetGameDescription()
{
	return "Half-Life: Blue Shift VR";
}

int VRGame::GetSaveMagic()
{
	return 'BSVR';
}

int VRGame::GetSaveVersion()
{
	return 1;
}

const char* VRGame::GetSaveIncompatibleMessage()
{
	return "COULDN'T LOAD SAVEGAME, SAVE FILE IS INCOMPATIBLE WITH HALF-LIFE: BLUE SHIFT VR!\n";
}

const char* VRGame::GetRestoreFailedFallbackMap()
{
	return "crossfire";
}

const char* VRGame::GetXenMoundTextureName()
{
	return "c2a5mound";
}

void VRGame::OnTriggerPushTouched(CBaseEntity* pTriggerPush, CBaseEntity* pOther)
{
}

void VRGame::OnPlayerTeleported(CBasePlayer* pPlayer, CBaseEntity* pTrigger)
{
}

bool VRGame::IsTrashCompactor(CBaseEntity* pEntity)
{
	return false;
}

// Blue Shift has no achievements and none of the Half-Life map fixes

void VRGame::OnHeadcrabBaseBalled(CBaseEntity* pHeadcrab, CBaseEntity* pPlayer)
{
}

void VRGame::OnButtonTriggered(CBaseEntity* pButton, CBaseEntity* pActivator)
{
}

void VRGame::OnPlayerTakeDamage(CBasePlayer* pPlayer, entvars_s* pevInflictor)
{
}

void VRGame::OnPlayerUsePressed(CBasePlayer* pPlayer)
{
}

void VRGame::OnBarnacleTongueTouch(CBaseEntity* pBarnacle, CBaseEntity* pTouchEnt)
{
}

void VRGame::OnSodaDrunk(CBaseEntity* pDrinker)
{
}

void VRGame::OnBarneyGetSchedule(CBaseMonster* pBarney)
{
}

void VRGame::OnBreakableDie(CBaseEntity* pBreakable, bool killedByTrigger)
{
}

void VRGame::OnPlayerRestoredWithLandmark(CBasePlayer* pPlayer)
{
}

void VRGame::OnPlayerUsedTrain(CBasePlayer* pPlayer)
{
}

void VRGame::OnGibCaught(CBaseEntity* pMonster, CBaseEntity* thrower)
{
}

void VRGame::OnBareHandPetting(CBasePlayer* pPlayer, CBaseEntity* pEntity)
{
}

void VRGame::OnPlayerTakeNegativeCrushDamage(CBaseEntity* pPlayer, float dmg)
{
}

void VRGame::OnPlayerOutsideIntroTrain(CBasePlayer* pPlayer)
{
}

void VRGame::OnPlayerKilled(CBasePlayer* pPlayer, entvars_s* pevAttacker)
{
}

bool VRGame::IsWeaponWithRange(int iId)
{
	return iId == WEAPON_GLOCK || iId == WEAPON_PYTHON || iId == WEAPON_MP5 || iId == WEAPON_CROSSBOW || iId == WEAPON_SHOTGUN || iId == WEAPON_RPG || iId == WEAPON_GAUSS || iId == WEAPON_EGON || iId == WEAPON_HORNETGUN;
}

bool VRGame::IsWeapon(int iId)
{
	return iId == WEAPON_CROWBAR || iId == WEAPON_GLOCK || iId == WEAPON_PYTHON || iId == WEAPON_MP5 || iId == WEAPON_CROSSBOW || iId == WEAPON_SHOTGUN || iId == WEAPON_RPG || iId == WEAPON_GAUSS || iId == WEAPON_EGON || iId == WEAPON_HANDGRENADE || iId == WEAPON_TRIPMINE || iId == WEAPON_SATCHEL || iId == WEAPON_SNARK;
}

bool VRGame::IsExplosiveWeapon(int iId)
{
	return iId == WEAPON_SNARK || iId == WEAPON_HANDGRENADE || iId == WEAPON_TRIPMINE || iId == WEAPON_SATCHEL;
}

bool VRGame::IsWeaponWithVRLaserSpot(int iId)
{
	return IsWeaponWithRange(iId) && iId != WEAPON_RPG && iId != WEAPON_HORNETGUN;
}

const char* VRGame::GetRetinaScannerTextureName()
{
	return "generic_113";
}

bool VRGame::IsDisabledRetinaScannerButton(edict_s* pentButton)
{
	return false;
}

bool VRGame::ReplaceSpecialWall(CBaseEntity* pWall)
{
	return false;
}

const VRAllyInfo* VRGame::GetAllyInfo(const char* classname)
{
	static const VRAllyInfo barney{"BA_GUN_ONE", "BA_GUN_TWO", "BA_GUN_THR", "BA_GUN_ATT", true, "Barney Disarm"};
	static const VRAllyInfo scientist{"SC_GUN_ONE", "SC_GUN_TWO", "SC_GUN_THR", "SC_GUN_RUN", false, nullptr};

	if (strcmp(classname, "monster_barney") == 0)
		return &barney;
	if (strcmp(classname, "monster_scientist") == 0)
		return &scientist;
	if (strcmp(classname, "monster_rosenberg") == 0) // Rosenberg is a following scientist too, with the scientists' gunpoint lines
		return &scientist;
	return nullptr;
}

bool VRGame::IsSoftWeapon(int weaponId)
{
	switch (weaponId)
	{
	case WEAPON_HORNETGUN:
	case WEAPON_NONE:
	case WEAPON_BAREHAND:
	case WEAPON_SNARK:
		return true;
	default:
		return false;
	}
}

float VRGame::GetMeleeDamageScale(int iId)
{
	switch (iId)
	{
	case WEAPON_CROWBAR:
		return 1.f;
	case WEAPON_HORNETGUN:
		return 0.05f;
	case WEAPON_NONE:
	case WEAPON_BAREHAND:
	case WEAPON_SNARK:
		return 0.1f;
	case WEAPON_HANDGRENADE:
	case WEAPON_TRIPMINE:
	case WEAPON_SATCHEL:
		return 0.2f;
	case WEAPON_GLOCK:
	case WEAPON_PYTHON:
		return 0.3f;
	case WEAPON_MP5:
	case WEAPON_SHOTGUN:
	case WEAPON_CROSSBOW:
		return 0.4f;
	case WEAPON_RPG:
	case WEAPON_GAUSS:
	case WEAPON_EGON:
		return 0.5f;
	default:
		return 0.1f;
	}
}

int VRGame::GetMeleeDamageType(int iId)
{
	switch (iId)
	{
	case WEAPON_CROWBAR:
		return DMG_CLUB;
	default:
		return DMG_CLUB | DMG_NEVERGIB;
	}
}

void VRGame::OnPlayerPreThinkStart(CBasePlayer* pPlayer)
{
}
