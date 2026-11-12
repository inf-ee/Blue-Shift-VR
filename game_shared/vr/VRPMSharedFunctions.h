// VR movement functions, moved from pm_shared.cpp. Included only at the end of pm_shared.cpp,
// so they see all its file-local helpers, like the code they were moved from.

#pragma once

// No gravity under water in VR - Max Makes Mods, 2018-01-28
// Moved into separate function with improved logic - Max Makes Mods, 2018-11-18
// Disable gravity after using VR teleporter in water
// and reenable gravity after moving/jumping/crouching using default game inputs.
// (g_vrTeleportInWater is set to true in player.cpp on server dll when teleporting in water.)
bool g_vrTeleportInWater = false;
bool IsInWaterAndIsGravityDisabled()
{
	// If player uses vanilla game movements, reset g_vrTeleportInWater to false.
	if (0 != pmove->cmd.forwardmove || 0 != pmove->cmd.sidemove || 0 != pmove->cmd.upmove || (pmove->cmd.buttons & (IN_JUMP | IN_DUCK)) != 0)
	{
		g_vrTeleportInWater = false;
	}
	return (pmove->waterlevel > 0) && g_vrTeleportInWater;
}

void PM_VR_MoveAwayFromStuckEntityIfPossible(Vector end, pmtrace_t& trace)
{
	// TODO!
	trace.fraction = 1;
	trace.allsolid = 0;
	VectorCopy(end, trace.endpos);
}

physent_t* g_pLastLadder = nullptr;

physent_t* PM_Ladder2(void)
{
	if (VRGlobalGetNoclipMode())
		return nullptr;

	int grabbedLaderIndex = VRGetGrabbedLadder(pmove->player_index);

	for (int i = 0; i < pmove->nummoveent; i++)
	{
		physent_t* pe = &pmove->moveents[i];

		if (pe->model && (modtype_t)pmove->PM_GetModelType(pe->model) == mod_brush && pe->skin == CONTENTS_LADDER)
		{
			if (grabbedLaderIndex > 0)
			{
				if (grabbedLaderIndex == pe->info)
				{
					return pe;
				}
			}
			else if (VRGetLadderMode() == VR_LADDER_MODE_LEGACY_ONLY)
			{
				Vector test;
				hull_t* hull = static_cast<hull_t*>(pmove->PM_HullForBsp(pe, test));

				// Offset the test point appropriately for this hull.
				// test = origin - test
				VectorSubtract(pmove->origin, test, test);

				// Test the player's hull for intersection with this model
				if (pmove->PM_HullPointContents(hull, hull->firstclipnode, test) == CONTENTS_EMPTY)
				{
					if (g_pLastLadder == pe)
					{
						// Even the slightest head movements make the player fall off a ladder,
						// so here we will do an additional check if the player is "close enough" to stay on the ladder.

						// Get ladder center in world space
						Vector modelmins{0.f, 0.f, 0.f};
						Vector modelmaxs{0.f, 0.f, 0.f};
						pmove->PM_GetModelBounds(pe->model, modelmins, modelmaxs);
						Vector ladderCenter{0.f, 0.f, 0.f};
						VectorAdd(modelmins, modelmaxs, ladderCenter);
						VectorScale(ladderCenter, 0.5, ladderCenter);
						VectorAdd(ladderCenter, pe->origin, ladderCenter);

						// Get 8 unit direction from player to ladder
						Vector dir;
						VectorSubtract(ladderCenter, pmove->origin, dir);
						VectorNormalize(dir);
						VectorScale(dir, 8.f, dir);

						// Add dir onto the test point and check again
						Vector test2;
						VectorAdd(test, dir, test2);
						if (pmove->PM_HullPointContents(hull, hull->firstclipnode, test2) != CONTENTS_EMPTY)
						{
							return pe;
						}

						// Subtract dir from the test point and check again
						// VectorAdd(test, dir, test2); - orignal
						VectorSubtract(test, dir, test2);
						if (pmove->PM_HullPointContents(hull, hull->firstclipnode, test2) != CONTENTS_EMPTY)
						{
							return pe;
						}
					}
					continue;
				}
				else
				{
					g_pLastLadder = pe;
					return pe;
				}
			}
		}
	}

	g_pLastLadder = nullptr;
	return nullptr;
}

bool PM_TryUnstuck()
{
	// Trying to determine if a player can move while being stuck (move away from wall) - Max Makes Mods, 2018-04-01
	// Updated and improved - Max Makes Mods, 2020-03-01

	// the maximum distance we try to move a stuck player
	constexpr const float VR_UNSTUCK_DISTANCE = 32.f;

	// Don't unstuck dead players
	if (0 != pmove->dead || pmove->deadflag != DEAD_NO)
		return false;

	// we aren't stuck (possible retry ducking)
	if (pmove->PM_TestPlayerPosition(pmove->origin, nullptr) < 0)
		return true;

	// check if it's just a step
	float oldz = pmove->origin[2];
	pmove->origin[2] += pmove->movevars->stepsize;
	if (pmove->PM_TestPlayerPosition(pmove->origin, nullptr) < 0)
		return true;

	// bruteforce our way out of this!
	pmove->origin[2] = oldz;
	int stuckent = pmove->PM_TestPlayerPosition(pmove->origin, nullptr);
	bool foundend = false;
	bool foundendinsideent = false;
	Vector foundendoffset;
	for (int i = 0; !foundend && i <= VR_UNSTUCK_DISTANCE; i++)
	{
		for (int x = -1; x <= 1; x++)
		{
			for (int y = -1; y <= 1; y++)
			{
				// go positive in z direction first (unstucking upwards is always better than downwards)
				for (int z = 1; z >= -1; z--)
				{
					Vector testend;
					Vector offset;
					offset[0] = x * i;
					offset[1] = y * i;
					offset[2] = z * i;
					VectorAdd(pmove->origin, offset, testend);
					if (pmove->PM_TestPlayerPosition(testend, nullptr) == -1)
					{
						if (stuckent > 0 && VRGlobalIsPointInsideEnt(testend, pmove->physents[stuckent].info) && (!foundendinsideent || Length(offset) < Length(foundendoffset)))
						{
							VectorCopy(offset, foundendoffset);
							foundendinsideent = true;
						}
						else if (!foundendinsideent && (!foundend || Length(offset) < Length(foundendoffset)))
						{
							VectorCopy(offset, foundendoffset);
						}
						foundend = true;
					}
				}
			}
		}
	}

	if (foundend)
	{
		// successfully unstucked
		VectorAdd(pmove->origin, foundendoffset, pmove->origin);
		return true;
	}

	// still stuck and already ducking, can't unstuck :(
	if ((pmove->flags & FL_DUCKING) != 0 || pmove->usehull == 1)
		return false;

	// try ducking
	pmove->flags |= FL_DUCKING;
	pmove->usehull = 1;
	for (int i = 0; i < 3; i++)
	{
		pmove->origin[i] -= (pmove->player_mins[1][i] - pmove->player_mins[0][i]);
	}

	// try again ducking
	return PM_TryUnstuck();
}

/*
====================
PM_YesClip
Moved from PM_Move() switch statement for MOVETYPE_WALK - Max Makes Mods, 2018-04-01
====================
*/
void PM_YesClip(physent_t* pLadder)
{
	// No gravity in water - Max Makes Mods, 2018-02-11
	if (IsInWaterAndIsGravityDisabled() || (pmove->flags & FL_BARNACLED) != 0)
	{
		pmove->velocity[2] = std::fmax(0.f, pmove->velocity[2]);
	}

	if (!PM_InWater() && (pmove->flags & FL_BARNACLED) == 0)
	{
		PM_AddCorrectGravity();
	}

	// If we are leaping out of the water, just update the counters.
	if (0 != pmove->waterjumptime)
	{
		PM_WaterJump();
		PM_FlyMove();

		// Make sure waterlevel is set correctly
		PM_CheckWater();
		return;
	}

	// If we are swimming in the water, see if we are nudging against a place we can jump up out
	//  of, and, if so, start out jump.  Otherwise, if we are not moving up, then reset jump timer to 0
	if (pmove->waterlevel > 1)
	{
		if (pmove->waterlevel == 2)
		{
			PM_CheckWaterJump();
		}

		// If we are falling again, then we must not trying to jump out of water any more.
		if (pmove->velocity[2] < 0 && 0 != pmove->waterjumptime)
		{
			pmove->waterjumptime = 0;
		}

		// Perform regular water movement
		PM_WaterMove();

		VectorSubtract(pmove->velocity, pmove->basevelocity, pmove->velocity);

		// Get a final position
		PM_CatagorizePosition();
	}
	else

	// Not underwater
	{
		// Was jump button pressed?
		if ((pmove->cmd.buttons & IN_JUMP) != 0)
		{
			if (!pLadder)
			{
				PM_Jump();
			}
		}
		else
		{
			pmove->oldbuttons &= ~IN_JUMP;
		}

		// Fricion is handled before we add in any base velocity. That way, if we are on a conveyor,
		//  we don't slow when standing still, relative to the conveyor.
		if (pmove->onground != -1) // BSVR: on ground (upstream IsOnGround())
		{
			pmove->velocity[2] = 0.0;
			PM_Friction();
		}

		// Make sure velocity is valid.
		PM_CheckVelocity();

		// Are we on ground now
		if (pmove->onground != -1) // BSVR: on ground (upstream IsOnGround())
		{
			PM_WalkMove();
		}
		else
		{
			PM_AirMove(); // Take into account movement when in air.
		}

		// Set final flags.
		PM_CatagorizePosition();

		// Now pull the base velocity back out.
		// Base velocity is set if you are on a moving object, like
		//  a conveyor (or maybe another monster?)
		VectorSubtract(pmove->velocity, pmove->basevelocity, pmove->velocity);

		// Make sure velocity is valid.
		PM_CheckVelocity();

		// Add any remaining gravitational component.
		if (!PM_InWater())
		{
			PM_FixupGravityVelocity();
		}

		// If we are on ground, no downward velocity.
		if (pmove->onground != -1) // BSVR: on ground (upstream IsOnGround())
		{
			pmove->velocity[2] = 0;
		}

		// See if we landed on the ground with enough force to play
		//  a landing sound.
		PM_CheckFalling();
	}

	if (IsInWaterAndIsGravityDisabled() || (pmove->flags & FL_BARNACLED) != 0)
	{
		pmove->velocity[2] = std::fmax(0.f, pmove->velocity[2]);
	}

	// Did we enter or leave the water?
	PM_PlayWaterSounds();
}

void PM_HandleMovement(physent_t* pLadder)
{
	// Handle movement
	switch (pmove->movetype)
	{
	default:
		pmove->Con_DPrintf("Bogus pmove player movetype %i on (%i) 0=cl 1=sv\n", pmove->movetype, pmove->server);
		break;

	case MOVETYPE_NONE:
		break;

	case MOVETYPE_TOSS:
	case MOVETYPE_BOUNCE:
		PM_Physics_Toss();
		break;

	case MOVETYPE_FLY:

		PM_CheckWater();

		// Was jump button pressed?
		// If so, set velocity to 270 away from ladder.  This is currently wrong.
		// Also, set MOVE_TYPE to walk, too.
		if ((pmove->cmd.buttons & IN_JUMP) != 0)
		{
			if (!pLadder)
			{
				PM_Jump();
			}
		}
		else
		{
			pmove->oldbuttons &= ~IN_JUMP;
		}

		// Perform the move accounting for any base velocity.
		VectorAdd(pmove->velocity, pmove->basevelocity, pmove->velocity);
		PM_FlyMove();
		VectorSubtract(pmove->velocity, pmove->basevelocity, pmove->velocity);
		break;

	case MOVETYPE_NOCLIP: // In VR player movetype is always NOCLIP so doors, elevators etc don't crush us
	case MOVETYPE_WALK:
		if (VRGlobalGetNoclipMode())
		{
			PM_NoClip();
		}
		else
		{
			PM_YesClip(pLadder); // Moved all the walking code in this switch-statement to its own method
		}
		break;
	}
}

// Added, so server.dll can access bsp models (see UTIL_GetBSPModel in util.cpp) - Max Makes Mods, 2018-01-21
struct playermove_s* PM_GetPlayerMove(void)
{
	return pmove;
}
