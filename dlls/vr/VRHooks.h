#pragma once

// The VR functions that SDK files call (the hooks), other than CBasePlayer's VR methods.
// Only forward declarations and basic types, so this header parses on its own from any SDK file.

class CBasePlayer;
class CSave;
class CRestore;
struct edict_s;

// client.cpp
bool VRClientCommand(edict_s* pEntity, CBasePlayer* player, const char* pcmd); // VRClientCommands.cpp
void VRServerActivate();
void VRServerDeactivate();
// Returns false if the rest of the server frame must be skipped (after a failed restore).
bool VRStartFrame();
void VRClientPrecache();

// world.cpp, cbase.cpp, client.cpp: save version marker.
// Savegames from the vanilla game or from another version of the mod are rejected. The engine can't cancel a load,
// so a failed restore sets a flag: nothing is spawned or restored, no client connects, and the server changes to a safe map.
bool VRWriteSaveMarker(CSave& save);
bool VRReadSaveMarker(CRestore& restore);
void VROnGlobalStateRestored(bool success);
void VROnGlobalStateReset();
bool VRIsRestoreFailed();
