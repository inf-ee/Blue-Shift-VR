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
