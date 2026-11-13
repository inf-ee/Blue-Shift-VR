
// Methods for handling entities that should crush the player (since getting killed when stuck is disabled due to bugs with VR mode)

#include <unordered_map>
#include <string>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "com_model.h"

#include "VRGameServer.h"

class VRCrushEntityHandler
{
private:
	// Determines the crush mode of an entity
	VRCrushMode GetCrushModeFromEntity(CBaseEntity* pEntity);

public:
	bool CheckEntAndMaybeCrushPlayer(CBaseEntity* pEntity, CBaseEntity* pPlayer);
};

VRCrushEntityHandler gVRCrushEntityHandler;

// called by pm_shared.cpp when we are stuck on an entity (pm_shared will unstuck us, but we use this here to determine if we should get killed)
bool VRNotifyStuckEnt(int player, int ent)
{
	if (ent <= 0)
		return false;

	CBaseEntity* pPlayer = UTIL_PlayerByIndex(player + 1);
	if (pPlayer == nullptr)
		return false;

	CBaseEntity* pEntity = CBaseEntity::SafeInstance<CBaseEntity>(INDEXENT(ent));
	if (pEntity == nullptr)
		return false;

	return gVRCrushEntityHandler.CheckEntAndMaybeCrushPlayer(pEntity, pPlayer);
}

VRCrushMode VRCrushEntityHandler::GetCrushModeFromEntity(CBaseEntity* pEntity)
{
	if (0 == pEntity->pev->model)
		return VRCrushMode::NEVER;

	std::string modelname = STRING(pEntity->pev->model);

	if (modelname.empty() || modelname[0] != '*')
		return VRCrushMode::NEVER;

	extern const model_t* VRGetBSPModel(CBaseEntity * pEntity);
	const model_t* mapmodel = VRGetBSPModel(CWorld::InstanceOrWorld(ENT(0)));

	if (mapmodel == nullptr || 0 != mapmodel->needload)
		return VRCrushMode::NEVER;

	return VRGame::GetCrushMode(mapmodel->name, modelname.c_str());
}

bool VRCrushEntityHandler::CheckEntAndMaybeCrushPlayer(CBaseEntity* pEntity, CBaseEntity* pPlayer)
{
	// already dead
	if (pPlayer->pev->deadflag != DEAD_NO)
		return true; // dont unstuck

	VRCrushMode crushMode = GetCrushModeFromEntity(pEntity);

	// healing bug entities
	if (crushMode == VRCrushMode::HEALING_EXPLOIT &&
		CVAR_GET_FLOAT("sv_cheats") != 0.f &&
		CVAR_GET_FLOAT("vr_cheat_enable_healing_exploit") != 0.f &&
		pEntity->pev->dmg < 0.f)
	{
		VRGame::OnPlayerTakeNegativeCrushDamage(pPlayer, pEntity->pev->dmg);
		pPlayer->TakeDamage(pEntity->pev, pEntity->pev, pEntity->pev->dmg, DMG_CRUSH);
		return true; // dont unstuck
	}

	bool crush = false;

	switch (crushMode)
	{
	case VRCrushMode::ALWAYS:
		crush = true;
		break;

	case VRCrushMode::WHEN_MOVING:
		crush = pEntity->pev->velocity.LengthSquared() > 0.f || pEntity->pev->avelocity.LengthSquared() > 0.f;
		break;

	case VRCrushMode::WHEN_MOVING_DOWN:
		crush = pEntity->pev->velocity.z < 0.f;
		break;

	case VRCrushMode::WHEN_MOVING_UP:
		crush = pEntity->pev->velocity.z > 0.f;
		break;

	case VRCrushMode::NEVER:
	default:
		crush = false;
		break;
	}

	if (crush)
	{
		pPlayer->TakeDamage(pEntity->pev, pEntity->pev, pPlayer->pev->health + pPlayer->pev->armorvalue + pEntity->pev->dmg + 1000.f, DMG_CRUSH | DMG_ALWAYSGIB);
		return true; // dont unstuck
	}

	return false;
}
