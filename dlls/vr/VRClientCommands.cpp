// VR client commands (controller and headset updates, poses, weapon animations), moved from ClientCommand in client.cpp

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"

// Returns true if pcmd is a VR command and was handled
bool VRClientCommand(edict_t* pEntity, CBasePlayer* player, const char* pcmd)
{
	if (FStrEq(pcmd, "vr_flashlight"))
	{
		if (0 != atoi(CMD_ARGV(1)))
		{
			Vector offset(atof(CMD_ARGV(2)), atof(CMD_ARGV(3)), atof(CMD_ARGV(4)));
			Vector angles(atof(CMD_ARGV(5)), atof(CMD_ARGV(6)), atof(CMD_ARGV(7)));
			player->SetFlashlightPose(offset, angles);
		}
		else
		{
			player->ClearFlashlightPose();
		}
	}
	else if (FStrEq(pcmd, "vr_teleporter"))
	{
		if (0 != atoi(CMD_ARGV(1)))
		{
			Vector offset(atof(CMD_ARGV(2)), atof(CMD_ARGV(3)), atof(CMD_ARGV(4)));
			Vector angles(atof(CMD_ARGV(5)), atof(CMD_ARGV(6)), atof(CMD_ARGV(7)));
			player->SetTeleporterPose(offset, angles);
		}
		else
		{
			player->ClearTeleporterPose();
		}
	}
	else if (FStrEq(pcmd, VR_CMD_WEAPON_ANIM)) // Client side weapon animations are now sent to the server - Max Makes Mods, 2019-04-13
	{
		int sequence = atoi(CMD_ARGV(1));
		int body = atoi(CMD_ARGV(2));
		player->PlayVRWeaponAnimation(sequence, body);
	}
	else if (FStrEq(pcmd, VR_CMD_MUZZLEFLASH)) // Client side weapon animations are now sent to the server - Max Makes Mods, 2019-04-13
	{
		player->PlayVRWeaponMuzzleflash();
	}
	else if (FStrEq(pcmd, VR_CMD_UPDATE_HMD)) // Client sends update for VR related data - Max Makes Mods, 2017-08-18
	{
		int size = CMD_ARGC();
		if (size == 14)
		{
			int timestamp = atoi(CMD_ARGV(1));
			Vector2D offset(atof(CMD_ARGV(2)), atof(CMD_ARGV(3)));
			float offsetZ = atof(CMD_ARGV(4));
			Vector forward(atof(CMD_ARGV(5)), atof(CMD_ARGV(6)), atof(CMD_ARGV(7)));
			Vector2D yawOffsetDelta(atof(CMD_ARGV(8)), atof(CMD_ARGV(9)));
			float prevYaw = atof(CMD_ARGV(10));
			float currentYaw = atof(CMD_ARGV(11));
			bool hasReceivedRestoreYawMsg = atoi(CMD_ARGV(12)) != 0;
			bool hasReceivedSpawnYaw = atoi(CMD_ARGV(13)) != 0;
			player->UpdateVRHeadset(timestamp, offset, offsetZ, forward, yawOffsetDelta, prevYaw, currentYaw, hasReceivedRestoreYawMsg, hasReceivedSpawnYaw);
		}
		else
		{
			char errormsg[1024] = {0};
			snprintf(errormsg, sizeof(errormsg), "Invalid vr update (%i): %s %s!\n", size, pcmd, CMD_ARGS());
			ClientPrint(&pEntity->v, HUD_PRINTCONSOLE, errormsg);
		}
	}
	else if (FStrEq(pcmd, VR_CMD_UPDATE_CONTROLLER)) // Client sends update for VR related data - Max Makes Mods, 2017-08-18
	{
		int size = CMD_ARGC();
		if (size == 16)
		{
			int timestamp = atoi(CMD_ARGV(1));
			bool isValid = atoi(CMD_ARGV(2)) != 0;
			VRControllerID id = VRControllerID(atoi(CMD_ARGV(3)));
			bool isMirrored = atoi(CMD_ARGV(4)) != 0;
			Vector offset(atof(CMD_ARGV(5)), atof(CMD_ARGV(6)), atof(CMD_ARGV(7)));
			Vector angles(atof(CMD_ARGV(8)), atof(CMD_ARGV(9)), atof(CMD_ARGV(10)));
			Vector velocity(atof(CMD_ARGV(11)), atof(CMD_ARGV(12)), atof(CMD_ARGV(13)));
			bool isDragging = atoi(CMD_ARGV(14)) != 0;
			bool isFiring = atoi(CMD_ARGV(15)) != 0;
			player->UpdateVRController(id, timestamp, isValid, isMirrored, offset, angles, velocity, isDragging, isFiring);
		}
		else
		{
			char errormsg[1024] = {0};
			snprintf(errormsg, sizeof(errormsg), "Invalid vr update (%i): %s %s!\n", size, pcmd, CMD_ARGS());
			ClientPrint(&pEntity->v, HUD_PRINTCONSOLE, errormsg);
		}
	}
	else if (FStrEq(pcmd, VR_CMD_TELEPORT))
	{
		if (0 != atoi(CMD_ARGV(1)))
		{
			player->StartVRTele();
		}
		else
		{
			player->StopVRTele();
		}
	}
	else
	{
		return false;
	}
	return true;
}
