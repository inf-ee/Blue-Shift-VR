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
