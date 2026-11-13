// VR handling of triggers, called from triggers.cpp through the hooks in VRHooks.h

#include "VRHooks.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "pm_defs.h"
#include "plane.h"
#include "com_model.h"
#include "vr/VRComModelHL25.h"

#include "VRCommons.h"
#include "VRGameServer.h"

extern const model_t* VRGetBSPModel(CBaseEntity* pEntity);

// VRPlayer.cpp
extern std::unordered_map<EHandleT<CBaseEntity>, EHandleT<CBaseEntity>, EHandleT<CBaseEntity>::Hash, EHandleT<CBaseEntity>::Equal> g_vrRetinaScanners;
extern std::unordered_set<EHandleT<CBaseEntity>, EHandleT<CBaseEntity>::Hash, EHandleT<CBaseEntity>::Equal> g_vrRetinaScannerButtons;

namespace
{
// trigger_push and trigger_multiple entities that belong to a xen jump pad
std::unordered_set<EHandleT<CBaseEntity>, EHandleT<CBaseEntity>::Hash, EHandleT<CBaseEntity>::Equal> g_vrXenJumpTriggers;

// pev->angles of every trigger_push at the time it spawned (InitTrigger clears them).
// Not filled when a savegame is restored (restored entities aren't spawned), same as the class member this replaced.
std::unordered_map<EHandleT<CBaseEntity>, Vector, EHandleT<CBaseEntity>::Hash, EHandleT<CBaseEntity>::Equal> g_vrTriggerPushSpawnAngles;

// Detect xen jump pads and enable special VR controls - Max Makes Mods, 2017-09-10
bool IsXenJumpPad(CBaseEntity* pTriggerPush)
{
	entvars_t* pev = pTriggerPush->pev;

	Vector m_spawnAngles;
	auto spawnAngles = g_vrTriggerPushSpawnAngles.find(EHandleT<CBaseEntity>{pTriggerPush});
	if (spawnAngles != g_vrTriggerPushSpawnAngles.end())
	{
		m_spawnAngles = spawnAngles->second;
	}

	if (pev->spawnflags == 0 &&
		(m_spawnAngles.x == -90 || m_spawnAngles.x == 270) &&
		m_spawnAngles.y == 0 &&
		m_spawnAngles.z == 0 &&
		pev->speed > 1000)
	{
		Vector center = (pev->absmin + pev->absmax) * 0.5f;
		Vector down = center;
		down.z = pev->absmin.z - 256.0f;
		TraceResult tr;
		UTIL_TraceLine(center, down, ignore_monsters, pTriggerPush->edict(), &tr);

		if (tr.pHit != nullptr && tr.flFraction < 1.0f)
		{
			const char* texture = TRACE_TEXTURE(tr.pHit, center, down);
			if (texture != nullptr && FStrEq(texture, VRGame::GetXenMoundTextureName()))
			{
				return true;
			}
		}
	}
	return false;
}
} // namespace

bool VRHandleFrictionTouch(CBaseEntity* pOther)
{
	if (pOther->pev->movetype == MOVETYPE_BOUNCEMISSILE || pOther->pev->movetype == MOVETYPE_BOUNCE)
		return false;

	// Allow disabling of func_friction in VR
	if (pOther->IsPlayer() && CVAR_GET_FLOAT("vr_disable_func_friction") != 0.f)
	{
		pOther->pev->friction = 1.f;
		return true;
	}

	return false;
}

void VROnChangeLevelNow(CBaseEntity* pPlayer)
{
	// Backup player VR offsets (fixes origin issues in roomscale) - Max Makes Mods, 2018-04-02
	if (pPlayer->IsPlayer())
	{
		reinterpret_cast<CBasePlayer*>(pPlayer)->StoreVROffsetsForLevelchange();
	}
}

bool VRForceInTransition(CBaseEntity* pEntity)
{
	// TODO: Always return 1 for players?
	// Can't think of a situation where a player wouldn't be in the transition volume
	// when touching a trigger_changelevel
	if (pEntity->IsPlayer())
	{
		CBasePlayer* pPlayer = dynamic_cast<CBasePlayer*>(pEntity);
		if (pPlayer && pPlayer->vr_didJustTeleportThroughChangeLevel)
			return true;
	}
	return false;
}

void VROnTeleported(CBaseEntity* pTrigger, CBaseEntity* pOther, const Vector& prevOrigin, const Vector& prevAngles)
{
	if (pOther->IsPlayer())
	{
		CBasePlayer* pPlayer = dynamic_cast<CBasePlayer*>(pOther);
		if (pPlayer)
		{
			pPlayer->VRJustTeleported(prevOrigin, prevAngles);
			VRGame::OnPlayerTeleported(pPlayer, pTrigger);
		}
	}
}

void VROnTriggerPushSpawn(CBaseEntity* pTriggerPush)
{
	g_vrTriggerPushSpawnAngles[EHandleT<CBaseEntity>{pTriggerPush}] = pTriggerPush->pev->angles;
}

// Called for every trigger_push after a map was loaded (VRPreThinkStart in VRPlayer.cpp)
bool VRCheckXenJumpPad(CBaseEntity* pTriggerPush)
{
	if (IsXenJumpPad(pTriggerPush))
	{
		// Find trigger_multiple
		CBaseEntity* pTriggerMultiple = nullptr;
		while ((pTriggerMultiple = UTIL_FindEntityByClassname(pTriggerMultiple, "trigger_multiple")) != nullptr)
		{
			if (UTIL_BBoxIntersectsBBox(pTriggerPush->pev->absmin, pTriggerPush->pev->absmax, pTriggerMultiple->pev->absmin, pTriggerMultiple->pev->absmax))
			{
				// Register the multi_manager for this xen mound at this position
				gGlobalXenMounds.Add((pTriggerPush->pev->absmin + pTriggerPush->pev->absmax) * 0.5f, pTriggerMultiple->pev->target);

				// Flag trigger_multiple and this trigger_push,
				// so players can disable xen jump push in VR (then only controller beam will activate the mound)
				g_vrXenJumpTriggers.insert(EHandleT<CBaseEntity>{pTriggerPush});
				g_vrXenJumpTriggers.insert(EHandleT<CBaseEntity>{pTriggerMultiple});
				return true;
			}
		}
	}
	pTriggerPush->SetThink(nullptr);
	return true;
}

bool VRIsXenJumpTrigger(CBaseEntity* pTrigger)
{
	return !g_vrXenJumpTriggers.empty() && g_vrXenJumpTriggers.count(EHandleT<CBaseEntity>{pTrigger}) != 0;
}

bool VRBlockXenJumpTouch(CBaseEntity* pTrigger, CBaseEntity* pOther)
{
	return pOther->IsPlayer() && VRIsXenJumpTrigger(pTrigger) && CVAR_GET_FLOAT("vr_xenjumpthingies_teleporteronly") != 0.f;
}

void VRClearSpecialEntities()
{
	g_vrXenJumpTriggers.clear();
	g_vrTriggerPushSpawnAngles.clear();
}

// Retina scanners: a func_wall (or a class derived from it) with the scanner texture, and the invisible func_button behind it.
// Called for every such wall after a map was loaded (VRPreThinkStart in VRPlayer.cpp), until it returns true.
bool VRCheckRetinaScanner(CBaseEntity* pWall)
{
	const model_t* model = VRGetBSPModel(pWall);
	if (model == nullptr)
	{
		// Repeat until GetBSPModel returns a value
		return false;
	}
	else
	{
		pWall->SetThink(nullptr);

		if (VRGame::ReplaceSpecialWall(pWall))
			return true;

		for (int i = 0; i < model->nummodelsurfaces; ++i)
		{
			const msurface_hl25_t& surface = *VRGetSurface(model, model->firstmodelsurface + i);
			if (surface.texinfo != nullptr && surface.texinfo->texture != nullptr && surface.texinfo->texture->name != nullptr)
			{
				std::string textureName = surface.texinfo->texture->name;
				std::transform(textureName.begin(), textureName.end(), textureName.begin(), ::tolower);
				if (textureName.find(VRGame::GetRetinaScannerTextureName()) != std::string::npos)
				{
					// We are a retina scanner!

					// Find our button! (func_button with rendermode kRenderTransTexture, renderamt 0 and model center not further away than 16 units away from our center)
					Vector retinaScannerCenter = (pWall->pev->absmin + pWall->pev->absmax) / 2.f;
					edict_t* pentButton = FIND_ENTITY_BY_CLASSNAME(nullptr, "func_button");
					while (!FNullEnt(pentButton))
					{
						// Don't add broken retina scanner after resonance cascade to list,
						// it must not react to eyes, but to use and controller touch
						bool isBrokenRetinaScannerAfterResonanceCascade = VRGame::IsDisabledRetinaScannerButton(pentButton);
						if (!isBrokenRetinaScannerAfterResonanceCascade)
						{
							// Add retina scanner and button to global list
							if (pentButton->v.renderamt == 0 && pentButton->v.rendermode == kRenderTransTexture)
							{
								Vector buttonScannerCenter = (pentButton->v.absmin + pentButton->v.absmax) * 0.5f;
								if ((buttonScannerCenter - retinaScannerCenter).Length() <= 16.f)
								{
									EHandleT<CBaseEntity> hButton = CBaseEntity::SafeInstance<CBaseEntity>(pentButton);
									g_vrRetinaScanners[EHandleT<CBaseEntity>{pWall}] = hButton;
									g_vrRetinaScannerButtons.insert(hButton);
									break;
								}
							}
						}

						pentButton = FIND_ENTITY_BY_CLASSNAME(pentButton, "func_button");
					}
					return true;
				}
			}
		}
	}

	return true;
}
