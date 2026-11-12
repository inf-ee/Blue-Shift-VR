// VR declarations for pm_shared.cpp, moved from its top. Included only from pm_shared.cpp, right after pmove is defined.

#pragma once

// Set by VRControllerInteractionManager while the player grabs a ladder with the controllers
bool pmoveIgnoreLadders = false;

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

// VR movement functions, defined in VRPMSharedFunctions.h (included at the end of pm_shared.cpp)
bool IsInWaterAndIsGravityDisabled();
void PM_VR_MoveAwayFromStuckEntityIfPossible(Vector end, pmtrace_t& trace);
physent_t* PM_Ladder2(void);
bool PM_TryUnstuck();
void PM_YesClip(physent_t* pLadder);
void PM_HandleMovement(physent_t* pLadder);
