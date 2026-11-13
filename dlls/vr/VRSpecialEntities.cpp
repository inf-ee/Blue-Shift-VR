// VR handling of triggers, called from triggers.cpp through the hooks in VRHooks.h

#include "VRHooks.h"

#include <unordered_map>
#include <unordered_set>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"

#include "VRCommons.h"
#include "VRGameServer.h"

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
