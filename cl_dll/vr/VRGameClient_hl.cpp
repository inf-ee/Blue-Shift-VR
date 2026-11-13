// Half-Life data and hooks for the client VR code, see VRGameClient.h
// The sky table is moved verbatim from Half-Life: VR (VRTextureHelper.cpp).
// Not compiled in the Blue Shift and Opposing Force builds.

#include <string>
#include <unordered_map>

#include "VRGameClient.h"

namespace
{
const std::unordered_map<std::string, std::string> m_mapSkyboxNames{
	{{"maps/c0a0.bsp", "desert"},
	 {"maps/c0a0a.bsp", "desert"},
	 {"maps/c0a0b.bsp", "2desert"},
	 {"maps/c0a0c.bsp", "desert"},
	 {"maps/c0a0d.bsp", "desert"},
	 {"maps/c0a0e.bsp", "desert"},
	 {"maps/c1a0.bsp", "desert"},
	 {"maps/c1a0a.bsp", "desert"},
	 {"maps/c1a0b.bsp", "desert"},
	 {"maps/c1a0c.bsp", "desert"},
	 {"maps/c1a0d.bsp", "desert"},
	 {"maps/c1a0e.bsp", "xen9"},
	 {"maps/c1a1.bsp", "desert"},
	 {"maps/c1a1a.bsp", "desert"},
	 {"maps/c1a1b.bsp", "desert"},
	 {"maps/c1a1c.bsp", "desert"},
	 {"maps/c1a1d.bsp", "desert"},
	 {"maps/c1a1f.bsp", "desert"},
	 {"maps/c1a2.bsp", "desert"},
	 {"maps/c1a2a.bsp", "desert"},
	 {"maps/c1a2b.bsp", "desert"},
	 {"maps/c1a2c.bsp", "desert"},
	 {"maps/c1a2d.bsp", "desert"},
	 {"maps/c1a3.bsp", "desert"},
	 {"maps/c1a3a.bsp", "desert"},
	 {"maps/c1a3b.bsp", "dusk"},
	 {"maps/c1a3c.bsp", "dusk"},
	 {"maps/c1a3d.bsp", "desert"},
	 {"maps/c1a4.bsp", "desert"},
	 {"maps/c1a4b.bsp", "desert"},
	 {"maps/c1a4d.bsp", "desert"},
	 {"maps/c1a4e.bsp", "desert"},
	 {"maps/c1a4f.bsp", "desert"},
	 {"maps/c1a4g.bsp", "desert"},
	 {"maps/c1a4i.bsp", "desert"},
	 {"maps/c1a4j.bsp", "desert"},
	 {"maps/c1a4k.bsp", "desert"},
	 {"maps/c2a1.bsp", "desert"},
	 {"maps/c2a1a.bsp", "desert"},
	 {"maps/c2a1b.bsp", "desert"},
	 {"maps/c2a2.bsp", "night"},
	 {"maps/c2a2a.bsp", "day"},
	 {"maps/c2a2b1.bsp", "desert"},
	 {"maps/c2a2b2.bsp", "desert"},
	 {"maps/c2a2c.bsp", "desert"},
	 {"maps/c2a2d.bsp", "desert"},
	 {"maps/c2a2e.bsp", "night"},
	 {"maps/c2a2f.bsp", "desert"},
	 {"maps/c2a2g.bsp", "night"},
	 {"maps/c2a2h.bsp", "night"},
	 {"maps/c2a3.bsp", "desert"},
	 {"maps/c2a3a.bsp", "desert"},
	 {"maps/c2a3b.bsp", "desert"},
	 {"maps/c2a3c.bsp", "dawn"},
	 {"maps/c2a3d.bsp", "2desert"},
	 {"maps/c2a3e.bsp", "desert"},
	 {"maps/c2a4.bsp", "morning"},
	 {"maps/c2a4a.bsp", "desert"},
	 {"maps/c2a4b.bsp", "desert"},
	 {"maps/c2a4c.bsp", "desert"},
	 {"maps/c2a4d.bsp", "desert"},
	 {"maps/c2a4e.bsp", "desert"},
	 {"maps/c2a4f.bsp", "desert"},
	 {"maps/c2a4g.bsp", "desert"},
	 {"maps/c2a5.bsp", "desert"},
	 {"maps/c2a5a.bsp", "cliff"},
	 {"maps/c2a5b.bsp", "desert"},
	 {"maps/c2a5c.bsp", "desert"},
	 {"maps/c2a5d.bsp", "desert"},
	 {"maps/c2a5e.bsp", "desert"},
	 {"maps/c2a5f.bsp", "desert"},
	 {"maps/c2a5g.bsp", "desert"},
	 {"maps/c2a5w.bsp", "desert"},
	 {"maps/c2a5x.bsp", "desert"},
	 {"maps/c3a1.bsp", "desert"},
	 {"maps/c3a1a.bsp", "desert"},
	 {"maps/c3a1b.bsp", "desert"},
	 {"maps/c3a2.bsp", "desert"},
	 {"maps/c3a2a.bsp", "desert"},
	 {"maps/c3a2b.bsp", "desert"},
	 {"maps/c3a2c.bsp", "desert"},
	 {"maps/c3a2d.bsp", "desert"},
	 {"maps/c3a2e.bsp", "desert"},
	 {"maps/c3a2f.bsp", "desert"},
	 {"maps/c4a1.bsp", "neb7"},
	 {"maps/c4a1a.bsp", "neb7"},
	 {"maps/c4a1b.bsp", "alien1"},
	 {"maps/c4a1c.bsp", "xen8"},
	 {"maps/c4a1d.bsp", "xen8"},
	 {"maps/c4a1e.bsp", "xen8"},
	 {"maps/c4a1f.bsp", "black"},
	 {"maps/c4a2.bsp", "neb6"},
	 {"maps/c4a2a.bsp", "neb6"},
	 {"maps/c4a2b.bsp", "neb6"},
	 {"maps/c4a3.bsp", "black"},
	 {"maps/c5a1.bsp", "xen10"}}
};
} // namespace

const char* VRGame::GetSkyboxName(const char* mapName)
{
	auto skyboxName = m_mapSkyboxNames.find(mapName);
	if (skyboxName != m_mapSkyboxNames.end())
		return skyboxName->second.data();
	else
		return nullptr;
}

const char* VRGame::GetModTitle()
{
	return "Half-Life: VR";
}

const char* VRGame::GetVersionFilePath()
{
	return "./valve/liblist.gam";
}

namespace
{
// Weapon ids, the same as WEAPON_* in dlls/weapons.h (not included, it needs the server headers).
// Ids work for any alias of the weapon entity, the weapon name the client gets is the classname the map used.
constexpr int ID_CROWBAR = 1;
constexpr int ID_GLOCK = 2;
constexpr int ID_PYTHON = 3;
constexpr int ID_MP5 = 4;
constexpr int ID_CROSSBOW = 6;
constexpr int ID_SHOTGUN = 7;
constexpr int ID_RPG = 8;
constexpr int ID_GAUSS = 9;
constexpr int ID_EGON = 10;
constexpr int ID_HORNETGUN = 11;
constexpr int ID_HANDGRENADE = 12;
constexpr int ID_TRIPMINE = 13;
constexpr int ID_SATCHEL = 14;
constexpr int ID_SNARK = 15;

// Weapons of each spoke from the center outwards, 0 is unused. Only the weapons the player has are drawn,
// one after the other without gaps, so the spoke grows as weapons are picked up. Weapons that don't exist
// in the game (like the crossbow in Blue Shift) never show up.
constexpr int SPOKES[VR_WHEEL_NUM_SPOKES][VR_WHEEL_SPOKE_LENGTH] = {
	{ID_GLOCK, ID_PYTHON, 0},				   // 12 o'clock: handguns
	{ID_MP5, ID_SHOTGUN, ID_CROSSBOW},		   // 2 o'clock: long guns
	{ID_RPG, ID_GAUSS, ID_EGON},			   // 4 o'clock: heavy
	{ID_CROWBAR, 0, 0},						   // 6 o'clock: melee
	{ID_SNARK, ID_HORNETGUN, 0},			   // 8 o'clock: alien
	{ID_HANDGRENADE, ID_SATCHEL, ID_TRIPMINE}, // 10 o'clock: explosives
};
} // namespace

int VRGame::GetWeaponWheelWeapon(int spoke, int index)
{
	return SPOKES[spoke][index];
}
