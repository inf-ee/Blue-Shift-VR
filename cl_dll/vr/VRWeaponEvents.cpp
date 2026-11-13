// Runs before the SDK's client side weapon events (ev_hldm.cpp and the like) and gives them the VR weapon's
// position instead of the player's, so the event functions themselves can stay SDK code.

#include <array>
#include <cstring>
#include <utility>

#include "hud.h"
#include "cl_util.h"
#include "event_args.h"

namespace
{
using EventFunc = void (*)(event_args_s* args);

// Blue Shift hooks 19 events, Opposing Force 28
constexpr int MAX_WRAPPED_EVENTS = 64;

// Events that are passed to the engine as they are:
// the egon takes its sound position from the muzzle and also runs without a view model (EV_EgonFire, EV_EgonStop)
const char* const s_unwrappedEvents[] = {
	"events/egon_fire.sc",
	"events/egon_stop.sc",
};

EventFunc s_eventFuncs[MAX_WRAPPED_EVENTS] = {};
int s_numWrappedEvents = 0;

void (*s_engineHookEvent)(const char* name, EventFunc pfnEvent) = nullptr;

void RunWrappedEvent(int slot, event_args_s* args)
{
	// The weapon is the view model in the hand. Without one there is nothing the event could show.
	cl_entity_s* viewModel = gEngfuncs.GetViewModel();
	if (viewModel == nullptr)
		return;

	// The event sees the weapon's origin, velocity and angles where the SDK code expects the player's.
	// A copy is changed, so the engine's event data stays as it was.
	event_args_s vrArgs = *args;
	VectorCopy(viewModel->curstate.origin, vrArgs.origin);
	VectorCopy(viewModel->curstate.velocity, vrArgs.velocity);
	VectorCopy(viewModel->curstate.angles, vrArgs.angles);
	vrArgs.angles[0] = -vrArgs.angles[0];

	s_eventFuncs[slot](&vrArgs);
}

// The engine calls an event function without saying which event it is, so every wrapped event gets a function of its own
template <int Slot>
void EventThunk(event_args_s* args)
{
	RunWrappedEvent(Slot, args);
}

template <int... Slots>
constexpr std::array<EventFunc, sizeof...(Slots)> MakeEventThunks(std::integer_sequence<int, Slots...>)
{
	return {{&EventThunk<Slots>...}};
}

constexpr std::array<EventFunc, MAX_WRAPPED_EVENTS> s_eventThunks = MakeEventThunks(std::make_integer_sequence<int, MAX_WRAPPED_EVENTS>{});

void VRHookEvent(const char* name, EventFunc pfnEvent)
{
	for (const char* unwrappedEvent : s_unwrappedEvents)
	{
		if (strcmp(name, unwrappedEvent) == 0)
		{
			s_engineHookEvent(name, pfnEvent);
			return;
		}
	}

	// The same function hooked again (for another event, or after a restart) uses its slot again
	int slot = 0;
	while (slot < s_numWrappedEvents && s_eventFuncs[slot] != pfnEvent)
		slot++;

	if (slot >= MAX_WRAPPED_EVENTS)
	{
		gEngfuncs.Con_Printf("VR: too many client events, %s gets the player's position instead of the weapon's\n", name);
		s_engineHookEvent(name, pfnEvent);
		return;
	}

	if (slot == s_numWrappedEvents)
	{
		s_eventFuncs[slot] = pfnEvent;
		s_numWrappedEvents++;
	}

	s_engineHookEvent(name, s_eventThunks[slot]);
}
} // namespace

void VRSwapWeaponEventFuncs()
{
	s_engineHookEvent = gEngfuncs.pfnHookEvent;
	gEngfuncs.pfnHookEvent = VRHookEvent;
}
