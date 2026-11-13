// VR methods of CBasePlayer, moved from player.cpp

#include <algorithm>

#include "extdll.h"
#include "util.h"

#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "pm_shared.h"
#include "UserMessages.h"

#include "pm_defs.h"
#include "pm_movevars.h"
#include "vr/VRPhysicsHelper.h"
#include "vr/VRMovementHandler.h"
#include "vr/VRControllerModel.h"
#include "vr/VRCommons.h"
#include "vr/VRGameServer.h"
#include "trains.h"

// Train HUD states and TrainSpeed from player.cpp (SDK code local to that file), used by the VR train controls
#define TRAIN_ACTIVE 0x80
#define TRAIN_NEW 0xc0
#define TRAIN_OFF 0x00
#define TRAIN_NEUTRAL 0x01
#define TRAIN_SLOW 0x02
#define TRAIN_MEDIUM 0x03
#define TRAIN_FAST 0x04
#define TRAIN_BACK 0x05
int TrainSpeed(int iSpeed, int iMax);

constexpr const float VR_RETINASCANNER_ACTIVATE_LOOK_TIME = 1.f;
std::unordered_map<EHandleT<CBaseEntity>, EHandleT<CBaseEntity>, EHandleT<CBaseEntity>::Hash, EHandleT<CBaseEntity>::Equal> g_vrRetinaScanners;
std::unordered_set<EHandleT<CBaseEntity>, EHandleT<CBaseEntity>::Hash, EHandleT<CBaseEntity>::Equal> g_vrRetinaScannerButtons;

// Set by ClientPrecache on every map load (new game, changelevel, load save), checked in PreThink
bool g_vrNeedRecheckForSpecialEntities = true;

bool CBasePlayer::CheckVRTRainButtonTouched(const Vector& buttonLeftPos, const Vector& buttonRightPos)
{
	for (auto& [id, controller] : m_vrControllers)
	{
		if (controller.IsValid())
		{
			if (VRPhysicsHelper::Instance().ModelIntersectsLine(controller.GetModel(), buttonLeftPos, buttonRightPos))
			{
				controller.AddTouch(VRController::TouchType::LIGHT_TOUCH, 0.1f);
				return true;
			}
		}
	}
	return false;
}

// VR methods:
void CBasePlayer::UpdateVRHeadset(const int timestamp, const Vector2D& hmdOffset, const float hmdOffsetZ, const Vector& hmdForward, const Vector2D& hmdYawOffsetDelta, float prevYaw, float currentYaw, bool hasReceivedRestoreYawMsg, bool hasReceivedSpawnYaw)
{
	// Filter out outdated updates
	if (timestamp <= vr_hmdLastUpdateClienttime && vr_hmdLastUpdateServertime >= gpGlobals->time)
	{
		return;
	}

	// Ignore all updates until client has processed yaw update (fixes timing issues)
	if (vr_needsToSendRestoreYawMsgToClient)
	{
		return;
	}
	if (vr_hasSentRestoreYawMsgToClient && !hasReceivedRestoreYawMsg)
	{
		return;
	}
	vr_hasSentRestoreYawMsgToClient = false;

	Vector TEMPDEBUG_originBefore = pev->origin;

	// Calculate view dir for view position offset
	Vector viewDir2D;
	UTIL_MakeVectorsPrivate(Vector{0.f, pev->angles.y, 0.f}, viewDir2D, nullptr, nullptr);
	viewDir2D.z = 0.f;
	viewDir2D = viewDir2D.Normalize();

	// Get view dir with length to hull bounds
	float vrViewOffsetDistToHullBounds = CVAR_GET_FLOAT("vr_view_dist_to_walls");
	Vector viewDirToHullBounds = viewDir2D * (VEC_HULL_MAX.x - vrViewOffsetDistToHullBounds);

	// If we just spawned we need to make sure that the HMD offset gets moved into the spawn position
	// Thus we set vr_lastHMDOffset to the current HMD offset,
	// and use the current HMD offset to set the client origin offset.
	// We also send the initial spawn yaw down to the client so it adjusts and looks in the direction of the spawn spot.
	// - Max Makes Mods, 2019-04-13
	if (vr_IsJustSpawned)
	{
		if (!vr_hasSentSpawnYawToClient)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgVRSetSpawnYaw, nullptr, pev);
			WRITE_ANGLE(vr_spawnYaw);
			MESSAGE_END();
			vr_spawnYaw = 0.f;
			vr_hasSentSpawnYawToClient = true;
			return;
		}
		if (vr_hasSentSpawnYawToClient && !hasReceivedSpawnYaw)
		{
			return;
		}
		vr_hasSentSpawnYawToClient = false;

		vr_lastHMDOffset = Vector{hmdOffset.x, hmdOffset.y, 0.f};
		vr_ClientOriginOffset = Vector{-hmdOffset.x, -hmdOffset.y, 0.f} + viewDirToHullBounds;
		vr_IsJustSpawned = false;

		return;
	}

	// We probably don't need this?
	if (vr_IsJustRestored)
	{
		//vr_ClientOriginOffset = vr_ClientOriginOffset + viewDir2D * VEC_HULL_MAX.x;
		vr_IsJustRestored = false;
	}

	vr_prevYaw = prevYaw;
	vr_currentYaw = currentYaw;

	vr_hmdLastUpdateClienttime = timestamp;
	vr_hmdLastUpdateServertime = gpGlobals->time;

	// First get origin where the client thinks it is:
	Vector clientOrigin = GetClientOrigin();

	// Add rotation offset delta into client origin (yaw rotates around play area center, so we need to adjust this here):
	clientOrigin = clientOrigin - hmdYawOffsetDelta;

	// Then get headset position:
	Vector newOrigin = clientOrigin + hmdOffset;

	// push origin back so that view position is on border of player's bounding box
	newOrigin = newOrigin - viewDirToHullBounds;

	// Use movement handler to move to new position (instead of simply teleporting)
	// Uses pm_shared code. Allows for climbing up stairs and handling all kinds of collisions with level geometry.
	pev->origin = VRMovementHandler::DoMovement(pev->origin, newOrigin, this);

	if (newOrigin.x != pev->origin.x || newOrigin.y != pev->origin.y)
	{
		//ALERT(at_console, "newOrigin != pev->origin: %f %f, %f %f\n", newOrigin.x, newOrigin.y, pev->origin.x, pev->origin.y);
		//ALERT(at_console, "Sending gmsgVRWalkedIntoWall\n");

		// tell client we walked into a wall
		MESSAGE_BEGIN(MSG_ONE, gmsgVRWalkedIntoWall, nullptr, pev);
		MESSAGE_END();
	}
	else
	{
		//ALERT(at_console, "newOrigin == pev->origin: %f %f, %f %f\n", newOrigin.x, newOrigin.y, pev->origin.x, pev->origin.y);
	}

	// Always add in newOrigin instead of pev->origin, creates much better and smoother results
	vr_ClientOriginOffset.x = clientOrigin.x - newOrigin.x;
	vr_ClientOriginOffset.y = clientOrigin.y - newOrigin.y;

	// Remember offset for wallcheck next call
	vr_lastHMDOffset.x = hmdOffset.x;
	vr_lastHMDOffset.y = hmdOffset.y;

	// HMD height and HMD direction for view_ofs and viewdir for looking at and interaction with stuff
	pev->view_ofs.z = pev->mins.z + hmdOffsetZ;
	vr_hmdForward = hmdForward;
	vr_hmdForward = vr_hmdForward.Normalize();
}

void CBasePlayer::UpdateVRController(const VRControllerID vrControllerID, const int timestamp, const bool isValid, const bool isMirrored, const Vector& offset, const Vector& angles, const Vector& velocity, bool isDragging, bool isFiring)
{
	int weaponId = WEAPON_BAREHAND;
	if (vrControllerID == VRControllerID::WEAPON)
	{
		ItemInfo itemInfo = {};
		if (m_pActiveItem != nullptr && m_pActiveItem->GetItemInfo(&itemInfo))
		{
			weaponId = itemInfo.iId;
		}
	}

	bool wasDragging = m_vrControllers[vrControllerID].IsDragging();

	m_vrControllers[vrControllerID].Update(this, timestamp, isValid, isMirrored, offset, angles, velocity, isDragging, isFiring, vrControllerID, weaponId);

	if (vrControllerID == VRControllerID::HAND && isValid && isDragging && !wasDragging && !m_vrControllers[vrControllerID].HasDraggedEntity())
	{
		constexpr const float VR_FLASHLIGHT_TOGGLE_DISTANCE_TO_HEAD = 16.f;
		float distance = (m_vrControllers[vrControllerID].GetPosition() - this->EyePosition()).Length();
		if (distance < VR_FLASHLIGHT_TOGGLE_DISTANCE_TO_HEAD + (std::max)(0.f, CVAR_GET_FLOAT("vr_view_dist_to_walls")))
		{
			if (FlashlightIsOn())
			{
				FlashlightTurnOff();
			}
			else
			{
				FlashlightTurnOn();
			}
		}
	}
}

const Vector CBasePlayer::GetWeaponPosition()
{
	if (m_vrControllers[VRControllerID::WEAPON].IsValid())
	{
		return GetClientOrigin() + m_vrControllers[VRControllerID::WEAPON].GetOffset();
	}
	else
	{
		return EyePosition();
	}
}

const Vector CBasePlayer::GetWeaponAngles()
{
	if (m_vrControllers[VRControllerID::WEAPON].IsValid())
	{
		return m_vrControllers[VRControllerID::WEAPON].GetAngles();
	}
	else
	{
		return UTIL_VecToAngles(vr_hmdForward);
	}
}

const Vector CBasePlayer::GetWeaponViewAngles()
{
	Vector angles = GetWeaponAngles();
	angles.x = -angles.x;
	return angles;
}

const Vector CBasePlayer::GetWeaponVelocity()
{
	if (m_vrControllers[VRControllerID::WEAPON].IsValid())
	{
		return m_vrControllers[VRControllerID::WEAPON].GetVelocity();
	}
	else
	{
		return Vector{};
	}
}

const Vector CBasePlayer::GetClientOrigin()
{
	return Vector(pev->origin.x + vr_ClientOriginOffset.x, pev->origin.y + vr_ClientOriginOffset.y, pev->origin.z);
}

const Vector CBasePlayer::GetClientViewOfs()
{
	return Vector((pev->origin + pev->view_ofs) - GetClientOrigin());
}

bool CBasePlayer::IsWeaponUnderWater()
{
	return UTIL_PointContents(GetWeaponPosition()) == CONTENTS_WATER || UTIL_PointContents(GetGunPosition()) == CONTENTS_WATER;
}

#define CROWBAR_BODYHIT_VOLUME 128
#define CROWBAR_WALLHIT_VOLUME 512
void CBasePlayer::PlayMeleeSmackSound(CBaseEntity* pSmackedEntity, const int weaponId, const Vector& pos, const Vector& velocity)
{
	TraceResult fakeTrace = {0};
	fakeTrace.pHit = pSmackedEntity->edict();
	fakeTrace.vecEndPos = pos;
	fakeTrace.flFraction = 0.5f;

	// Meat sounds
	if (weaponId == WEAPON_HORNETGUN)
	{
		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			EMIT_SOUND_DYN(m_pActiveItem->edict(), CHAN_ITEM, "weapons/bullet_hit1.wav", 1, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
			break;
		case 1:
			EMIT_SOUND_DYN(m_pActiveItem->edict(), CHAN_ITEM, "weapons/bullet_hit2.wav", 1, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
			break;
		}
	}
	// Squeek and soft meat sounds
	else if (weaponId == WEAPON_SNARK)
	{
		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			EMIT_SOUND_DYN(m_pActiveItem->edict(), CHAN_ITEM, "weapons/bullet_hit1.wav", 1, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
			break;
		case 1:
			EMIT_SOUND_DYN(m_pActiveItem->edict(), CHAN_ITEM, "weapons/bullet_hit2.wav", 1, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
			break;
		}
		EMIT_SOUND_DYN(m_pActiveItem->edict(), CHAN_VOICE, (RANDOM_FLOAT(0, 1) <= 0.5) ? "squeek/sqk_hunt2.wav" : "squeek/sqk_hunt3.wav", 1, ATTN_NORM, 0, 105);
	}
	// TODO: Hand slap sounds
	else if (weaponId == WEAPON_BAREHAND)
	{
		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/bullet_hit1.wav", 1, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
			break;
		case 1:
			EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/bullet_hit2.wav", 1, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
			break;
		}
	}
	// Any other weapon makes crowbar sounds
	else if (pSmackedEntity->Classify() == CLASS_NONE || pSmackedEntity->Classify() == CLASS_MACHINE)
	{
		this->m_iWeaponVolume = CROWBAR_WALLHIT_VOLUME;

		TEXTURETYPE_PlaySound(&fakeTrace, pos - velocity.Normalize() * 32.f, pos + velocity.Normalize() * 32.f, BULLET_PLAYER_CROWBAR);

		// also play crowbar strike
		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			EMIT_SOUND_DYN(pSmackedEntity->edict(), CHAN_ITEM, "weapons/cbar_hit1.wav", 1, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
			break;
		case 1:
			EMIT_SOUND_DYN(pSmackedEntity->edict(), CHAN_ITEM, "weapons/cbar_hit2.wav", 1, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
			break;
		}

		DecalGunshot(&fakeTrace, BULLET_PLAYER_CROWBAR);
	}
	else
	{
		this->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;

		// play thwack or smack sound
		switch (RANDOM_LONG(0, 2))
		{
		case 0:
			EMIT_SOUND_DYN(pSmackedEntity->edict(), CHAN_ITEM, "weapons/cbar_hitbod1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			break;
		case 1:
			EMIT_SOUND_DYN(pSmackedEntity->edict(), CHAN_ITEM, "weapons/cbar_hitbod2.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			break;
		case 2:
			EMIT_SOUND_DYN(pSmackedEntity->edict(), CHAN_ITEM, "weapons/cbar_hitbod3.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
	}
}

void CBasePlayer::PlayVRWeaponAnimation(int iAnim, int body)
{
	//pev->weaponanim = iAnim;
	m_vrControllers[VRControllerID::WEAPON].PlayWeaponAnimation(iAnim, body);
}

void CBasePlayer::PlayVRWeaponMuzzleflash()
{
	m_vrControllers[VRControllerID::WEAPON].PlayWeaponMuzzleflash();
}

void CBasePlayer::UpdateFlashlight()
{
	// always call GetFlashlightPose, as it sets/unsets hand model flashlight body on the appropriate controller(s)
	Vector position;
	Vector dir;
	GetFlashlightPose(position, dir);

	if (FlashlightIsOn())
	{
		if (hFlashlightMonster)
		{
			ClearBits(hFlashlightMonster->pev->effects, EF_DIMLIGHT);
			hFlashlightMonster = nullptr;
		}
		if (hFlashLight)
		{
			TraceResult tr;
			UTIL_TraceLine(position, dir * 8192., dont_ignore_monsters, edict(), &tr);
			CBaseMonster* pFlashlightMonster = CBaseEntity::SafeInstance<CBaseMonster>(tr.pHit);
			if (pFlashlightMonster != nullptr)
			{
				SetBits(pFlashlightMonster->pev->effects, EF_DIMLIGHT);
				hFlashlightMonster = pFlashlightMonster;
				hFlashLight->pev->effects = EF_NODRAW;
			}
			else
			{
				UTIL_SetOrigin(hFlashLight->pev, tr.vecEndPos - dir);
				hFlashLight->pev->effects = EF_DIMLIGHT;
			}
		}
	}
}

void CBasePlayer::StartVRTele()
{
	Vector telePos, dummy;
	GetTeleporterPose(telePos, dummy);
	m_vrControllerTeleporter.StartTele(this, telePos);
}

void CBasePlayer::StopVRTele()
{
	m_vrControllerTeleporter.StopTele(this);
}

void CBasePlayer::UpdateVRTele()
{
	Vector telePos, teleDir;
	GetTeleporterPose(telePos, teleDir);
	m_vrControllerTeleporter.UpdateTele(this, telePos, teleDir);
}

void CBasePlayer::SetFlashlightPose(const Vector& offset, const Vector& angles)
{
	m_vrFlashlightOffset = offset;
	m_vrFlashlightAngles = angles;
	m_vrHasFlashlightPose = true;
}

void CBasePlayer::ClearFlashlightPose()
{
	m_vrHasFlashlightPose = false;
}

constexpr const int VR_FLASHLIGHT_ATTACHMENT_HAND = 0;
constexpr const int VR_FLASHLIGHT_ATTACHMENT_WEAPON = 1;
constexpr const int VR_FLASHLIGHT_ATTACHMENT_HEAD = 2;
constexpr const int VR_FLASHLIGHT_ATTACHMENT_POSE = 3;

// TODO: Merge with GetTeleporterPose, super redundant code
void CBasePlayer::GetFlashlightPose(Vector& position, Vector& dir)
{
	int attachment = int(CVAR_GET_FLOAT("vr_flashlight_attachment"));

	for (auto& controller : m_vrControllers)
	{
		controller.second.SetHasFlashlight(false);
	}

	if (attachment == VR_FLASHLIGHT_ATTACHMENT_POSE)
	{
		if (m_vrHasFlashlightPose)
		{
			position = GetClientOrigin() + m_vrFlashlightOffset;
			UTIL_MakeAimVectorsPrivate(m_vrFlashlightAngles, dir, nullptr, nullptr);
			return;
		}
		else
		{
			// fallback to hand
			attachment = VR_FLASHLIGHT_ATTACHMENT_HAND;
		}
	}

	if (attachment == VR_FLASHLIGHT_ATTACHMENT_HAND)
	{
		if (m_vrControllers[VRControllerID::HAND].IsValid())
		{
			Vector pos1;
			Vector pos2;
			bool result1 = m_vrControllers[VRControllerID::HAND].GetAttachment(VR_MUZZLE_ATTACHMENT, pos1);
			bool result2 = m_vrControllers[VRControllerID::HAND].GetAttachment(VR_MUZZLE_ATTACHMENT + 1, pos2);
			if (result1 && result2 && pos2 != pos1)
			{
				position = pos1;
				dir = (pos2 - pos1).Normalize();
			}
			else
			{
				position = m_vrControllers[VRControllerID::HAND].GetPosition();
				UTIL_MakeAimVectorsPrivate(m_vrControllers[VRControllerID::HAND].GetAngles(), dir, nullptr, nullptr);
			}
			m_vrControllers[VRControllerID::HAND].SetHasFlashlight(true);
			return;
		}
		else
		{
			// fallback to weapon
			attachment = VR_FLASHLIGHT_ATTACHMENT_WEAPON;
		}
	}

	if (attachment == VR_FLASHLIGHT_ATTACHMENT_WEAPON)
	{
		if (m_vrControllers[VRControllerID::WEAPON].IsValid())
		{
			Vector pos1;
			Vector pos2;
			bool result1 = m_vrControllers[VRControllerID::WEAPON].GetAttachment(VR_MUZZLE_ATTACHMENT, pos1);
			bool result2 = m_vrControllers[VRControllerID::WEAPON].GetAttachment(VR_MUZZLE_ATTACHMENT + 1, pos2);
			if (result1 && result2 && pos2 != pos1)
			{
				position = pos1;
				dir = (pos2 - pos1).Normalize();
			}
			else
			{
				position = m_vrControllers[VRControllerID::WEAPON].GetPosition();
				UTIL_MakeAimVectorsPrivate(m_vrControllers[VRControllerID::WEAPON].GetAngles(), dir, nullptr, nullptr);
			}
			m_vrControllers[VRControllerID::WEAPON].SetHasFlashlight(true);
			return;
		}
		else
		{
			// fallback to pose
			attachment = VR_FLASHLIGHT_ATTACHMENT_WEAPON;
		}
	}

	// either attachment is pose or we fallback to pose because no other attachment was available
	position = EyePosition();
	UTIL_MakeVectorsPrivate(pev->v_angle + pev->punchangle, dir, nullptr, nullptr);
}

void CBasePlayer::SetTeleporterPose(const Vector& offset, const Vector& angles)
{
	m_vrTeleporterOffset = offset;
	m_vrTeleporterAngles = angles;
	m_vrHasTeleporterPose = true;
}

void CBasePlayer::ClearTeleporterPose()
{
	m_vrHasTeleporterPose = false;
}

// TODO: Merge with GetFlashlightPose, super redundant code
void CBasePlayer::GetTeleporterPose(Vector& position, Vector& dir)
{
	int attachment = int(CVAR_GET_FLOAT("vr_teleport_attachment"));

	if (attachment == VR_FLASHLIGHT_ATTACHMENT_POSE)
	{
		if (m_vrHasTeleporterPose)
		{
			position = GetClientOrigin() + m_vrTeleporterOffset;
			UTIL_MakeAimVectorsPrivate(m_vrTeleporterAngles, dir, nullptr, nullptr);
			return;
		}
		else
		{
			// fallback to hand
			attachment = VR_FLASHLIGHT_ATTACHMENT_HAND;
		}
	}

	if (attachment == VR_FLASHLIGHT_ATTACHMENT_HAND)
	{
		if (m_vrControllers[VRControllerID::HAND].IsValid())
		{
			Vector pos1;
			Vector pos2;
			bool result1 = m_vrControllers[VRControllerID::HAND].GetAttachment(VR_MUZZLE_ATTACHMENT, pos1);
			bool result2 = m_vrControllers[VRControllerID::HAND].GetAttachment(VR_MUZZLE_ATTACHMENT + 1, pos2);
			if (result1 && result2 && pos2 != pos1)
			{
				position = pos1;
				dir = (pos2 - pos1).Normalize();
			}
			else
			{
				position = m_vrControllers[VRControllerID::HAND].GetPosition();
				UTIL_MakeAimVectorsPrivate(m_vrControllers[VRControllerID::HAND].GetAngles(), dir, nullptr, nullptr);
			}
			return;
		}
		else
		{
			// fallback to weapon
			attachment = VR_FLASHLIGHT_ATTACHMENT_WEAPON;
		}
	}

	if (attachment == VR_FLASHLIGHT_ATTACHMENT_WEAPON)
	{
		if (m_vrControllers[VRControllerID::WEAPON].IsValid())
		{
			Vector pos1;
			Vector pos2;
			bool result1 = m_vrControllers[VRControllerID::WEAPON].GetAttachment(VR_MUZZLE_ATTACHMENT, pos1);
			bool result2 = m_vrControllers[VRControllerID::WEAPON].GetAttachment(VR_MUZZLE_ATTACHMENT + 1, pos2);
			if (result1 && result2 && pos2 != pos1)
			{
				position = pos1;
				dir = (pos2 - pos1).Normalize();
			}
			else
			{
				position = m_vrControllers[VRControllerID::WEAPON].GetPosition();
				UTIL_MakeAimVectorsPrivate(m_vrControllers[VRControllerID::WEAPON].GetAngles(), dir, nullptr, nullptr);
			}
			return;
		}
		else
		{
			// fallback to pose
			attachment = VR_FLASHLIGHT_ATTACHMENT_WEAPON;
		}
	}

	// either attachment is pose or we fallback to pose because no other attachment was available
	position = EyePosition();
	UTIL_MakeVectorsPrivate(pev->v_angle + pev->punchangle, dir, nullptr, nullptr);
}

// In VR it's rather difficult to longjump (run + crouch + jump + correct timing),
// so players can map the long jump on a simple VR input action.
// - Max Makes Mods - 2019-04-13
#define BUNNYJUMP_MAX_SPEED_FACTOR 1.7f
#define PLAYER_LONGJUMP_SPEED      350
void CBasePlayer::DoLongJump(bool playStepSound, bool force)
{
	if (!force)
	{
		if (0 != pev->waterlevel)
			return;

		if ((pev->button & IN_JUMP) != 0)
			return;

		if ((pev->oldbuttons & IN_JUMP) != 0)
			return;

		if (0 != pmove->waterjumptime)
			return;

		if ((pev->flags & FL_ONGROUND) == 0)
			return;
	}

	float maxscaledspeed = BUNNYJUMP_MAX_SPEED_FACTOR * pmove->maxspeed;
	if (maxscaledspeed <= 0.f)
		return;

	if (playStepSound)
	{
		extern void PM_PlayStepSound(int step, float fvol);
		extern int PM_MapTextureTypeStepType(char chTextureType);
		PM_PlayStepSound(PM_MapTextureTypeStepType(pmove->chtexturetype), 1.0f);
	}

	if (m_fLongJump)
	{
		pev->punchangle.x = -5.f;

		Vector forward = pmove->forward;
		if (pmove->forward.Length2D() > EPSILON)
		{
			forward = pmove->forward;
		}
		else if (pmove->velocity.Length2D() > EPSILON)
		{
			forward = pmove->velocity;
		}
		else if (pev->velocity.Length2D() > EPSILON)
		{
			forward = pev->velocity;
		}
		else
		{
			forward = vr_hmdForward; // TODO: This should fall back to the device used for movement, not simply the HMD
		}
		forward.z = 0.f;
		forward = forward.Normalize();

		pev->velocity.x = forward.x * PLAYER_LONGJUMP_SPEED * 1.6f;
		pev->velocity.y = forward.y * PLAYER_LONGJUMP_SPEED * 1.6f;
		pev->velocity.z = sqrt(2.f * 800.f * 56.f);

		SetAnimation(PLAYER_SUPERJUMP);
	}
	else
	{
		pev->velocity.z = sqrt(2.f * 800.f * 56.f);
		SetAnimation(PLAYER_JUMP);
	}

	//pev->velocity.z -= (pev->gravity * g_psv_gravity->value * gpGlobals->frametime * 0.5f);

	if (pev->velocity.x > pmove->movevars->maxvelocity)
		pev->velocity.x = pmove->movevars->maxvelocity;
	if (pev->velocity.y > pmove->movevars->maxvelocity)
		pev->velocity.y = pmove->movevars->maxvelocity;
	if (pev->velocity.z > pmove->movevars->maxvelocity)
		pev->velocity.z = pmove->movevars->maxvelocity;
	if (pev->velocity.x < -pmove->movevars->maxvelocity)
		pev->velocity.x = -pmove->movevars->maxvelocity;
	if (pev->velocity.y < -pmove->movevars->maxvelocity)
		pev->velocity.y = -pmove->movevars->maxvelocity;
	if (pev->velocity.z < -pmove->movevars->maxvelocity)
		pev->velocity.z = -pmove->movevars->maxvelocity;
}

void CBasePlayer::HolsterWeapon(bool force)
{
	// force last item to be nullptr if we are already holstered
	// (SelectItem doesn't do that as it's a no-op if the item doesn't change)
	if (force && m_pActiveItem == nullptr)
	{
		m_pLastItem = nullptr;
	}
	SelectItem("weapon_barehand");
}

void CBasePlayer::SetLadderGrabbingController(VRControllerID controller, CBaseEntity* pLadder)
{
	m_ladderGrabbingControllers.push_back(VRLadderGrabbingController{controller, pLadder});

	MESSAGE_BEGIN(MSG_ONE, gmsgVRGrabbedLadder, nullptr, pev);
	WRITE_SHORT(ENTINDEX(pLadder->edict()));
	MESSAGE_END();
}

void CBasePlayer::ClearLadderGrabbingControllers()
{
	if (m_ladderGrabbingControllers.empty())
		return;

	m_ladderGrabbingControllers.clear();
	MESSAGE_BEGIN(MSG_ONE, gmsgVRGrabbedLadder, nullptr, pev);
	WRITE_SHORT(0);
	MESSAGE_END();
}

void CBasePlayer::ClearLadderGrabbingController(VRControllerID controller)
{
	if (m_ladderGrabbingControllers.empty())
		return;

	m_ladderGrabbingControllers.erase(
		std::remove_if(
			m_ladderGrabbingControllers.begin(),
			m_ladderGrabbingControllers.end(),
			[controller](const VRLadderGrabbingController& v)
			{
				return v.controller == controller;
			}),
		m_ladderGrabbingControllers.end());

	if (m_ladderGrabbingControllers.empty())
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgVRGrabbedLadder, nullptr, pev);
		WRITE_SHORT(0);
		MESSAGE_END();
	}
}

bool CBasePlayer::IsLadderGrabbingController(VRControllerID controller, CBaseEntity* pLadder)
{
	if (m_ladderGrabbingControllers.empty())
		return false;

	return m_ladderGrabbingControllers.back().controller == controller && m_ladderGrabbingControllers.back().ladder == pLadder;
}

int CBasePlayer::GetGrabbedLadderEntIndex()
{
	if (m_ladderGrabbingControllers.empty() || (VRGetLadderMode() == VR_LADDER_MODE_LEGACY_ONLY))
		return 0;

	return ENTINDEX(m_ladderGrabbingControllers.back().ladder.Get());
}


void CBasePlayer::StartPullingLedge(const Vector& ledgeTargetPosition, float speed)
{
	m_vrLedgeTargetPosition = ledgeTargetPosition;
	m_vrLedgePullSpeed = speed;
	m_vrLedgePullStartTime = gpGlobals->time;
	m_vrLedgePullStartPosition = pev->origin;
	m_vrIsPullingOnLedge = true;

	MESSAGE_BEGIN(MSG_ONE, gmsgVRPullingLedge, nullptr, pev);
	WRITE_BYTE(1);
	MESSAGE_END();
}

void CBasePlayer::StopPullingLedge()
{
	if (!m_vrIsPullingOnLedge)
		return;

	m_vrIsPullingOnLedge = false;

	MESSAGE_BEGIN(MSG_ONE, gmsgVRPullingLedge, nullptr, pev);
	WRITE_BYTE(0);
	MESSAGE_END();
}

constexpr const int SF_TANK_CANCONTROL = 0x0020;

bool IsValidTankDraggingController(const VRController& controller, entvars_t* pevTank)
{
	return controller.IsValid() &&
		   controller.IsDragging() &&
		   !controller.HasDraggedEntity() &&
		   ((controller.GetPosition() - pevTank->origin).Length() < CVAR_GET_FLOAT("vr_tankcontrols_max_distance"));
}

bool CBasePlayer::IsTankVRControlled(entvars_t* pevTank)
{
	if (CVAR_GET_FLOAT("vr_tankcontrols") == 0.f)
		return false;

	if (!pevTank)
		return false;

	if (FBitSet(pevTank->spawnflags, SF_TANK_CANCONTROL) && UTIL_StartsWith(STRING(pevTank->classname), "func_tank"))
	{
		// vr_tankcontrols is 0 for off (checked above)
		// or 1 for one handed
		// or 2 for two handed tank controls
		if (CVAR_GET_FLOAT("vr_tankcontrols") == 1.f)
		{
			return IsValidTankDraggingController(m_vrControllers[VRControllerID::HAND], pevTank) ||
				   IsValidTankDraggingController(m_vrControllers[VRControllerID::WEAPON], pevTank);
		}
		else
		{
			return IsValidTankDraggingController(m_vrControllers[VRControllerID::HAND], pevTank) &&
				   IsValidTankDraggingController(m_vrControllers[VRControllerID::WEAPON], pevTank);
		}
	}

	return false;
}

CBaseEntity* CBasePlayer::VRFindTank(const char* func_tank_classname)
{
	edict_t* pent = FIND_ENTITY_BY_CLASSNAME(nullptr, func_tank_classname);
	while (!FNullEnt(pent))
	{
		if (IsTankVRControlled(&pent->v) &&
			(UTIL_IsFacing(pev->origin, pev->angles.ToViewAngles(), pent->v.origin) || UTIL_IsFacing(pev->origin, pev->angles.ToViewAngles(), (pent->v.absmax + pent->v.absmin) * 0.5)))
		{
			return CBaseEntity::SafeInstance<CBaseEntity>(pent);
		}
		pent = FIND_ENTITY_BY_CLASSNAME(pent, func_tank_classname);
	}
	return nullptr;
}

void CBasePlayer::VRUseOrUnuseTank()
{
	if (m_vrIsUsingTankWithVRControllers)
	{
		bool cancelUse = false;
		if (m_pTank && IsTankVRControlled(m_pTank->pev))
		{
			std::vector<Vector> allControllerPositions;
			for (auto& [id, controller] : m_vrControllers)
			{
				if (IsValidTankDraggingController(controller, m_pTank->pev))
				{
					allControllerPositions.push_back(controller.GetPosition());
				}
			}

			if (allControllerPositions.empty())
			{
				cancelUse = true;
			}
			else
			{
				Vector position;
				for (auto& controllerPosition : allControllerPositions)
				{
					position = position + controllerPosition;
				}
				position = position / float(allControllerPositions.size());

				Vector direction = (m_pTank->pev->origin - position).Normalize();
				m_vrTankVRControllerAngles = UTIL_VecToAngles(direction);

				// make sure no weapon is selected (noop if no weapon is selected)
				HolsterWeapon(true);

				// fire the gun
				m_pTank->Use(this, this, USE_SET, 2);
			}
		}
		else
		{
			cancelUse = true;
		}

		if (cancelUse)
		{
			if (m_pTank)
				m_pTank->Use(this, this, USE_OFF, 0);
			m_pTank = nullptr;
			if (m_pActiveItem == nullptr)
			{
				SelectLastItem();
			}
			m_vrIsUsingTankWithVRControllers = false;
			m_vrTankVRControllerAngles = Vector{};
		}
	}

	if (!m_vrIsUsingTankWithVRControllers)
	{
		CBaseEntity* pTank = VRFindTank("func_tank");
		if (!pTank)
			pTank = VRFindTank("func_tanklaser");
		if (!pTank)
			pTank = VRFindTank("func_tankrocket");
		if (!pTank)
			pTank = VRFindTank("func_tankmortar");

		if (pTank)
		{
			if (m_pTank && m_pTank != pTank)
			{
				m_pTank->Use(this, this, USE_OFF, 0);
			}
			m_pTank = pTank;
			m_pTank->Use(this, this, USE_ON, 1337); // 1337 is hacky way to tell CFuncTank that this is a VR initiated control
			m_vrIsUsingTankWithVRControllers = true;
			m_vrTankVRControllerAngles = m_pTank->pev->angles;
			HolsterWeapon(true);
		}
	}
}

Vector CBasePlayer::GetTankControlAngles()
{
	if (m_vrIsUsingTankWithVRControllers)
	{
		return m_vrTankVRControllerAngles;
	}
	else
	{
		Vector angles = pev->v_angle;
		angles.x = -angles.x;
		return angles;
	}
}

bool CBasePlayer::VRCanAttack()
{
	TraceResult tr{0};
	UTIL_TraceLine(pev->origin, GetGunPosition(), ignore_monsters, nullptr, &tr);
	return tr.flFraction == 1.f;
}

void CBasePlayer::VRJustTeleported(const Vector& fromOrigin, const Vector& fromAngles)
{
	// Let go off ladders if teleporting
	for (auto& bla : m_ladderGrabbingControllers)
	{
		m_vrControllers[bla.controller].ClearDraggedEntity();
	}
	ClearLadderGrabbingControllers();

	// Let go off ledges if teleporting
	StopPullingLedge();

	// Make sure grabbed objects come with us
	for (auto& [controllerID, controller] : m_vrControllers)
	{
		controller.VRJustTeleported(this, fromOrigin, fromAngles);
	}
}

void EXPORT CBaseEntity::DragThink(void)
{
	// can happen afer a levelchange, should get fixed in the next VRControllerInteractionManager::HandleGrabbables update
	if (!m_vrDragger)
	{
		return;
	}

	if (m_isFirstDragThink)
	{
		m_isFirstDragThink = false;
		HandleDragStart();
	}

	m_pfnThink = nullptr;

	EHandleT<CBasePlayer> hPlayer = m_vrDragger;
	if (hPlayer && m_vrDragController != VRControllerID::INVALID)
	{
		auto& controller = hPlayer->GetController(m_vrDragController);
		if (controller.IsValid())
		{
			Vector origin = controller.GetGunPosition();
			Vector velocity = controller.GetVelocity();
			Vector angles = controller.GetAngles();

			HandleDragUpdate(origin, velocity, UTIL_AnglesMod(angles));
		}
	}
}

void CBasePlayer::VRHandleRetinaScanners()
{
	for (auto [hRetinaScanner, hRetinaScannerButton] : g_vrRetinaScanners)
	{
		Vector retinaScannerPosition = (hRetinaScanner->pev->absmax + hRetinaScanner->pev->absmin) * 0.5;
		bool isLookingAtRetinaScanner =
			UTIL_IsFacing(pev->origin, pev->angles.ToViewAngles(), retinaScannerPosition) && ((EyePosition() - retinaScannerPosition).Length() < 32.f) && (EyePosition().z >= hRetinaScanner->pev->absmin.z) && (EyePosition().z <= hRetinaScanner->pev->absmax.z);

		if (isLookingAtRetinaScanner)
		{
			if (m_vrHRetinaScanner == hRetinaScanner)
			{
				if ((gpGlobals->time - m_vrRetinaScannerLookTime) >= VR_RETINASCANNER_ACTIVATE_LOOK_TIME)
				{
					if (!m_vrRetinaScannerUsed)
					{
						g_vrRetinaScanners[hRetinaScanner]->Use(this, this, USE_SET, 1);
						m_vrRetinaScannerUsed = true;
					}
				}
			}
			else
			{
				m_vrHRetinaScanner = hRetinaScanner;
				m_vrRetinaScannerLookTime = gpGlobals->time;
				m_vrRetinaScannerUsed = false;
			}
		}
		else
		{
			if (m_vrHRetinaScanner == hRetinaScanner)
			{
				m_vrHRetinaScanner = nullptr;
				m_vrRetinaScannerLookTime = 0.f;
				m_vrRetinaScannerUsed = false;
			}
		}
	}
}

void CBasePlayer::UpdateVRLaserSpot()
{
	int weaponId = WEAPON_BAREHAND;
	ItemInfo itemInfo = {};
	if (m_pActiveItem != nullptr && m_pActiveItem->GetItemInfo(&itemInfo))
	{
		weaponId = itemInfo.iId;
	}

	// No laser dot while a camera controls the view (trigger_camera): the hands and weapon aren't drawn then
	if (IsAlive() && IsWeaponWithVRLaserSpot(weaponId) && CVAR_GET_FLOAT("vr_enable_aim_laser") != 0.f && !m_hViewEntity)
	{
		if (!m_hLaserSpot)
		{
			CLaserSpot* pLaserSpot = CLaserSpot::CreateSpot();
			pLaserSpot->Revive();
			pLaserSpot->pev->scale = 0.1f;
			m_hLaserSpot = pLaserSpot;
		}

		TraceResult tr;
		UTIL_TraceLine(GetWeaponPosition(), GetWeaponPosition() + (GetAutoaimVector() * 8192.f), dont_ignore_monsters, edict(), &tr);
		UTIL_SetOrigin(m_hLaserSpot->pev, tr.vecEndPos);
	}
	else
	{
		if (m_hLaserSpot)
		{
			UTIL_Remove(m_hLaserSpot);
			m_hLaserSpot = nullptr;
		}
	}
}

bool CBasePlayer::IsAnyControllerFiringAndHoldingThisTank(CBaseEntity* pTank)
{
	if (pTank != m_pTank)
		return false;

	for (auto& [id, controller] : m_vrControllers)
	{
		if (controller.IsFiring() && IsValidTankDraggingController(controller, pTank->pev))
		{
			return true;
		}
	}

	return false;
}

float GetTrainSpeed(const char* cvarname)
{
	float speed = CVAR_GET_FLOAT(cvarname);

	if (speed <= 0.f)
		return 0.25f;

	if (speed > 1.f)
		return 1.f;

	return speed;
}

bool IsTrashCompactor(CBaseEntity* pEntity)
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

bool CBasePlayer::IsUsableTrackTrain(CBaseEntity* pTrain)
{
	return nullptr != pTrain && !FBitSet(pTrain->pev->spawnflags, SF_TRACKTRAIN_NOCONTROL) && FBitSet(pTrain->ObjectCaps(), FCAP_DIRECTIONAL_USE) && pTrain->OnControls(pev) && !IsTrashCompactor(pTrain);
}

void CBasePlayer::StoreVROffsetsForLevelchange()
{
	g_vrLevelChangeData.lastHMDOffset = this->vr_lastHMDOffset;
	g_vrLevelChangeData.clientOriginOffset = this->vr_ClientOriginOffset;
	g_vrLevelChangeData.prevYaw = this->vr_prevYaw;
	g_vrLevelChangeData.currentYaw = this->vr_currentYaw;
	g_vrLevelChangeData.hasData = true;
}

// Savedata descriptor for vr level change data
TYPEDESCRIPTION g_vrLevelChangeDataSaveData[] =
	{
		DEFINE_FIELD(VRLevelChangeData, lastHMDOffset, FIELD_VECTOR),
		DEFINE_FIELD(VRLevelChangeData, clientOriginOffset, FIELD_VECTOR),
		DEFINE_FIELD(VRLevelChangeData, hasData, FIELD_BOOLEAN),
		DEFINE_FIELD(VRLevelChangeData, prevYaw, FIELD_FLOAT),
		DEFINE_FIELD(VRLevelChangeData, currentYaw, FIELD_FLOAT),
};

// Savedata descriptor for vr level change data
// TYPEDESCRIPTION g_vrAchievementsAndStatsData[] =
// {
// 	DEFINE_FIELD(CVRAchievementsAndStatsData, m_bitFlags, FIELD_INTEGER),
// 	DEFINE_FIELD(CVRAchievementsAndStatsData, m_bpBridgesDestroyed, FIELD_CHARACTER),
// 	DEFINE_FIELD(CVRAchievementsAndStatsData, m_exp1HeadcrabsKilled, FIELD_CHARACTER),
// 	DEFINE_FIELD(CVRAchievementsAndStatsData, m_exp2HeadcrabsKilled, FIELD_CHARACTER),
// 	DEFINE_FIELD(CVRAchievementsAndStatsData, m_totalNegativeCrushDamage, FIELD_FLOAT),
// 	DEFINE_FIELD(CVRAchievementsAndStatsData, m_residueBarneyStartedRunningTime, FIELD_FLOAT),
// 	DEFINE_ARRAY(CVRAchievementsAndStatsData, m_prevMap, FIELD_CHARACTER, 32)
// };

// Hooks called from the SDK player code in player.cpp

// Start of PreThink: achievement checks (disabled) and the special VR entity scan after a map load
void CBasePlayer::VRPreThinkStart()
{
	// VRAchievementsAndStatsTracker::Update(this);

	if (!m_vrHasSurfacedInThatMapWithTheTank)
	{
		// if (pev->origin.z > 0.f && FStrEq(STRING(INDEXENT(0)->v.model), "maps/c2a5b.bsp"))
		// {
		// 	m_vrHasSurfacedInThatMapWithTheTank = true;
		// 	VRAchievementsAndStatsTracker::PlayerSurfacedInThatMapWithTheTank(this);
		// }
	}

	if (m_vrJustGotYeetedByBPTrain && FBitSet(pev->flags, FL_ONGROUND))
	{
		// we landed after getting yeeted by that train at the beginning of Blast Pit
		m_vrJustGotYeetedByBPTrain = false;

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
	}

	extern bool VRGlobalGetNoclipMode();
	if (VRGlobalGetNoclipMode())
	{
		// VRAchievementsAndStatsTracker::PlayerUsedNoclip(this);
	}

	// gets reset by ClientPrecache everytime new map is loaded (new game, changelevel, load save)
	if (g_vrNeedRecheckForSpecialEntities)
	{
		g_vrNeedRecheckForSpecialEntities = false;
		for (int index = 1; index < gpGlobals->maxEntities; index++)
		{
			edict_t* pent = INDEXENT(index);
			if (FNullEnt(pent))
				continue;

			EHandleT<CBaseEntity> hEntity = CBaseEntity::SafeInstance<CBaseEntity>(pent);
			if (hEntity)
			{
				if (!hEntity->CheckIsSpecialVREntity())
				{
					g_vrNeedRecheckForSpecialEntities = true;
				}
			}
		}
	}
}

// PreThink: controller interactions with the world, retina scanners and VR train controls
void CBasePlayer::VRPreThinkControllersAndTrain()
{
	// VR stuff: Calculate controller interactions with world
	m_vrControllerInteractionManager.CheckAndPressButtons(this, m_vrControllers[VRControllerID::HAND], m_vrControllers[VRControllerID::WEAPON]);
	m_vrControllerInteractionManager.CheckAndPressButtons(this, m_vrControllers[VRControllerID::WEAPON], m_vrControllers[VRControllerID::HAND]);

	// Special interaction with 2 controllers at once (e.g. pull up on ledges)
	if (m_vrControllers[VRControllerID::HAND].IsValid() && m_vrControllers[VRControllerID::WEAPON].IsValid())
	{
		m_vrControllerInteractionManager.DoMultiControllerActions(this, m_vrControllers[VRControllerID::HAND], m_vrControllers[VRControllerID::WEAPON]);
	}

	// Handle retina scanners
	VRHandleRetinaScanners();

	// Check if we are on a train
	if (CVAR_GET_FLOAT("vr_train_controls") == 0.f)
	{
		if (pev->groundentity && !FBitSet(m_afPhysicsFlags, PFLAG_ONTRAIN))
		{
			CBaseEntity* pMaybeTrain = CBaseEntity::InstanceOrWorld(pev->groundentity);
			if (IsUsableTrackTrain(pMaybeTrain))
			{
				// Start controlling the train!
				m_afPhysicsFlags |= PFLAG_ONTRAIN;
				m_iTrain = TrainSpeed(pMaybeTrain->pev->speed, pMaybeTrain->pev->impulse);
				m_iTrain |= TRAIN_NEW;
				EMIT_SOUND(ENT(pev), CHAN_ITEM, "plats/train_use1.wav", 0.8, ATTN_NORM);
				// VRAchievementsAndStatsTracker::PlayerUsedTrain(this);
			}
		}
	}

	// Set FL_ONTRAIN only if legacy train controls are enabled (this flag prevents movement in pm_shared)
	if (CVAR_GET_FLOAT("vr_train_controls") != 0.f && (m_afPhysicsFlags & PFLAG_ONTRAIN) != 0)
		pev->flags |= FL_ONTRAIN;
	else
		pev->flags &= ~FL_ONTRAIN;

	// Check if we are still on a train
	if ((m_afPhysicsFlags & PFLAG_ONTRAIN) != 0)
	{
		CBaseEntity* pTrain = CBaseEntity::InstanceOrWorld(pev->groundentity);

		if (!pTrain)
		{
			// Maybe this is on the other side of a level transition
			TraceResult trainTrace;
			UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, -38), ignore_monsters, ENT(pev), &trainTrace);

			// HACKHACK - Just look for the func_tracktrain classname
			if (trainTrace.flFraction != 1.0 && trainTrace.pHit)
			{
				pTrain = CBaseEntity::InstanceOrWorld(trainTrace.pHit);
			}
		}

		if (!IsUsableTrackTrain(pTrain))
		{
			// Turn off the train if the train controls go dead or you leave the control area
			m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
			m_iTrain = TRAIN_NEW | TRAIN_OFF;
		}
		else
		{
			// Train speed control
			Vector forward;
			Vector right;
			Vector up;
			UTIL_MakeVectorsPrivate(pTrain->pev->angles, forward, right, up);
			forward.z = 0.f;
			right.z = 0.f;
			forward = forward.Normalize();
			right = right.Normalize();

			Vector2D controlsOffset;
			auto pTrackTrain = dynamic_cast<CFuncTrackTrain*>(pTrain);
			if (pTrackTrain)
			{
				controlsOffset = pTrackTrain->GetVRControlsOffset();
			}
			else
			{
				controlsOffset = Vector2D{-48.f, 20.f};
			}
			vr_trainControlPosition = pTrain->pev->origin + (forward * controlsOffset.x) + (right * controlsOffset.y);
			vr_trainControlPosition.z = pev->origin.z + 8.f;
			vr_trainControlYaw = pTrain->pev->angles.y;

			if (m_iTrain == TRAIN_OFF)
			{
				m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
			}

			if (CVAR_GET_FLOAT("vr_train_controls") != 0.f)
			{
				pev->velocity = g_vecZero;
				float vel = 0;
				if ((m_afButtonPressed & IN_FORWARD) != 0)
				{
					vel = 1;
				}
				else if ((m_afButtonPressed & IN_BACK) != 0)
				{
					vel = -1;
				}
				if (0 != vel)
				{
					pTrain->Use(this, this, USE_SET, vel);
					m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
				}
			}
			else
			{
				Vector trainFastButtonPos = vr_trainControlPosition + (forward * -8.f) + (up * 20.f);
				Vector trainMedmButtonPos = vr_trainControlPosition + (forward * -6.25f) + (up * 15.5f);
				Vector trainSlowButtonPos = vr_trainControlPosition + (forward * -4.5f) + (up * 11.5f);
				Vector trainNeutButtonPos = vr_trainControlPosition + (forward * -2.75f) + (up * 6.5f);
				Vector trainBackButtonPos = vr_trainControlPosition + (forward * -1.f) + (up * 2.f);

				float newspeed = pTrain->pev->speed;
				if (CheckVRTRainButtonTouched(trainNeutButtonPos + (right * 8.f), trainNeutButtonPos - (right * 8.f)))
				{
					m_iTrain = TRAIN_NEUTRAL;
					newspeed = 0.f;
				}
				else if (CheckVRTRainButtonTouched(trainSlowButtonPos + (right * 8.f), trainSlowButtonPos - (right * 8.f)))
				{
					m_iTrain = TRAIN_SLOW;
					newspeed = pTrain->pev->impulse * GetTrainSpeed("vr_train_speed_slow");
				}
				else if (CheckVRTRainButtonTouched(trainMedmButtonPos + (right * 8.f), trainMedmButtonPos - (right * 8.f)))
				{
					m_iTrain = TRAIN_MEDIUM;
					newspeed = pTrain->pev->impulse * GetTrainSpeed("vr_train_speed_medium");
				}
				else if (CheckVRTRainButtonTouched(trainFastButtonPos + (right * 8.f), trainFastButtonPos - (right * 8.f)))
				{
					m_iTrain = TRAIN_FAST;
					newspeed = pTrain->pev->impulse * GetTrainSpeed("vr_train_speed_fast");
				}
				else if (CheckVRTRainButtonTouched(trainBackButtonPos + (right * 8.f), trainBackButtonPos - (right * 8.f)))
				{
					m_iTrain = TRAIN_BACK;
					newspeed = -pTrain->pev->impulse * GetTrainSpeed("vr_train_speed_back");
				}

				if (newspeed != pTrain->pev->speed)
				{
					EMIT_SOUND(ENT(pev), CHAN_ITEM, "plats/train_use2.wav", 0.6, ATTN_NORM);

					pTrain->pev->speed = newspeed;
					auto pTrackTrain = dynamic_cast<CFuncTrackTrain*>(pTrain);
					if (pTrackTrain)
						pTrackTrain->Next();
				}
			}

			m_iTrain |= TRAIN_ACTIVE | TRAIN_NEW;

			m_hLastTrain = pTrain;
		}
	}

	// autostop train if we are not standing on it anymore
	if (!FBitSet(m_afPhysicsFlags, PFLAG_ONTRAIN))
	{
		if (m_hLastTrain && (m_hLastTrain->edict() != pev->groundentity) && m_hLastTrain->pev->speed > 0 && CVAR_GET_FLOAT("vr_train_autostop") != 0.f)
		{
			m_hLastTrain->pev->speed = 0;
			m_hLastTrain->pev->velocity = g_vecZero;
			m_hLastTrain->pev->avelocity = g_vecZero;
			m_hLastTrain->SetThink(nullptr);
			EHandleT<CFuncTrackTrain> hTrackTrain = m_hLastTrain;
			if (hTrackTrain)
			{
				hTrackTrain->StopSound();
			}
			m_hLastTrain = nullptr;
		}
	}

	if (FBitSet(m_iTrain, TRAIN_ACTIVE) && !FBitSet(m_afPhysicsFlags, PFLAG_ONTRAIN))
	{
		m_iTrain = TRAIN_NEW; // turn off train
	}
}

// Start of PostThink: flashlight, teleporter, laser spot and VR tank use
void CBasePlayer::VRPostThinkStart()
{
	UpdateFlashlight();
	UpdateVRTele();
	UpdateVRLaserSpot();

	VRUseOrUnuseTank();
	if (!m_vrIsUsingTankWithVRControllers && m_pTank != nullptr)
	{
		if (!m_pTank->OnControls(pev))
		{
			// player moved too far from the gun, unuse the gun
			m_pTank->Use(this, this, USE_OFF, 0);
			m_pTank = nullptr;
			SelectLastItem();
		}
		else if (m_pActiveItem != nullptr || 0 != pev->weaponmodel || m_vrControllers[VRControllerID::WEAPON].GetWeaponId() != WEAPON_BAREHAND)
		{
			// player selected a weapon, unuse the gun
			m_pTank->Use(this, this, USE_OFF, 0);
			m_pTank = nullptr;
			if (m_pActiveItem == nullptr)
			{
				SelectLastItem();
			}
		}
		else
		{
			// fire the gun
			m_pTank->Use(this, this, USE_SET, 2);
		}
	}
}

// PostThink, after ItemPostFrame
void CBasePlayer::VRPostFrameControllers()
{
	// controller feedback
	for (auto& [id, controller] : m_vrControllers)
	{
		if (controller.IsValid())
		{
			controller.PostFrame();
		}
	}
}

// Start of Killed
void CBasePlayer::VRKilled(entvars_t* pevAttacker, int iGib)
{
	FlashlightTurnOff();
	ClearLadderGrabbingControllers();
	StopPullingLedge();

	m_afPhysicsFlags &= ~PFLAG_ONBARNACLE;
	pev->flags &= ~FL_BARNACLED;

	// Debug death console message - Max Makes Mods, 2019-04-13
	// if (pevAttacker)
	// {
	// 	ALERT(at_console, "Killed by %s %s, damage type: %s, iGib: %s\n", STRING(pevAttacker->classname), STRING(pevAttacker->targetname), DamageBitsToString(bitsDamageType).data(), GibToString(iGib));
	// }
	// else
	// {
	// 	ALERT(at_console, "Killed by world, damage type: %s, iGib: %s\n", DamageBitsToString(bitsDamageType).data(), GibToString(iGib));
	// }

	// if (pevAttacker
	// 	&& FClassnameIs(pevAttacker, "func_tank")
	// 	&& (FStrEq(STRING(INDEXENT(0)->v.model), "maps/c2a5c.bsp") || FStrEq(STRING(INDEXENT(0)->v.model), "maps/c2a5e.bsp") || FStrEq(STRING(INDEXENT(0)->v.model), "maps/c2a2g.bsp"))
	// 	&& (FStrEq(STRING(pevAttacker->targetname), "sniper1") || FStrEq(STRING(pevAttacker->targetname), "sniper2")))
	// {
	// 	UTIL_VRGiveAchievement(this, VRAchievement::GEN_SNIPED);
	// }
}

// Save, after the PLAYER fields
void CBasePlayer::VRSave(CSave& save)
{
	StoreVROffsetsForLevelchange();

	save.WriteFields("PLAYERVROffsetsForLevelchange", &g_vrLevelChangeData, g_vrLevelChangeDataSaveData, (int)std::size(g_vrLevelChangeDataSaveData));
	// save.WriteFields("PLAYERVRAchievementsAndStatsData", &m_vrAchievementsAndStatsData, g_vrAchievementsAndStatsData, (int)std::size(g_vrAchievementsAndStatsData));
}

// Restore, after the PLAYER fields: VR fields and the VR offsets stored by the last level change
void CBasePlayer::VRRestore(CRestore& restore)
{
	restore.ReadFields("PLAYERVROffsetsForLevelchange", &g_vrLevelChangeData, g_vrLevelChangeDataSaveData, (int)std::size(g_vrLevelChangeDataSaveData));
	// restore.ReadFields("PLAYERVRAchievementsAndStatsData", &m_vrAchievementsAndStatsData, g_vrAchievementsAndStatsData, (int)std::size(g_vrAchievementsAndStatsData));

	// Restore VR offsets if levelchange has stored them (fixes origin issues in roomscale) - Max Makes Mods, 2018-04-02
	if (g_vrLevelChangeData.hasData)
	{
		this->vr_lastHMDOffset = g_vrLevelChangeData.lastHMDOffset;
		this->vr_ClientOriginOffset = g_vrLevelChangeData.clientOriginOffset;
		this->vr_prevYaw = g_vrLevelChangeData.prevYaw;
		this->vr_currentYaw = g_vrLevelChangeData.currentYaw;
		vr_needsToSendRestoreYawMsgToClient = true;
	}

	g_vrLevelChangeData.hasData = false;
}

// End of Restore
void CBasePlayer::VRRestoreEnd()
{
	// Make sure old predisaster savegames start up with hand model
	if (m_pActiveItem == nullptr)
	{
		pev->viewmodel = MAKE_STRING("models/v_hand.mdl");
		pev->weaponmodel = iStringNull;
	}

	vr_IsJustRestored = true;
	vr_hasSentRestoreYawMsgToClient = false;
	vr_hasSentSpawnYawToClient = false;

	VRGame::OnPlayerRestored(this); // per-game map hacks
}

// FlashlightTurnOn: the VR flashlight is a light entity, not EF_DIMLIGHT on the player
void CBasePlayer::VRFlashlightTurnOn()
{
	if (!hFlashLight)
	{
		hFlashLight = CBaseEntity::Create<CBaseEntity>("info_target", pev->origin, Vector());
		hFlashLight->m_objectCaps = FCAP_DONT_SAVE;
		SET_MODEL(hFlashLight->edict(), "sprites/black.spr");
		UTIL_SetOrigin(hFlashLight->pev, pev->origin);
		UTIL_SetSize(hFlashLight->pev, Vector(-4, -4, -4), Vector(4, 4, 4));
		hFlashLight->pev->effects = EF_NODRAW;
		hFlashLight->pev->rendermode = kRenderTransAlpha;
	}
	fFlashlightIsOn = true;
}

// FlashlightTurnOff
void CBasePlayer::VRFlashlightTurnOff()
{
	if (hFlashLight)
	{
		hFlashLight->pev->effects = EF_NODRAW;
		UTIL_Remove(hFlashLight);
		hFlashLight = nullptr;
	}
	if (hFlashlightMonster)
	{
		ClearBits(hFlashlightMonster->pev->effects, EF_DIMLIGHT);
		hFlashlightMonster = nullptr;
	}
	fFlashlightIsOn = false;
}

// UpdateClientData: where the VR train controls are
void CBasePlayer::VRSendTrainControls()
{
	if ((m_afPhysicsFlags & PFLAG_ONTRAIN) != 0)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgVRTrainControls, nullptr, pev);
		WRITE_COORD(vr_trainControlPosition.x);
		WRITE_COORD(vr_trainControlPosition.y);
		WRITE_COORD(vr_trainControlPosition.z);
		WRITE_ANGLE(vr_trainControlYaw);
		MESSAGE_END();
	}
}

// UpdateClientData: view yaw after loading a save or a level change
void CBasePlayer::VRSendRestoreYaw()
{
	if (vr_needsToSendRestoreYawMsgToClient)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgVRRestoreYaw, nullptr, pev);
		WRITE_ANGLE(this->vr_prevYaw);
		WRITE_ANGLE(this->vr_currentYaw);
		MESSAGE_END();
		vr_hasSentRestoreYawMsgToClient = true;
		vr_needsToSendRestoreYawMsgToClient = false;
	}
}

// Weapon aim from the controller: replace the SDK GetGunPosition and GetAutoaimVector (early return there)
Vector CBasePlayer::VRGetGunPosition()
{
	// Gun position and angles determined by attachments on weapon model - Max Makes Mods, 2019-03-30
	if (m_vrControllers[VRControllerID::WEAPON].IsValid())
	{
		Vector pos;
		bool result = m_vrControllers[VRControllerID::WEAPON].GetAttachment(VR_MUZZLE_ATTACHMENT, pos);
		if (result)
		{
			return pos;
		}
	}

	return GetWeaponPosition();
}

Vector CBasePlayer::GetAimAngles()
{
	return UTIL_VecToAngles(GetAutoaimVector());
}

Vector CBasePlayer::VRGetAutoaimVector()
{
	// Gun position and angles determined by attachments on weapon model - Max Makes Mods, 2019-03-30
	if (m_vrControllers[VRControllerID::WEAPON].IsValid())
	{
		Vector pos1;
		Vector pos2;
		bool result1 = m_vrControllers[VRControllerID::WEAPON].GetAttachment(VR_MUZZLE_ATTACHMENT, pos1);
		bool result2 = m_vrControllers[VRControllerID::WEAPON].GetAttachment(VR_MUZZLE_ATTACHMENT + 1, pos2);
		if (result1 && result2 && pos2 != pos1)
		{
			return (pos2 - pos1).Normalize();
		}
	}
	UTIL_MakeAimVectors(GetWeaponAngles());
	return gpGlobals->v_forward;
}
