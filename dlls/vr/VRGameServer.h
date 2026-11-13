#pragma once

// Per-game data and hooks used by the shared server VR code.
// Every branch has all the VRGameServer_<game>.cpp files; the build compiles the one for its game
// (VR_GAME in linux/Makefile.hldll, the ClCompile entry in projects/vs2019/hldll.vcxproj).
// Game data (map names, entity names, per-map hacks) belongs there, not in the shared VR files.

class CBaseEntity;
class CBaseMonster;
class CBasePlayer;
struct edict_s;
struct entvars_s;

// How a brush entity crushes a player who is stuck in it (VRCrushEntityHandler.cpp)
enum class VRCrushMode
{
	NEVER,
	ALWAYS,
	WHEN_MOVING,
	WHEN_MOVING_DOWN,
	WHEN_MOVING_UP,
	HEALING_EXPLOIT
};

// A friendly NPC that reacts to the player's gestures and to being held at gunpoint (VRControllerInteractionManager.cpp)
struct VRAllyInfo
{
	// Sentence groups for a gun pointed at the NPC: first, second and third warning, then the NPC turns on the player
	const char* gunpointOne;
	const char* gunpointTwo;
	const char* gunpointThree;
	const char* gunpointAttack;
	// An armed NPC goes into combat state at the third warning (it draws its weapon)...
	bool isArmed;
	// ...and runs this schedule when the player stops aiming at it. nullptr if it has none.
	const char* disarmScheduleName;
};

namespace VRGame
{
// Crush mode of a brush model ("*63") on a map ("maps/c1a0c.bsp"). VRCrushMode::NEVER if not listed.
VRCrushMode GetCrushMode(const char* mapName, const char* brushModelName);

// Targetname of the intro ride train the player must stay in on this map, nullptr if the map has none.
const char* GetIntroTrainRideTargetName(const char* mapName);

// True if the brush entity with this targetname must never be used as the player's ground entity on this map.
bool IsExcludedGroundEntity(const char* mapName, const char* targetName);

// Called at the end of the player's restore from a savegame.
void OnPlayerRestored(CBasePlayer* pPlayer);

// Retina scanners (VRSpecialEntities.cpp)
// Part of the name of the texture that marks a func_wall as a retina scanner.
const char* GetRetinaScannerTextureName();
// True if this func_button behind a retina scanner must not be usable by looking into the scanner.
bool IsDisabledRetinaScannerButton(edict_s* pentButton);
// Chance to replace a func_wall by something else when a map was loaded. Returns true if it did.
bool ReplaceSpecialWall(CBaseEntity* pWall);

// Texture under a trigger_push that marks it as a xen jump pad (VRSpecialEntities.cpp), also used by the VR teleporter.
const char* GetXenMoundTextureName();

// Called when a trigger_push pushed an entity that touches it (the non-instant kind of push).
void OnTriggerPushTouched(CBaseEntity* pTriggerPush, CBaseEntity* pOther);

// Called after a trigger_teleport teleported the player.
void OnPlayerTeleported(CBasePlayer* pPlayer, CBaseEntity* pTrigger);

// The ally data for an entity classname, nullptr if entities of this class are not allies.
const VRAllyInfo* GetAllyInfo(const char* classname);

// Weapon sets, by weapon id (WEAPON_*)
// Weapons that shoot: NPCs react when one is pointed at them
bool IsWeaponWithRange(int iId);
// Any real weapon, as opposed to the bare hand
bool IsWeapon(int iId);
bool IsExplosiveWeapon(int iId);
bool IsWeaponWithVRLaserSpot(int iId);
// Weapons that do no visible harm when the player hits something with them (no blood)
bool IsSoftWeapon(int iId);
// Damage of hitting something with the weapon in the hand, as a factor of the crowbar's damage
float GetMeleeDamageScale(int iId);
// Damage type (DMG_*) of hitting something with the weapon in the hand
int GetMeleeDamageType(int iId);

// Called at the start of the player's PreThink.
void OnPlayerPreThinkStart(CBasePlayer* pPlayer);

// True for brush entities that look like a controllable train but must not be usable as one
// (the trash compactors in Half-Life, which the vanilla game lets the player "drive").
bool IsTrashCompactor(CBaseEntity* pEntity);

// Achievement and statistics triggers, and per-map fixes. Empty functions for a game that has none.
// The comment of each says where it is called from; the Half-Life code was moved from there verbatim.
// From CHeadCrab::BaseBalled in headcrab.cpp (there: pev is the headcrab's)
void OnHeadcrabBaseBalled(CBaseEntity* pHeadcrab, CBaseEntity* pPlayer);
// From CBaseButton::TriggerAndWait in buttons.cpp (there: pev is the button's, m_hActivator is pActivator)
void OnButtonTriggered(CBaseEntity* pButton, CBaseEntity* pActivator);
// From CBasePlayer::TakeDamage in player.cpp (there: this is the player)
void OnPlayerTakeDamage(CBasePlayer* pPlayer, entvars_s* pevInflictor);
// From CBasePlayer::PlayerUse in player.cpp, after the tank and train checks (there: this is the player)
void OnPlayerUsePressed(CBasePlayer* pPlayer);
// From CBarnacle::BarnacleThink in barnacle.cpp, when the tongue has grabbed pTouchEnt
void OnBarnacleTongueTouch(CBaseEntity* pBarnacle, CBaseEntity* pTouchEnt);
// From CItemSoda::Drink in effects.cpp
void OnSodaDrunk(CBaseEntity* pDrinker);
// From CBarney::GetSchedule in barney.cpp (there: this is the barney)
void OnBarneyGetSchedule(CBaseMonster* pBarney);
// From CBreakable::Die in func_break.cpp (there: pev is the breakable's, killedByTrigger is m_killMethod == KillMethod::TRIGGER)
void OnBreakableDie(CBaseEntity* pBreakable, bool killedByTrigger);
// From CBasePlayer::Restore in player.cpp, the else branch of "landmark isn't present" (there: this is the player)
void OnPlayerRestoredWithLandmark(CBasePlayer* pPlayer);
// From CBasePlayer::PlayerUse in player.cpp and VRPreThinkControllersAndTrain in VRPlayer.cpp, when the player starts controlling a train
void OnPlayerUsedTrain(CBasePlayer* pPlayer);
// From CTalkMonster::GibAttack in VRGibs.cpp
void OnGibCaught(CBaseEntity* pMonster, CBaseEntity* thrower);
// From VRControllerInteractionManager::CheckAndPressButtons; the hook is called when the bare hand touches an entity while the grip is held and nothing is dragged
void OnBareHandPetting(CBasePlayer* pPlayer, CBaseEntity* pEntity);
// From VRCrushEntityHandler::CheckEntAndMaybeCrushPlayer (there: dmg is pEntity->pev->dmg)
void OnPlayerTakeNegativeCrushDamage(CBaseEntity* pPlayer, float dmg);
// From VRGroundEntityHandler::DetectAndSetGroundEntity, when the player is outside the intro train more than 2 seconds after spawning
void OnPlayerOutsideIntroTrain(CBasePlayer* pPlayer);
// From CBasePlayer::VRKilled in VRPlayer.cpp (there: this is the player)
void OnPlayerKilled(CBasePlayer* pPlayer, entvars_s* pevAttacker);

// Game name the server reports before the game rules exist (GetGameDescription in client.cpp).
const char* GetGameDescription();

// Save version marker (VRServerHooks.cpp): a savegame loads only if its magic and version match.
// Increment the version every time the savedata of entities is changed.
int GetSaveMagic();
int GetSaveVersion();
// Console message for a savegame that was rejected, with a newline at the end.
const char* GetSaveIncompatibleMessage();
// Map the server changes to after a savegame was rejected (the engine can't cancel a load).
const char* GetRestoreFailedFallbackMap();
} // namespace VRGame
