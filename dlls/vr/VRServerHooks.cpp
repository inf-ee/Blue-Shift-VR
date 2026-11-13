// Hooks called from SDK files (client.cpp, world.cpp, cbase.cpp), declared in VRHooks.h

#include "VRHooks.h"

#include <iterator>
#include <unordered_map>
#include <unordered_set>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"

#include "VRCommons.h"
#include "VRGameServer.h"
#include "VRPhysicsHelper.h"

extern void VRClearCvarCache(); // VREngineCallbacks.cpp

// VRPlayer.cpp
extern std::unordered_map<EHandleT<CBaseEntity>, EHandleT<CBaseEntity>, EHandleT<CBaseEntity>::Hash, EHandleT<CBaseEntity>::Equal> g_vrRetinaScanners;
extern std::unordered_set<EHandleT<CBaseEntity>, EHandleT<CBaseEntity>::Hash, EHandleT<CBaseEntity>::Equal> g_vrRetinaScannerButtons;
extern bool g_vrNeedRecheckForSpecialEntities;

namespace
{
// Marks the current save version of the mod. Prevents loading incompatible savegames.
// Not to be confused with the mod version itself.
// Incremented every time the savedata of entities is changed.
// The mod is not compatible with savegames from vanilla HL, so this protects us from that as well.
struct HLVRSaveVersionMarker
{
public:
	int magic = VRGame::GetSaveMagic(); // per game, so saves from the other games' VR mods are rejected as well
	int version = VRGame::GetSaveVersion();
};

TYPEDESCRIPTION g_hlvrSaveVersionMarkerSaveData[] =
	{
		DEFINE_FIELD(HLVRSaveVersionMarker, magic, FIELD_INTEGER),
		DEFINE_FIELD(HLVRSaveVersionMarker, version, FIELD_INTEGER),
};

bool g_didRestoreSaveGameFail = false; // Initially false
bool g_didRestoreSaveGameFail_MapChangedToSafety = false;
} // namespace

bool VRWriteSaveMarker(CSave& save)
{
	HLVRSaveVersionMarker hlvrSaveVersionMarker; // not const: CSave::WriteFields takes void*
	return save.WriteFields("HLVRSaveVersionMarker", &hlvrSaveVersionMarker, g_hlvrSaveVersionMarkerSaveData, (int)std::size(g_hlvrSaveVersionMarkerSaveData));
}

bool VRReadSaveMarker(CRestore& restore)
{
	const HLVRSaveVersionMarker expectedMarker;
	HLVRSaveVersionMarker hlvrSaveVersionMarker;
	bool status = restore.ReadFields("HLVRSaveVersionMarker", &hlvrSaveVersionMarker, g_hlvrSaveVersionMarkerSaveData, (int)std::size(g_hlvrSaveVersionMarkerSaveData));
	return status && hlvrSaveVersionMarker.magic == expectedMarker.magic && hlvrSaveVersionMarker.version == expectedMarker.version;
}

void VROnGlobalStateRestored(bool success)
{
	if (!success)
	{
		// if result is negative, we couldn't load the savegame.
		// the engine doesn't support canceling loads, so we do it in a bit of a hacky way:
		// we set this global flag and check it in several places (VRIsRestoreFailed):
		//  - cbase.cpp DispatchSpawn
		//  - cbase.cpp DispatchRestore
		//  - client.cpp ClientConnect
		//  - client.cpp ServerActivate
		//  - client.cpp StartFrame (VRStartFrame)
		g_didRestoreSaveGameFail = true;
		g_didRestoreSaveGameFail_MapChangedToSafety = false;

		// Inform user
		g_engfuncs.pfnServerPrint(VRGame::GetSaveIncompatibleMessage());
		ALERT(at_error, VRGame::GetSaveIncompatibleMessage());
	}
	else
	{
		g_didRestoreSaveGameFail = false;
		g_didRestoreSaveGameFail_MapChangedToSafety = false;
	}
}

void VROnGlobalStateReset()
{
	// Engine calls this on every new game. Clear the failed-restore flag set by
	// RestoreGlobalState, otherwise ClientConnect keeps rejecting the player.
	g_didRestoreSaveGameFail = false;
	g_didRestoreSaveGameFail_MapChangedToSafety = false;
}

bool VRIsRestoreFailed()
{
	return g_didRestoreSaveGameFail;
}

void VRServerActivate()
{
	VRPhysicsHelper::CreateInstance();
}

void VRServerDeactivate()
{
	VRPhysicsHelper::DestroyInstance();
}

bool VRStartFrame()
{
	VRClearCvarCache();

	// Don't think in game loaded from invalid savegame (see VROnGlobalStateRestored)
	if (g_didRestoreSaveGameFail)
	{
		// We cannot shut down a server, but we can switch to another map
		if (!g_didRestoreSaveGameFail_MapChangedToSafety)
		{
			CHANGE_LEVEL(VRGame::GetRestoreFailedFallbackMap(), nullptr);
			g_didRestoreSaveGameFail_MapChangedToSafety = true;
		}
		return false;
	}

	VRPhysicsHelper::Instance().StartFrame();
	return true;
}

void VRClientPrecache()
{
	PRECACHE_SOUND("plats/train_use2.wav"); // use a train in VR
	PRECACHE_MODEL("sprites/black.spr");

	// Clear global VR stuff here - Max Makes Mods, 2018-04-02
	gGlobalXenMounds.Clear();
	g_vrRetinaScanners.clear();
	g_vrRetinaScannerButtons.clear();
	VRClearSpecialEntities();
	g_vrNeedRecheckForSpecialEntities = true;
}
