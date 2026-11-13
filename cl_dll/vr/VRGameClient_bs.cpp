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
