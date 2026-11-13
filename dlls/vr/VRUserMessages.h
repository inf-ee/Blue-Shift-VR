#pragma once

// User messages (server to client) of the VR code. Included by UserMessages.h, so every file that sees the SDK's gmsg* sees these.

inline int gmsgVRRestoreYaw = 0;
inline int gmsgVRGroundEntity = 0;
inline int gmsgVRSetSpawnYaw = 0;
inline int gmsgVRControllerEnt = 0;
inline int gmsgVRTrainControls = 0;
inline int gmsgVRGrabbedLadder = 0;
inline int gmsgVRPullingLedge = 0;
inline int gmsgVRUpdateEgon = 0;
inline int gmsgVRScreenShake = 0;
inline int gmsgVRTouch = 0;
inline int gmsgVRWalkedIntoWall = 0;
inline int gmsgVRLevelChange = 0;
inline int gmsgVRAchievement = 0;

// Registers the messages above, called at the end of LinkUserMessages (UserMessages.cpp)
void VRLinkUserMessages();
