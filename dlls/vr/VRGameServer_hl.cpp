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
#include "weapons.h"

#include "VRGameServer.h"
#include "vr/VRShared.h"

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

// Achievement and statistics triggers and map fixes of Half-Life: VR, disabled (commented out) as they were
// in the files they were moved from. They need VRAchievementsAndStatsTracker and UTIL_VRGiveAchievement from Half-Life: VR.

// From VRPlayer.cpp. The check was disabled there (commented out, returning false).
bool VRGame::IsTrashCompactor(CBaseEntity* pEntity)
{
	// if (!pEntity)
	// 	return false;

	// std::string modelname = STRING(pEntity->pev->model);

	// if (modelname.empty() || modelname[0] != '*')
	// 	return false;

	// std::string mapname = STRING(INDEXENT(0)->v.model);

	// if (mapname != std::string{ "maps/c2a3e.bsp" })
	// 	return false;

	// return modelname == "*6" || modelname == "*13";
	return false;
}

// From CHeadCrab::BaseBalled in headcrab.cpp (there: pev is the headcrab's)
void VRGame::OnHeadcrabBaseBalled(CBaseEntity* pHeadcrab, CBaseEntity* pPlayer)
{
	// if baseballed by player in mid-air, give achievement
	// if (!FBitSet(pev->flags, FL_ONGROUND) && pPlayer && pPlayer->IsNetClient())
	// {
	// 	UTIL_VRGiveAchievement(pPlayer, VRAchievement::GEN_BASEBALLED);
	// }
}

// From CBaseButton::TriggerAndWait in buttons.cpp (there: pev is the button's, m_hActivator is pActivator)
void VRGame::OnButtonTriggered(CBaseEntity* pButton, CBaseEntity* pActivator)
{
	// // used by player
	// if (m_hActivator && m_hActivator->IsNetClient())
	// {
	// 	// entrance map
	// 	if (FStrEq(STRING(INDEXENT(0)->v.model), "maps/c1a0.bsp"))
	// 	{
	// 		// button for that laptop with "important messages"
	// 		if (FStrEq(STRING(pev->target), "introroomgizmomm"))
	// 		{
	// 			UTIL_VRGiveAchievement(m_hActivator, VRAchievement::AM_IMPORTANTMSG);
	// 		}
	// 		// alarm button on barney's desk
	// 		else if (FStrEq(STRING(pev->target), "buzzerlightsonmm"))
	// 		{
	// 			UTIL_VRGiveAchievement(m_hActivator, VRAchievement::AM_TROUBLE);
	// 		}
	// 		// light switch in that one office
	// 		else if (FStrEq(STRING(pev->target), "office1mm"))
	// 		{
	// 			UTIL_VRGiveAchievement(m_hActivator, VRAchievement::AM_LIGHTSOFF);
	// 		}
	// 	}
	// 	// rocket map
	// 	else if (FStrEq(STRING(INDEXENT(0)->v.model), "maps/c2a2h.bsp"))
	// 	{
	// 		// button that launches rocket
	// 		if (FStrEq(STRING(pev->target), "launch_countdownmm"))
	// 		{
	// 			UTIL_VRGiveAchievement(m_hActivator, VRAchievement::OAR_INFINITY);
	// 		}
	// 	}
	// 	// blast pit map
	// 	else if (FStrEq(STRING(INDEXENT(0)->v.model), "maps/c1a4i.bsp"))
	// 	{
	// 		// button that launches rocket
	// 		if (FStrEq(STRING(pev->target), "init_rocket_fire"))
	// 		{
	// 			VRAchievementsAndStatsTracker::PlayerLaunchedTentacleRocketFire(m_hActivator);
	// 		}
	// 	}
	// }
}

// From CBasePlayer::TakeDamage in player.cpp (there: this is the player)
void VRGame::OnPlayerTakeDamage(CBasePlayer* pPlayer, entvars_s* pevInflictor)
{
	// //  start of office complex
	// if (FStrEq(STRING(INDEXENT(0)->v.model), "maps/c1a2.bsp"))
	// {
	// 	// ceiling light that zaps the player
	// 	if (FStrEq(STRING(pevInflictor->classname), "trigger_hurt") && FStrEq(STRING(pevInflictor->targetname), "kill_player_zap"))
	// 	{
	// 		UTIL_VRGiveAchievement(this, VRAchievement::OC_OSHAVIOLATION);
	// 	}
	// 	// fan that kills player in vents if not ducked
	// 	else if (FStrEq(STRING(pevInflictor->classname), "func_rotating") && FStrEq(STRING(pevInflictor->model), "*76"))
	// 	{
	// 		UTIL_VRGiveAchievement(this, VRAchievement::OC_DUCK);
	// 	}
	// }
}

// From CBasePlayer::PlayerUse in player.cpp, after the tank and train checks (there: this is the player)
void VRGame::OnPlayerUsePressed(CBasePlayer* pPlayer)
{
	// // Check if player tries to use the trash compactors as train (which is possible in vanilla halflife)
	// CBaseEntity* pMaybeTrash = CBaseEntity::InstanceOrWorld(pev->groundentity);
	// if (IsTrashCompactor(pMaybeTrash))
	// {
	// 	UTIL_VRGiveAchievement(this, VRAchievement::HID_NOTTRAINS);
	// }
}

// From CBarnacle::BarnacleThink in barnacle.cpp, when the tongue has grabbed pTouchEnt
void VRGame::OnBarnacleTongueTouch(CBaseEntity* pBarnacle, CBaseEntity* pTouchEnt)
{
	// // check if a scientist walked into the barnacle tongue while following a player, or after being pushed by the player
	// if (FClassnameIs(pTouchEnt->pev, "monster_scientist"))
	// {
	// 	CBaseMonster* pMonster = dynamic_cast<CBaseMonster*>(pTouchEnt);
	// 	if (pMonster)
	// 	{
	// 		// scientists use bits_COND_SPECIAL1 as bits_COND_CLIENT_PUSH
	// 		if (pMonster->HasConditions(bits_COND_SPECIAL1) || pMonster->IsFollowing())
	// 		{
	// 			UTIL_VRGiveAchievementAll(VRAchievement::HID_ROPES);
	// 		}
	// 	}
	// }
}

// From CItemSoda::Drink in effects.cpp
void VRGame::OnSodaDrunk(CBaseEntity* pDrinker)
{
	// if (pDrinker && pDrinker->IsNetClient())
	// {
	// 	UTIL_VRGiveAchievement(pDrinker, VRAchievement::GEN_REFRESHING);
	// }
}

// From CBarney::GetSchedule in barney.cpp (there: this is the barney)
void VRGame::OnBarneyGetSchedule(CBaseMonster* pBarney)
{
	// if (m_MonsterState != MONSTERSTATE_SCRIPT && m_MonsterState != MONSTERSTATE_PRONE && !m_pCine && IsAlive())
	// {
	// 	// we are the barney at the start of the first Residue Processing level, who runs into a barnacle.
	// 	// if we are alive without a script, a player probably saved us
	// 	if (FStrEq(STRING(INDEXENT(0)->v.model), "maps/c2a4a.bsp")
	// 		&& FStrEq(STRING(pev->targetname), "barn_run_barn"))
	// 	{
	// 		VRAchievementsAndStatsTracker::ResidueBarneyIsAlive();
	// 	}
	// }
}

// From CBreakable::Die in func_break.cpp (there: pev is the breakable's, killedByTrigger is m_killMethod == KillMethod::TRIGGER)
void VRGame::OnBreakableDie(CBaseEntity* pBreakable, bool killedByTrigger)
{
	// destroyed by trigger
	// if (m_killMethod == KillMethod::TRIGGER)
	// {
	// 	// map with kitchen
	// 	if (FStrEq(STRING(INDEXENT(0)->v.model), "maps/c1a0d.bsp"))
	// 	{
	// 		// microwave stew
	// 		if (FStrEq(STRING(pev->targetname), "stewgibs"))
	// 		{
	// 			UTIL_VRGiveAchievementAll(VRAchievement::AM_MAGNUSSON);
	// 		}
	// 	}
	// }

	// // Blast Pit map outside tentacles/rocket silo
	// if (FStrEq(STRING(INDEXENT(0)->v.model), "maps/c1a4b.bsp"))
	// {
	// 	if (FStrEq(STRING(pev->targetname), "barrel_explosion_1")
	// 		|| FStrEq(STRING(pev->targetname), "barrel_explosion_1d")
	// 		|| FStrEq(STRING(pev->targetname), "barrel_explosion_1e"))
	// 	{
	// 		VRAchievementsAndStatsTracker::PlayerDestroyedBlastPitBridge();
	// 	}
	// }
	// // Blast Pit map with that big fan
	// else if (FStrEq(STRING(INDEXENT(0)->v.model), "maps/c1a4e.bsp"))
	// {
	// 	// grate above big fan, if this breaks, player solved puzzle or cheated
	// 	if (FStrEq(STRING(pev->model), "*24"))
	// 	{
	// 		VRAchievementsAndStatsTracker::PlayerSolvedBlastPitFan();
	// 	}
	// }
}

// From CBasePlayer::Restore in player.cpp, the else branch of "landmark isn't present" (there: this is the player)
void VRGame::OnPlayerRestoredWithLandmark(CBasePlayer* pPlayer)
{
	// else
	// {
	// 	// Fix weird bug that puts players slightly too low under the actual spawn location in c4a1f
	// 	if (STRING(INDEXENT(0)->v.model) == std::string{ "maps/c4a1f.bsp" })
	// 	{
	// 		edict_t* pentLandmark = UTIL_FindLandmark("c4a1f");
	// 		if (pentLandmark)
	// 		{
	// 			pev->origin = VARS(pentLandmark)->origin + Vector(0, 0, 1);
	// 		}
	// 	}
	// }
}

// From CBasePlayer::PlayerUse in player.cpp and VRPreThinkControllersAndTrain in VRPlayer.cpp, when the player starts controlling a train
void VRGame::OnPlayerUsedTrain(CBasePlayer* pPlayer)
{
	// VRAchievementsAndStatsTracker::PlayerUsedTrain(this);
}

// From CTalkMonster::GibAttack in VRGibs.cpp
void VRGame::OnGibCaught(CBaseEntity* pMonster, CBaseEntity* thrower)
{
	// if (thrower->IsNetClient())
	// {
	// 	UTIL_VRGiveAchievement(thrower, VRAchievement::GEN_CATCH);
	// }
}

// From VRControllerInteractionManager::CheckAndPressButtons; the hook is called when the bare hand touches an entity while the grip is held and nothing is dragged
void VRGame::OnBareHandPetting(CBasePlayer* pPlayer, CBaseEntity* pEntity)
{
	// The rest of the original condition is checked by the caller:
	// && !controller.HasDraggedEntity() && isTouching && (isDragging || controller.IsDragging()) && controller.GetWeaponId() == WEAPON_BAREHAND
	if (FClassnameIs(pEntity->pev, "monster_houndeye"))
	{
		// UTIL_VRGiveAchievement(pPlayer, VRAchievement::GEN_PETDOGGO);
	}
}

// From VRCrushEntityHandler::CheckEntAndMaybeCrushPlayer (there: dmg is pEntity->pev->dmg)
void VRGame::OnPlayerTakeNegativeCrushDamage(CBaseEntity* pPlayer, float dmg)
{
	// VRAchievementsAndStatsTracker::PlayerTakeNegativeCrushDamage(pPlayer, pEntity->pev->dmg);
}

// From VRGroundEntityHandler::DetectAndSetGroundEntity, when the player is outside the intro train more than 2 seconds after spawning
void VRGame::OnPlayerOutsideIntroTrain(CBasePlayer* pPlayer)
{
	// UTIL_VRGiveAchievement(m_pPlayer, VRAchievement::HID_HOW);
}

// From CBasePlayer::VRKilled in VRPlayer.cpp (there: this is the player)
void VRGame::OnPlayerKilled(CBasePlayer* pPlayer, entvars_s* pevAttacker)
{
	// if (pevAttacker
	// 	&& FClassnameIs(pevAttacker, "func_tank")
	// 	&& (FStrEq(STRING(INDEXENT(0)->v.model), "maps/c2a5c.bsp") || FStrEq(STRING(INDEXENT(0)->v.model), "maps/c2a5e.bsp") || FStrEq(STRING(INDEXENT(0)->v.model), "maps/c2a2g.bsp"))
	// 	&& (FStrEq(STRING(pevAttacker->targetname), "sniper1") || FStrEq(STRING(pevAttacker->targetname), "sniper2")))
	// {
	// 	UTIL_VRGiveAchievement(this, VRAchievement::GEN_SNIPED);
	// }
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

// From CFuncWall::CheckIsSpecialVREntity in bmodels.cpp
bool VRGame::IsDisabledRetinaScannerButton(edict_s* pentButton)
{
	// The retina scanner that is broken after the resonance cascade
	return FStrEq(STRING(pentButton->v.target), "broken_airlockmm") && FStrEq(STRING(INDEXENT(0)->v.model), "maps/c1a0c.bsp");
}

// From CFuncWall::CheckIsSpecialVREntity in bmodels.cpp (there: pev is the wall's)
bool VRGame::ReplaceSpecialWall(CBaseEntity* pWall)
{
	// If we are the coffee cup and flask in gordon's locker, replace us with the Worlds Smallest Cup (easter egg)
	// see https://www.youtube.com/watch?v=M_AnIizeH1Q
	if (strcmp(STRING(pWall->pev->model), "*67") == 0 && FStrEq(STRING(INDEXENT(0)->v.model), "maps/c1a0d.bsp"))
	{
		edict_t* pentCup = CREATE_NAMED_ENTITY(MAKE_STRING("vr_easteregg"));
		if (!FNullEnt(pentCup))
		{
			pentCup->v.origin = (pWall->pev->absmin + pWall->pev->absmax) * 0.5f;
			DispatchSpawn(pentCup);
		}
		pWall->SetThink(&CBaseEntity::SUB_Remove);
		pWall->pev->nextthink = gpGlobals->time;
		return true;
	}
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

// From CBasePlayer::VRPreThinkStart in VRPlayer.cpp (there: this is the player). The code that was live there is
// commented out here as well: it uses the private members m_vrHasSurfacedInThatMapWithTheTank and
// m_vrJustGotYeetedByBPTrain of CBasePlayer, which nothing sets while the achievements are disabled.
void VRGame::OnPlayerPreThinkStart(CBasePlayer* pPlayer)
{
	// VRAchievementsAndStatsTracker::Update(this);

	// if (!m_vrHasSurfacedInThatMapWithTheTank)
	// {
		// if (pev->origin.z > 0.f && FStrEq(STRING(INDEXENT(0)->v.model), "maps/c2a5b.bsp"))
		// {
		// 	m_vrHasSurfacedInThatMapWithTheTank = true;
		// 	VRAchievementsAndStatsTracker::PlayerSurfacedInThatMapWithTheTank(this);
		// }
	// }

	// if (m_vrJustGotYeetedByBPTrain && FBitSet(pev->flags, FL_ONGROUND))
	// {
		// we landed after getting yeeted by that train at the beginning of Blast Pit
	// 	m_vrJustGotYeetedByBPTrain = false;

		// // sanity check that map is still correct
		// if (FStrEq(STRING(INDEXENT(0)->v.model), "maps/c1a4.bsp"))
		// {
		// 	// check that player landed roughly in pipe opening
		// 	if (pev->absmin.z > -3290.f && pev->absmin.z < -3130.f
		// 		&& pev->origin.x > -990.f && pev->origin.x < -780.f
		// 		&& pev->origin.y > -2610.f && pev->origin.y < -2470.f)
		// 	{
		// 		UTIL_VRGiveAchievement(this, VRAchievement::BP_PERFECT);
		// 	}
		// }
	// }

	// extern bool VRGlobalGetNoclipMode();
	// if (VRGlobalGetNoclipMode())
	// {
		// VRAchievementsAndStatsTracker::PlayerUsedNoclip(this);
	// }
}
