// VR weapon slot helpers for the HUD's weapon list, moved from ammo.cpp

#include "hud.h"
#include "cl_util.h"

#include "VRHudAmmo.h"

#include "ammohistory.h"

int VRFirstSlotWithWeapon()
{
	for (int slot = 0; slot < MAX_WEAPON_SLOTS; slot++)
	{
		if (VRFirstSlotPosWithWeapon(slot) >= 0)
		{
			return slot;
		}
	}
	return -1;
}

int VRFirstSlotPosWithWeapon(const int slot)
{
	for (int pos = 0; pos < MAX_WEAPON_POSITIONS; pos++)
	{
		WEAPON* wsp = gWR.GetWeaponSlot(slot, pos);
		if (wsp && gWR.HasAmmo(wsp))
		{
			return pos;
		}
	}
	return -1;
}

int VRLastSlotWithWeapon()
{
	for (int slot = MAX_WEAPON_SLOTS - 1; slot >= 0; slot--)
	{
		if (VRFirstSlotPosWithWeapon(slot) >= 0)
		{
			return slot;
		}
	}
	return -1;
}

int VRLastSlotPosWithWeapon(const int slot)
{
	for (int pos = MAX_WEAPON_POSITIONS - 1; pos >= 0; pos--)
	{
		WEAPON* wsp = gWR.GetWeaponSlot(slot, pos);
		if (wsp && gWR.HasAmmo(wsp))
		{
			return pos;
		}
	}
	return -1;
}

bool VRIsCurrentWeaponFirstWeapon()
{
	const WEAPON* m_pWeapon = gHUD.m_Ammo.m_pWeapon;
	return m_pWeapon != nullptr && m_pWeapon->iSlot <= VRFirstSlotWithWeapon() && m_pWeapon->iSlotPos <= VRFirstSlotPosWithWeapon(m_pWeapon->iSlot);
}

bool VRIsCurrentWeaponLastWeapon()
{
	const WEAPON* m_pWeapon = gHUD.m_Ammo.m_pWeapon;
	return m_pWeapon != nullptr && m_pWeapon->iSlot >= VRLastSlotWithWeapon() && m_pWeapon->iSlotPos >= VRLastSlotPosWithWeapon(m_pWeapon->iSlot);
}

// For weapons_shared.cpp and hl_weapons.cpp - Max Makes Mods, 2018-02-04
bool GetHUDWeaponBlocked()
{
	return (gHUD.m_iHideHUDDisplay & HIDEHUD_WEAPONBLOCKED) != 0;
}
