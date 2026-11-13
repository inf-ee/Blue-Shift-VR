// Half-Life: Blue Shift data and hooks for the server VR code, see VRGameServer.h

#include <cstring>

#include "VRGameServer.h"

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
