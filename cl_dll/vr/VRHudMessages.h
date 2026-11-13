#pragma once

// State the server sends to the VR client code through user messages (handlers in VRHudMessages.cpp)

#include "cl_dll.h"
#include "vr/VRControllerModelData.h"

struct cl_entity_s;

struct VRHudState
{
	ControllerModelData m_leftControllerModelData{0};
	ControllerModelData m_rightControllerModelData{0};

	int m_vrGrabbedLadderEntIndex{-1};
	bool m_vrIsPullingOnLedge{false};

	float m_screenShakeAmplitude = 0;
	float m_screenShakeDuration = 0;
	float m_screenShakeFrequency = 0;
	bool m_hasScreenShake = false;

	float m_vrLeftHandTouchVibrateIntensity = 0.f;
	float m_vrRightHandTouchVibrateIntensity = 0.f;

	// Ground entity for rotating with them in VR (since player rotation is done client side completely) - Max Makes Mods, 2019-04-09
	cl_entity_s* GetGroundEntity();

	// HUD (train control sprites) attachment for train we are controlling
	bool GetTrainControlsOriginAndOrientation(Vector& origin, Vector& angles);

	bool MsgFunc_GroundEnt(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_VRCtrlEnt(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_TrainCtrl(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_VRScrnShke(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_GrbdLddr(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_PullLdg(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_VRTouch(const char* pszName, int iSize, void* pbuf);

private:
	int m_iGroundEntIndex{0};
	Vector m_trainControlPosition;
	float m_trainControlYaw{0.f};
};

extern VRHudState gVRHudState;
