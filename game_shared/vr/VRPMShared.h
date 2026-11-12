// VR movement declarations shared by pm_shared.cpp and vr/VRPMShared.cpp (moved from the top of pm_shared.cpp)

#pragma once

#include "Platform.h"
#include "mathlib.h"
#include "const.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_shared.h"

extern playermove_t* pmove; // pm_shared.cpp

// Set by VRControllerInteractionManager while the player grabs a ladder with the controllers (vr/VRPMShared.cpp)
extern bool pmoveIgnoreLadders;

// and again we are in include hell and use an extern declaration to escape :S
extern bool VRGlobalIsInstantAccelerateOn();
extern bool VRGlobalIsInstantDecelerateOn();
extern bool VRGlobalGetNoclipMode();
extern bool VRNotifyStuckEnt(int player, int ent);
extern bool VRGlobalIsPointInsideEnt(const float* point, int ent);
extern void VRGetMaxClimbSpeed(float& updown, float& sideways);
extern void VRGetCVARSpeeds(float& forwardSpeed, float& backSpeed, float& sideSpeed, float& upSpeed);
extern int VRGetLadderMode();
extern int VRGetGrabbedLadder(int player); // For client or server to use to identify (index into edicts or cl_entities)
inline bool VRIsGrabbingLadder() { return VRGetGrabbedLadder(pmove->player_index) > 0; }
extern bool VRIsPullingOnLedge(int player);
extern bool VRIsAutoDuckingEnabled(int player);
extern float VRGetSmoothStepsSetting();


// Forward declare methods, so we can move them around without the compiler going all "omg" - Max Makes Mods, 2018-04-01
void PM_CheckWaterJump(void);
void PM_CheckFalling(void);
void PM_PlayWaterSounds(void);
void PM_Jump(void);

#define DEFAULT_MOVEMENT_SPEED 270
#define DEFAULT_MAX_CLIMB_SPEED 200
#define DEFAULT_MAX_SIDEWAYS_SPEED 50

// VR movement functions, defined in vr/VRPMShared.cpp
bool IsInWaterAndIsGravityDisabled();
void PM_VR_MoveAwayFromStuckEntityIfPossible(Vector end, pmtrace_t& trace);
physent_t* PM_Ladder2(void);
bool PM_TryUnstuck();
void PM_YesClip(physent_t* pLadder);
void PM_HandleMovement(physent_t* pLadder);

// Hooks called from the SDK functions, defined in vr/VRPMShared.cpp
void PM_VRLadderMove(physent_t* pLadder);
void PM_VRMove(physent_t* pLadder);
bool PM_VRCheckStuck(qboolean server);
void PM_VRWaterWishVelocity(Vector& wishvel);
void PM_VRDuckSpeedAndHull();

// SDK functions in pm_shared.cpp called by the VR movement code in vr/VRPMShared.cpp
void PM_AddCorrectGravity();
void PM_AirMove();
void PM_CatagorizePosition();
bool PM_CheckStuck();
void PM_CheckVelocity();
bool PM_CheckWater();
void PM_Duck();
void PM_FixupGravityVelocity();
int PM_FlyMove();
void PM_Friction();
bool PM_InWater();
void PM_LadderMove(physent_t* pLadder);
void PM_NoClip();
void PM_Physics_Toss();
void PM_PlayerMove(qboolean server);
void PM_WalkMove();
void PM_WaterJump();
void PM_WaterMove();
