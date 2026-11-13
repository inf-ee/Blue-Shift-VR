// Half-Life data and hooks for the server VR code, see VRGameServer.h
// Moved verbatim from Half-Life: VR code (VRCrushEntityHandler.cpp, VRGroundEntityHandler.cpp, player.cpp).
// Not compiled in the Blue Shift and Opposing Force builds.

#include <cstring>
#include <string>
#include <unordered_map>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"

#include "VRGameServer.h"

namespace
{
const std::unordered_map<std::string, std::unordered_map<std::string, VRCrushMode>> m_entityCrushModes{
		{
			{"maps/c1a0c.bsp",
				{{
					{"*63", VRCrushMode::WHEN_MOVING} // toppling over server rack
				}}},
			{"maps/c1a1c.bsp",
				{{
					{"*147", VRCrushMode::WHEN_MOVING}, // under water grinder thingies
					{"*148", VRCrushMode::WHEN_MOVING},
					{"*31", VRCrushMode::WHEN_MOVING},
					{"*32", VRCrushMode::WHEN_MOVING},
				}}},
			{"maps/c1a1f.bsp",
				{{
					{"*47", VRCrushMode::WHEN_MOVING} // toppling over server rack
				}}},
			{"maps/c1a2.bsp",
				{{
					{"*76", VRCrushMode::WHEN_MOVING} // that one evil ventilator
				}}},
			{"maps/c1a3.bsp",
				{{
					{"*18", VRCrushMode::NEVER} // ventilator in we got hostiles
				}}},
			{"maps/c1a3c.bsp",
				{{
					{"*1", VRCrushMode::NEVER} // ventilator in we got hostiles
				}}},
			{"maps/c1a4e.bsp",
				{{
					{"*1", VRCrushMode::WHEN_MOVING} // that huge ventilator near the tentacles
				}}},
			{"maps/c2a1.bsp",
				{{
					{"*97", VRCrushMode::WHEN_MOVING} // cave-in near gargantua before on a rail
				}}},
			{"maps/c2a3.bsp",
				{{
					{"*89", VRCrushMode::WHEN_MOVING} // cave-in under water after on a rail
				}}},
			{"maps/c2a3b.bsp",
				{{
					{"*4", VRCrushMode::NEVER}, // stompy thingies where that spit monster is
					{"*8", VRCrushMode::NEVER}, // these are the bottom ones and should not kill
					{"*9", VRCrushMode::NEVER},
					{"*10", VRCrushMode::WHEN_MOVING_DOWN}, // these are the top ones and should kill when moving down
					{"*11", VRCrushMode::WHEN_MOVING_DOWN},
					{"*12", VRCrushMode::WHEN_MOVING_DOWN},
				}}},
			{"maps/c2a3e.bsp",
				{{{"*6", VRCrushMode::WHEN_MOVING}, // the trash compactor walls
					{"*13", VRCrushMode::WHEN_MOVING}}}},
			{"maps/c2a4b.bsp",
				{{
					{"*3", VRCrushMode::NEVER},		// underwater ventilator players need to swim through
					{"*6", VRCrushMode::WHEN_MOVING}, // underwater grinder thingy players must avoid
					{"*9", VRCrushMode::WHEN_MOVING}, // underwater grinder thingy players must avoid
					{"*16", VRCrushMode::NEVER},		// underwater grinder thingy players must swim under

					{"*19", VRCrushMode::WHEN_MOVING_DOWN}, // stompy piston thingies on conveyor belts
					{"*20", VRCrushMode::WHEN_MOVING_DOWN},
					{"*21", VRCrushMode::WHEN_MOVING_DOWN},
					{"*22", VRCrushMode::WHEN_MOVING_DOWN},
					{"*23", VRCrushMode::WHEN_MOVING_DOWN},
					{"*24", VRCrushMode::WHEN_MOVING_DOWN},
				}}},
			{"maps/c2a4c.bsp",
				{{
					{"*4", VRCrushMode::WHEN_MOVING}, // the sideways murder thingies in residue processing
					{"*5", VRCrushMode::WHEN_MOVING},
					{"*6", VRCrushMode::WHEN_MOVING},

					{"*2", VRCrushMode::WHEN_MOVING_DOWN}, // the downwards stompy murder thingies in residue processing
					{"*3", VRCrushMode::WHEN_MOVING_DOWN},
					{"*46", VRCrushMode::WHEN_MOVING_DOWN},

					{"*29", VRCrushMode::WHEN_MOVING}, // the rolling murder thingies in residue processing
					{"*30", VRCrushMode::WHEN_MOVING},
					{"*31", VRCrushMode::WHEN_MOVING},

					{"*38", VRCrushMode::WHEN_MOVING}, // the crushy-bity murder thingies at the end of residue processing
					{"*39", VRCrushMode::WHEN_MOVING},
				}}},
			{"maps/c2a4e.bsp",
				{{{"*54", VRCrushMode::WHEN_MOVING}, // that super weird automated murder equipment in questionable ethics
					{"*56", VRCrushMode::WHEN_MOVING}}}},
			{"maps/c2a5.bsp",
				{{{"*133", VRCrushMode::WHEN_MOVING}, // underwater ventilators in the canyon dam (need to be shut off from control tower)
					{"*134", VRCrushMode::WHEN_MOVING}}}},
			{"maps/c2a5f.bsp",
				{{{"*139", VRCrushMode::HEALING_EXPLOIT}, // the healing exploit doors
					{"*144", VRCrushMode::HEALING_EXPLOIT}}}},
			{"maps/c2a5g.bsp",
				{{{"*95", VRCrushMode::WHEN_MOVING}, // the cars being kicked by gargantua in that underground parking lot
					{"*93", VRCrushMode::WHEN_MOVING},
					{"*96", VRCrushMode::WHEN_MOVING}}}},
			{"maps/c3a1.bsp",
				{{
					{"*8", VRCrushMode::WHEN_MOVING}, // cave-in ceiling pieces
					{"*9", VRCrushMode::WHEN_MOVING},
					{"*10", VRCrushMode::WHEN_MOVING},
					{"*11", VRCrushMode::WHEN_MOVING},
					{"*12", VRCrushMode::WHEN_MOVING},
					{"*13", VRCrushMode::WHEN_MOVING},
					{"*14", VRCrushMode::WHEN_MOVING},
					{"*27", VRCrushMode::WHEN_MOVING},
				}}},
			{"maps/c3a1b.bsp",
				{{
					{"*47", VRCrushMode::WHEN_MOVING}, // collapsing walls in that alien vs military warzone
					{"*48", VRCrushMode::WHEN_MOVING},
					{"*49", VRCrushMode::WHEN_MOVING},
				}}},
		}};
} // namespace

VRCrushMode VRGame::GetCrushMode(const char* mapName, const char* brushModelName)
{
	auto mapEntityCrushModes = m_entityCrushModes.find(mapName);
	if (mapEntityCrushModes != m_entityCrushModes.end())
	{
		auto entityCrushMode = mapEntityCrushModes->second.find(brushModelName);
		if (entityCrushMode != mapEntityCrushModes->second.end())
		{
			return entityCrushMode->second;
		}
	}

	return VRCrushMode::NEVER;
}

const char* VRGame::GetIntroTrainRideTargetName(const char* mapName)
{
	// The intro train ride (Half-Life: VR's IntroTrainRideMapNames, the train's targetname is "train")
	static const char* const introTrainRideMapNames[] = {
		"maps/c0a0.bsp",
		"maps/c0a0a.bsp",
		"maps/c0a0b.bsp",
		"maps/c0a0c.bsp",
		"maps/c0a0d.bsp"};

	for (const char* introTrainRideMapName : introTrainRideMapNames)
	{
		if (strcmp(mapName, introTrainRideMapName) == 0)
			return "train";
	}
	return nullptr;
}

bool VRGame::IsExcludedGroundEntity(const char* mapName, const char* targetName)
{
	return false;
}

void VRGame::OnPlayerRestored(CBasePlayer* pPlayer)
{
	// ugly but easy hack to ensure damage visuals don't disturb the end scene with gman
	// if player health is very low
	if (pPlayer->pev->health < 100)
	{
		std::string mapname = STRING(INDEXENT(0)->v.model);
		if (mapname == std::string{"maps/c5a1.bsp"})
		{
			pPlayer->pev->health = 100;
		}
	}
}

const char* VRGame::GetGameDescription()
{
	return "Half-Life";
}

int VRGame::GetSaveMagic()
{
	return 'HLVR';
}

int VRGame::GetSaveVersion()
{
	return 1;
}

const char* VRGame::GetSaveIncompatibleMessage()
{
	return "COULDN'T LOAD SAVEGAME, SAVE FILE IS INCOMPATIBLE WITH HALF-LIFE: VR!\n";
}

const char* VRGame::GetRestoreFailedFallbackMap()
{
	return "crossfire";
}

const char* VRGame::GetXenMoundTextureName()
{
	return "c2a5mound";
}

// From CTriggerPush::Touch in triggers.cpp (there: pev is the trigger's)
void VRGame::OnTriggerPushTouched(CBaseEntity* pTriggerPush, CBaseEntity* pOther)
{
	// if (pOther->IsNetClient())
	// {
	// 	CBasePlayer* pPlayer = dynamic_cast<CBasePlayer*>(pOther);

	// 	// player in residue processing map with all those conveyor belts interacts with
	// 	// trigger_push that player has to pass when taking the wrong turn and coming back to start
	// 	if (pPlayer
	// 		&& FStrEq(STRING(INDEXENT(0)->v.model), "maps/c2a4c.bsp")
	// 		&& FStrEq(STRING(pev->model), "*78"))
	// 	{
	// 		UTIL_VRGiveAchievement(pPlayer, VRAchievement::RP_MOEBIUS);
	// 	}
	// }
}

// From CBaseTrigger::TeleportTouch in triggers.cpp (there: pev is the trigger's)
void VRGame::OnPlayerTeleported(CBasePlayer* pPlayer, CBaseEntity* pTrigger)
{
	// endgame map
	// if (FStrEq(STRING(INDEXENT(0)->v.model), "maps/c5a1.bsp"))
	// {
	// 	// teleport at the exit of train to black screen (joined g-man)
	// 	if (FStrEq(STRING(pev->target), "endroom"))
	// 	{
	// 		UTIL_VRGiveAchievement(pPlayer, VRAchievement::END_LIMITLESS);
	// 	}
	// 	// teleport in the train to grunt army (refused g-man)
	// 	else if (FStrEq(STRING(pev->target), "loser"))
	// 	{
	// 		UTIL_VRGiveAchievement(pPlayer, VRAchievement::END_UNWINNABLE);
	// 	}
	// }
}
