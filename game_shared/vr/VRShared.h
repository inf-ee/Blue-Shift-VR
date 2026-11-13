#pragma once

#include <cstdint>

// Definitions shared by the client and the server VR code

enum class VRControllerID : int32_t
{
	WEAPON = 0,
	HAND,
	INVALID
};

// Index of the muzzle attachment in VR weapon and hand models (the attachment after it gives the aim direction)
#define VR_MUZZLE_ATTACHMENT 0

// "Weapon id" of a controller that holds no weapon. It is not the id of a weapon in any game: those are below
// MAX_WEAPONS (64), and Opposing Force uses the ids up to 25. Only the server uses it, and only to compare with;
// it is not saved or sent to the client.
constexpr int WEAPON_BAREHAND = 255;

// Client commands the client DLL sends to the server DLL (handled in dlls/vr/VRClientCommands.cpp)
#define VR_CMD_UPDATE_HMD "vrupd_hmd"
#define VR_CMD_UPDATE_CONTROLLER "vrupdctrl"
#define VR_CMD_WEAPON_ANIM "vr_wpnanim"
#define VR_CMD_MUZZLEFLASH "vr_muzzleflash" // also sent by EV_MuzzleFlash in cl_dll/ev_common.cpp
#define VR_CMD_TELEPORT "vrtele"

enum class VRPoseType
{
	LEFT_HAND,
	RIGHT_HAND,
	LEFT_POINTER,
	RIGHT_POINTER
};

enum class VREyeMode
{
	Both,
	LeftOnly,		   //  Left eye is rendered, right eye is black
	RightOnly,		   // Right eye is rendered,  left eye is black
	LeftOnlyMirrored,  //  Left eye is rendered, right eye is mirror of left eye
	RightOnlyMirrored, // Right eye is rendered,  left eye is mirror of right eye
	PancakeMode
};

struct VRQuaternion
{
	float x, y, z, w;
};
