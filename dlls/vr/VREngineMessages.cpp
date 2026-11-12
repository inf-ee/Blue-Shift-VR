// Engine message functions that drop messages with a bogus type instead of crashing, swapped into g_engfuncs by VRSwapEngineMessages

#include "extdll.h"
#include "util.h"

namespace
{
// The engine's own functions
void (*s_engineMessageBegin)(int msg_dest, int msg_type, const float* pOrigin, edict_t* ed);
void (*s_engineMessageEnd)();
void (*s_engineWriteByte)(int iValue);
void (*s_engineWriteChar)(int iValue);
void (*s_engineWriteShort)(int iValue);
void (*s_engineWriteLong)(int iValue);
void (*s_engineWriteAngle)(float flValue);
void (*s_engineWriteCoord)(float flValue);
void (*s_engineWriteString)(const char* sz);
void (*s_engineWriteEntity)(int iValue);

bool s_isBogusMessage = false;

void VRMessageBegin(int msg_dest, int msg_type, const float* pOrigin, edict_t* ed)
{
	if (s_isBogusMessage)
	{
		g_engfuncs.pfnAlertMessage(at_error, "Last message was bogus and not finished before new message!\n");
		s_isBogusMessage = false;
	}

	// Very very very rarely for some reason the game might crash with
	// "Tried to create a message with a bogus message type ( 0 )".
	// We catch that here and log it instead of crashing.
	if (msg_type == 0)
	{
		s_isBogusMessage = true;
		g_engfuncs.pfnAlertMessage(at_error, "Tried to send a bogus message, intercepted: dest %i\n", msg_dest);
		if (pOrigin)
		{
			g_engfuncs.pfnAlertMessage(at_error, "Bogus message pOrigin: %f %f %f\n", pOrigin[0], pOrigin[1], pOrigin[2]);
		}
		if (ed)
		{
			g_engfuncs.pfnAlertMessage(at_error, "Bogus message ed\n");
		}
		return;
	}

	s_engineMessageBegin(msg_dest, msg_type, pOrigin, ed);
}

void VRMessageEnd()
{
	if (s_isBogusMessage)
	{
		g_engfuncs.pfnAlertMessage(at_error, "Bogus message end.\n");
		s_isBogusMessage = false;
		return;
	}
	s_engineMessageEnd();
}

void VRWriteByte(int iValue)
{
	if (s_isBogusMessage)
	{
		g_engfuncs.pfnAlertMessage(at_error, "Bogus message byte: %i\n", iValue);
		return;
	}
	s_engineWriteByte(iValue);
}

void VRWriteChar(int iValue)
{
	if (s_isBogusMessage)
	{
		g_engfuncs.pfnAlertMessage(at_error, "Bogus message char: %i\n", iValue);
		return;
	}
	s_engineWriteChar(iValue);
}

void VRWriteShort(int iValue)
{
	if (s_isBogusMessage)
	{
		g_engfuncs.pfnAlertMessage(at_error, "Bogus message short: %i\n", iValue);
		return;
	}
	s_engineWriteShort(iValue);
}

void VRWriteLong(int iValue)
{
	if (s_isBogusMessage)
	{
		g_engfuncs.pfnAlertMessage(at_error, "Bogus message long: %i\n", iValue);
		return;
	}
	s_engineWriteLong(iValue);
}

void VRWriteAngle(float flValue)
{
	if (s_isBogusMessage)
	{
		g_engfuncs.pfnAlertMessage(at_error, "Bogus message angle: %f\n", flValue);
		return;
	}
	s_engineWriteAngle(flValue);
}

void VRWriteCoord(float flValue)
{
	if (s_isBogusMessage)
	{
		g_engfuncs.pfnAlertMessage(at_error, "Bogus message coord: %f\n", flValue);
		return;
	}
	s_engineWriteCoord(flValue);
}

void VRWriteString(const char* sz)
{
	if (s_isBogusMessage)
	{
		g_engfuncs.pfnAlertMessage(at_error, "Bogus message string: %s\n", sz);
		return;
	}
	s_engineWriteString(sz);
}

void VRWriteEntity(int iValue)
{
	if (s_isBogusMessage)
	{
		g_engfuncs.pfnAlertMessage(at_error, "Bogus message entity: %i\n", iValue);
		return;
	}
	s_engineWriteEntity(iValue);
}
} // namespace

// Every MESSAGE_BEGIN, MESSAGE_END and WRITE_* in the DLL goes through g_engfuncs
void VRSwapEngineMessages()
{
	s_engineMessageBegin = g_engfuncs.pfnMessageBegin;
	s_engineMessageEnd = g_engfuncs.pfnMessageEnd;
	s_engineWriteByte = g_engfuncs.pfnWriteByte;
	s_engineWriteChar = g_engfuncs.pfnWriteChar;
	s_engineWriteShort = g_engfuncs.pfnWriteShort;
	s_engineWriteLong = g_engfuncs.pfnWriteLong;
	s_engineWriteAngle = g_engfuncs.pfnWriteAngle;
	s_engineWriteCoord = g_engfuncs.pfnWriteCoord;
	s_engineWriteString = g_engfuncs.pfnWriteString;
	s_engineWriteEntity = g_engfuncs.pfnWriteEntity;

	g_engfuncs.pfnMessageBegin = VRMessageBegin;
	g_engfuncs.pfnMessageEnd = VRMessageEnd;
	g_engfuncs.pfnWriteByte = VRWriteByte;
	g_engfuncs.pfnWriteChar = VRWriteChar;
	g_engfuncs.pfnWriteShort = VRWriteShort;
	g_engfuncs.pfnWriteLong = VRWriteLong;
	g_engfuncs.pfnWriteAngle = VRWriteAngle;
	g_engfuncs.pfnWriteCoord = VRWriteCoord;
	g_engfuncs.pfnWriteString = VRWriteString;
	g_engfuncs.pfnWriteEntity = VRWriteEntity;
}
