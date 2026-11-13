#pragma once

// Weapon slot helpers for the HUD's weapon list (VRHudAmmo.cpp). A slot or position without a weapon gives -1.

int VRFirstSlotWithWeapon();
int VRFirstSlotPosWithWeapon(int slot);
int VRLastSlotWithWeapon();
int VRLastSlotPosWithWeapon(int slot);

// True if the weapon the player holds is the first / last one of the weapon list
bool VRIsCurrentWeaponFirstWeapon();
bool VRIsCurrentWeaponLastWeapon();
