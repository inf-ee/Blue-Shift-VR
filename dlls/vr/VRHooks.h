#pragma once

// The VR functions that SDK files call (the hooks), other than CBasePlayer's VR methods.
// Only forward declarations and basic types, so this header parses on its own from any SDK file.

class CBaseEntity;
class CBasePlayer;
class CSave;
class CRestore;
class Vector;
struct clientdata_s;
struct edict_s;
struct entity_state_s;

// client.cpp
bool VRClientCommand(edict_s* pEntity, CBasePlayer* player, const char* pcmd); // VRClientCommands.cpp
void VRServerActivate();
void VRServerDeactivate();
// Returns false if the rest of the server frame must be skipped (after a failed restore).
bool VRStartFrame();
void VRClientPrecache();
// SetupVisibility / AddToFullPack: remembers for every entity if it is in the player's PVS (used by the gib limit).
void VRSetPVSCache(unsigned char* pvs);
bool VRCheckAndRecordPVS(edict_s* ent, unsigned char* pSet);
// CreateBaseline: the VR player is a noclip entity without gravity, with its real bounds.
void VROverridePlayerBaseline(entity_state_s* baseline, edict_s* entity);
// UpdateClientData: the client gets the origin and view offset of the headset.
void VRSetClientDataOrigin(CBasePlayer* pl, clientdata_s* cd);

// world.cpp, cbase.cpp, client.cpp: save version marker.
// Savegames from the vanilla game or from another version of the mod are rejected. The engine can't cancel a load,
// so a failed restore sets a flag: nothing is spawned or restored, no client connects, and the server changes to a safe map.
bool VRWriteSaveMarker(CSave& save);
bool VRReadSaveMarker(CRestore& restore);
void VROnGlobalStateRestored(bool success);
void VROnGlobalStateReset();
bool VRIsRestoreFailed();

// triggers.cpp (VRSpecialEntities.cpp)
// True while the VR noclip cheat is on (VRCommons.cpp; also declared for pm_shared.cpp in VRPMShared.h)
bool VRGlobalGetNoclipMode();
// func_friction: true if the VR code set the toucher's friction and the original code must be skipped.
bool VRHandleFrictionTouch(CBaseEntity* pOther);
// trigger_changelevel: called right before the level change.
void VROnChangeLevelNow(CBaseEntity* pPlayer);
// trigger_changelevel: true if the entity must be treated as inside the transition volume.
bool VRForceInTransition(CBaseEntity* pEntity);
// trigger_teleport: called after pOther was teleported.
void VROnTeleported(CBaseEntity* pTrigger, CBaseEntity* pOther, const Vector& prevOrigin, const Vector& prevAngles);
// Xen jump pads: a trigger_push and the trigger_multiple around it. Found once per map (VRCheckXenJumpPad),
// so the player can choose to activate them with the VR teleporter only.
void VROnTriggerPushSpawn(CBaseEntity* pTriggerPush);
bool VRCheckXenJumpPad(CBaseEntity* pTriggerPush);
bool VRIsXenJumpTrigger(CBaseEntity* pTrigger);
bool VRBlockXenJumpTouch(CBaseEntity* pTrigger, CBaseEntity* pOther);
void VRClearSpecialEntities();
// Retina scanners (VRSpecialEntities.cpp): false if the wall's model isn't loaded yet and it must be checked again.
bool VRCheckRetinaScanner(CBaseEntity* pWall);
