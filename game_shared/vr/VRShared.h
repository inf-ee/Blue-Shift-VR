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
