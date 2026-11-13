// VR gib limit and NPC reaction to thrown gibs, moved from combat.cpp and talkmonster.cpp

#include <algorithm>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "talkmonster.h"
#include "func_break.h"

#include "VRGameServer.h"

int CGib::m_numGibs = 0;

// CGib::Spawn: called first, removes old gibs if there are too many and counts the new one
void CGib::VRGibSpawning(const char* szGibModel)
{
	ASSERT(szGibModel && strlen(szGibModel) > 0);

	LimitNumberOfGibs();

	m_numGibs++;
}

// BounceGibTouch, StickyGibTouch: the first touch of a gib the player has thrown.
// Returns true if the gib was thrown (its hit sound replaces the bounce sound).
bool CGib::VRThrownGibTouch(CBaseEntity* pOther)
{
	if (!m_hThrower)
		return false;

	pOther->GibAttack(m_hThrower, pev->origin, m_bloodColor);
	m_hThrower = nullptr;

	// play a satisfying sound if we throw a gib
	float vel = pev->velocity.Length();
	float volume = std::min(1.f, vel / 450.f);
	int material = (pOther->BloodColor() == DONT_BLEED) ? m_material : Materials::matFlesh;
	CBreakable::MaterialSoundRandom(edict(), static_cast<Materials>(material), volume);
	return true;
}

void CGib::UpdateOnRemove()
{
	CBaseEntity::UpdateOnRemove();
	m_numGibs--;
}

void CGib::LimitNumberOfGibs()
{
	constexpr const int DEFAULT_MAX_GIBS = 50;
	int maxGibs = atoi(CVAR_GET_STRING("vr_max_interactive_debris"));
	if (maxGibs <= 0)
	{
		maxGibs = DEFAULT_MAX_GIBS;
	}

	if (m_numGibs >= maxGibs)
	{
		// Delete all gibs not in PVS
		int numGibs = m_numGibs;
		CBaseEntity* pGib = nullptr;
		int counter = 0;
		while (counter < m_numGibs && (pGib = UTIL_FindEntityByClassname(pGib, "gib")))
		{
			if (!pGib->m_isInPVS)
			{
				UTIL_Remove(pGib);
				numGibs--;
			}
			counter++;
		}

		// too many gibs are in PVS! remove 1/4 of all gibs immediately
		// (this automagically deletes gibs by age, as older gibs are "earlier" in the engine's edict array)
		counter = 0;
		while (counter < m_numGibs && numGibs >= (maxGibs * 3 / 4) && (pGib = UTIL_FindEntityByClassname(pGib, "gib")))
		{
			// Don't remove gibs currently being dragged
			if (!pGib->m_vrDragger)
			{
				UTIL_Remove(pGib);
				numGibs--;
			}
			counter++;
		}
		m_numGibs = numGibs;

		// Fade out enough gibs to reduce to half of max gibs
		// (this automagically fades out gibs by age, as older gibs are "earlier" in the engine's edict array)
		int fadeoutgibs = m_numGibs - maxGibs / 2;
		pGib = nullptr;
		while (fadeoutgibs > 0 && (pGib = UTIL_FindEntityByClassname(pGib, "gib")))
		{
			// Don't fade out gibs currently being dragged
			if (!pGib->m_vrDragger)
			{
				pGib->SetThink(&CGib::SUB_StartFadeOut);
				pGib->pev->nextthink = gpGlobals->time;
			}
			fadeoutgibs--;
		}
	}
}

// react to player throwing stuff at me
void CTalkMonster::GibAttack(EHandleT<CBaseEntity> thrower, const Vector& pos, int bloodcolor)
{
	if (!thrower || !thrower->IsPlayer())
		return;

	if (!FOkToSpeak())
		return;

	if ((bloodcolor == BLOOD_COLOR_RED || bloodcolor == BLOOD_COLOR_GREEN) && RANDOM_LONG(0, 1) == 0)
	{
		PlaySentence(m_szGrp[TLK_GIB_BLOODY], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
	}
	else
	{
		PlaySentence(m_szGrp[TLK_GIB_NEUTRAL], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
	}

	MakeIdealYaw(thrower->pev->origin);
	IdleHeadTurn(thrower->pev->origin);


	VRGame::OnGibCaught(this, thrower);
}
