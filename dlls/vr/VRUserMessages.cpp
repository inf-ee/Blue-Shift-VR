#include "extdll.h"
#include "util.h"

#include "VRUserMessages.h"

void VRLinkUserMessages()
{
	gmsgVRRestoreYaw = REG_USER_MSG("VRRstrYaw", 2);
	gmsgVRGroundEntity = REG_USER_MSG("GroundEnt", 2);
	gmsgVRSetSpawnYaw = REG_USER_MSG("VRSpawnYaw", 1);
	gmsgVRControllerEnt = REG_USER_MSG("VRCtrlEnt", -1);
	gmsgVRTrainControls = REG_USER_MSG("TrainCtrl", 7);
	gmsgVRGrabbedLadder = REG_USER_MSG("GrbdLddr", 2);
	gmsgVRPullingLedge = REG_USER_MSG("PullLdg", 1);
	gmsgVRUpdateEgon = REG_USER_MSG("VRUpdEgon", -1);
	gmsgVRScreenShake = REG_USER_MSG("VRScrnShke", 3 * sizeof(float));
	gmsgVRTouch = REG_USER_MSG("VRTouch", 5);
	gmsgVRWalkedIntoWall = REG_USER_MSG("VRWlkWl", 0);
	gmsgVRLevelChange = REG_USER_MSG("VRLvlChng", 0);
	gmsgVRAchievement = REG_USER_MSG("VRAchvmnt", sizeof(int));
}
