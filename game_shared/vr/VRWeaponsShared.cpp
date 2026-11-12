// VR methods of weapon and player classes whose code is built into both DLLs (moved from the weapon .cpp files)

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"

// moved from dlls/crowbar.cpp
void CCrowbar::ItemPostFrame()
{
#ifndef CLIENT_DLL
	Vector weaponVelocity = m_pPlayer->GetWeaponVelocity();
	float speed = weaponVelocity.Length();
	if (speed >= GetMeleeSwingSpeed())
	{
		if (!playedWooshSound)
		{
			// prevent w-w-woo-woosh stutters when player waves crowbar around frantically
			if (UTIL_WeaponTimeBase() > lastWooshSoundTime + 0.5f)
			{
				EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "weapons/cbar_miss1.wav", 1, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				lastWooshSoundTime = UTIL_WeaponTimeBase();
			}
			playedWooshSound = true;
		}
	}
	else
	{
		playedWooshSound = false;
	}
#endif

	if (CVAR_GET_FLOAT("vr_crowbar_vanilla_attack_enabled") != 0.f && (m_pPlayer->pev->button & IN_ATTACK) != 0)
	{
		// Blue Shift SDK has one shared ItemPostFrame/CanAttack in weapons_shared.cpp for both DLLs,
		// so upstream's client-only CanAttack(float) from hl_weapons.cpp does not exist here.
		// #ifdef CLIENT_DLL
		// 		extern bool CanAttack(float flNextAttack);
		// 		if (CanAttack(m_flNextPrimaryAttack))
		// #else
		extern bool CanAttack(CBasePlayer * pPlayer, float attack_time, float curtime, bool isPredicted);
		if (CanAttack(m_pPlayer, m_flNextPrimaryAttack, gpGlobals->time, UseDecrement()))
		// #endif
		{
			PrimaryAttack();
		}
	}
}

// moved from dlls/tripmine.cpp
#ifndef CLIENT_DLL
void CTripmine::UpdateGhost()
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		if (m_hGhost)
		{
			UTIL_Remove(m_hGhost);
		}
		m_hGhost = nullptr;
		return;
	}

	float distance = 0.f;
	if (CVAR_GET_FLOAT("vr_weapon_grenade_mode") != 0.f)
	{
		distance = 128.f;
	}
	else
	{
		distance = VR_TRIPMINE_PLACEMENT_DISTANCE;
	}

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector();

	TraceResult tr;

	UTIL_TraceLine(vecSrc, vecSrc + (vecAiming * distance), dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	bool hideGhost = true;
	if (tr.flFraction < 1.0)
	{
		CBaseEntity* pEntity = CBaseEntity::SafeInstance<CBaseEntity>(tr.pHit);
		if (pEntity && (pEntity->pev->flags & FL_CONVEYOR) == 0)
		{
			Vector origin = tr.vecEndPos;
			if (!m_hGhost)
			{
				m_hGhost = CSprite::SpriteCreate("models/v_tripmine.mdl", origin, false);
				m_hGhost->pev->spawnflags |= SF_SPRITE_TEMPORARY | SF_SPRITE_STARTON;
				m_hGhost->pev->rendermode = kRenderTransTexture;
				m_hGhost->pev->renderamt = 50;
				m_hGhost->pev->body = 3;
				m_hGhost->pev->sequence = TRIPMINE_WORLD;
			}
			m_hGhost->pev->angles = UTIL_VecToAngles(tr.vecPlaneNormal);
			m_hGhost->pev->origin = origin;
			m_hGhost->pev->effects &= ~EF_NODRAW;
			hideGhost = false;
		}
	}

	if (m_hGhost && hideGhost)
	{
		m_hGhost->pev->effects |= EF_NODRAW;
	}
}
#endif

// moved from dlls/weapons_shared.cpp
// VR version: with no last item, switch to the bare hand
void CBasePlayer::VRSelectLastItem()
{
	if (m_pActiveItem != nullptr && !m_pActiveItem->CanHolster())
	{
		return;
	}

	ResetAutoaim();

	// FIX, this needs to queue them up and delay
	if (m_pActiveItem != nullptr)
	{
		m_pActiveItem->Holster();
	}

	if (m_pLastItem == nullptr)
	{
		m_pLastItem = m_pActiveItem;
		m_pActiveItem = nullptr;
		pev->viewmodel = MAKE_STRING("models/v_hand.mdl");
		pev->weaponmodel = iStringNull;
	}
	else
	{
		CBasePlayerItem* pTemp = m_pActiveItem;
		m_pActiveItem = m_pLastItem;
		m_pLastItem = pTemp;
		m_pActiveItem->Deploy();
		m_pActiveItem->UpdateItemInfo();
	}
}
