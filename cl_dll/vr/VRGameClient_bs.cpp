// Half-Life: Blue Shift data and hooks for the client VR code, see VRGameClient.h

#include <string>
#include <unordered_map>

#include "VRGameClient.h"

namespace
{
const std::unordered_map<std::string, std::string> m_mapSkyboxNames{
	{{"maps/ba_canal1.bsp", "desert"},
		{"maps/ba_canal1b.bsp", "desert"},
		{"maps/ba_canal2.bsp", "desert"},
		{"maps/ba_canal3.bsp", "desert"},
		{"maps/ba_elevator.bsp", "desert"},
		{"maps/ba_hazard1.bsp", "desert"},
		{"maps/ba_hazard2.bsp", "desert"},
		{"maps/ba_hazard3.bsp", "desert"},
		{"maps/ba_hazard4.bsp", "desert"},
		{"maps/ba_hazard5.bsp", "desert"},
		{"maps/ba_hazard6.bsp", "desert"},
		{"maps/ba_maint.bsp", "desert"},
		{"maps/ba_outro.bsp", "2desert"},
		{"maps/ba_power1.bsp", "desert"},
		{"maps/ba_power2.bsp", "desert"},
		{"maps/ba_security1.bsp", "desert"},
		{"maps/ba_security2.bsp", "desert"},
		{"maps/ba_teleport1.bsp", "desert"},
		{"maps/ba_teleport2.bsp", "desert"},
		{"maps/ba_tram1.bsp", "desert"},
		{"maps/ba_tram2.bsp", "desert"},
		{"maps/ba_tram3.bsp", "desert"},
		{"maps/ba_xen1.bsp", "alien1"},
		{"maps/ba_xen2.bsp", "alien1"},
		{"maps/ba_xen3.bsp", "alien1"},
		{"maps/ba_xen4.bsp", "alien1"},
		{"maps/ba_xen5.bsp", "alien1"},
		{"maps/ba_xen6.bsp", "alien1"},
		{"maps/ba_yard1.bsp", "desert"},
		{"maps/ba_yard2.bsp", "desert"},
		{"maps/ba_yard3.bsp", "desert"},
		{"maps/ba_yard3a.bsp", "desert"},
		{"maps/ba_yard3b.bsp", "desert"},
		{"maps/ba_yard4.bsp", "desert"},
		{"maps/ba_yard4a.bsp", "desert"},
		{"maps/ba_yard5.bsp", "desert"},
		{"maps/ba_yard5a.bsp", "desert"}}};
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
