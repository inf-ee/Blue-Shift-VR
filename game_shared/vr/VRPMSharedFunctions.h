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

// Hooks called from the SDK functions in pm_shared.cpp

// Replaces the SDK PM_LadderMove (early return there): Half-Life: VR ladder movement
void PM_VRLadderMove(physent_t* pLadder)
{
	if (VRGlobalGetNoclipMode())
	{
		return;
	}

	int ladderMode = VRGetLadderMode();
	if (ladderMode == VR_LADDER_MODE_IMMERSIVE_ONLY)
	{
		return;
	}

	pmove->gravity = 0;
	pmove->movetype = MOVETYPE_FLY;

	Vector modelmins{0.f, 0.f, 0.f};
	Vector modelmaxs{0.f, 0.f, 0.f};
	pmove->PM_GetModelBounds(pLadder->model, modelmins, modelmaxs);

	Vector ladderCenter{0.f, 0.f, 0.f};
	VectorAdd(modelmins, modelmaxs, ladderCenter);
	VectorScale(ladderCenter, 0.5, ladderCenter);
	VectorAdd(ladderCenter, pLadder->origin, ladderCenter);

	bool onFloor = ((pmove->flags & FL_ONGROUND) != 0) || pmove->onground >= 0;

	if (!onFloor)
	{
		Vector floor{0.f, 0.f, 0.f};
		VectorCopy(pmove->origin, floor);
		floor[2] += pmove->player_mins[pmove->usehull][2] - 1;
		onFloor = pmove->PM_PointContents(floor, nullptr) == CONTENTS_SOLID;
	}

	trace_t trace{0};
	float result = pmove->PM_TraceModel(pLadder, pmove->origin, ladderCenter, &trace);
	if (result >= 0.f && result < 1.f && trace.fraction >= 0.f && trace.fraction < 1.f && !IS_NAN(trace.plane.normal[0]) && !IS_NAN(trace.plane.normal[1]) && !IS_NAN(trace.plane.normal[2]))
	{
		if ((pmove->cmd.buttons & IN_JUMP) != 0 && ladderMode == VR_LADDER_MODE_LEGACY_ONLY)
		{
			pmove->movetype = MOVETYPE_NOCLIP; // pmove->movetype = MOVETYPE_WALK; Restore to MOVETYPE_NOCLIP in VR
			VectorScale(trace.plane.normal, 270, pmove->velocity);
		}
		else
		{
			float forwardSpeed = DEFAULT_MOVEMENT_SPEED;
			float backSpeed = DEFAULT_MOVEMENT_SPEED;
			float sideSpeed = DEFAULT_MOVEMENT_SPEED;
			float upSpeed = DEFAULT_MOVEMENT_SPEED;
			float maxClimbSpeed = DEFAULT_MAX_CLIMB_SPEED;
			float maxSidewaysSpeed = DEFAULT_MAX_SIDEWAYS_SPEED;
			VRGetCVARSpeeds(forwardSpeed, backSpeed, sideSpeed, upSpeed);
			VRGetMaxClimbSpeed(maxClimbSpeed, maxSidewaysSpeed);
			if (forwardSpeed <= 1.f)
				forwardSpeed = DEFAULT_MOVEMENT_SPEED;
			if (backSpeed <= 1.f)
				backSpeed = DEFAULT_MOVEMENT_SPEED;
			if (sideSpeed <= 1.f)
				sideSpeed = DEFAULT_MOVEMENT_SPEED;
			if (upSpeed <= 1.f)
				upSpeed = DEFAULT_MOVEMENT_SPEED;
			if (maxClimbSpeed <= 1.f)
				maxClimbSpeed = DEFAULT_MAX_CLIMB_SPEED;
			if (maxSidewaysSpeed <= 1.f)
				maxSidewaysSpeed = DEFAULT_MAX_SIDEWAYS_SPEED;

			float up;
			if (pmove->cmd.forwardmove > 0.f)
			{
				up = (pmove->cmd.forwardmove / forwardSpeed) * maxClimbSpeed;
			}
			else
			{
				up = (pmove->cmd.forwardmove / backSpeed) * maxClimbSpeed;
			}
			up += (pmove->cmd.upmove / upSpeed) * maxClimbSpeed;

			float right = (pmove->cmd.sidemove / sideSpeed) * maxSidewaysSpeed;

			if (up != 0.f || right != 0.f)
			{
				Vector ladderSize;
				VectorSubtract(pLadder->maxs, pLadder->mins, ladderSize);

				// get direction vectors for climbing on this ladder
				Vector v_up{0.f, 0.f, 0.f};
				Vector v_right{0.f, 0.f, 0.f};

				// ladder is horizontal
				if (fabs(trace.plane.normal[2]) > 0.995f)
				{
					// guesstimate vectors in horizontal ladder plane
					if (ladderSize[0] > ladderSize[1])
					{
						v_up[0] = 1.f;
						v_right[1] = -1.f;
					}
					else
					{
						v_up[1] = 1.f;
						v_right[0] = 1.f;
					}
				}
				else
				{
					// calculate vectors in vertical ladder plane
					CrossProduct(Vector{0.f, 0.f, 1.f}, trace.plane.normal, v_right);
					CrossProduct(trace.plane.normal, v_right, v_up);
				}

				// Calculate velocity
				Vector velocity;
				VectorScale(v_up, up, velocity);
				VectorMA(velocity, right, v_right, velocity);

				if (onFloor && velocity[2] < 0.f)
				{
					VectorScale(trace.plane.normal, Length(velocity), pmove->velocity);
				}
				else
				{
					VectorCopy(velocity, pmove->velocity);
				}
			}
			else
			{
				VectorClear(pmove->velocity);
			}
		}
	}

	PM_CheckVelocity();
}

// End of PM_PlayerMove, replaces the SDK movement switch (early return there): movement with auto-duck, see PM_HandleMovement
void PM_VRMove(physent_t* pLadder)
{
	// Always calculate movement (on land) as if ducking, then afterwards unduck if possible or stay ducked if not possible (this essentially makes players autoduck if necessary)
	// In water we don't do this, as it messes with waterjump
	bool tryAutoDuck = VRIsAutoDuckingEnabled(pmove->player_index) && pmove->usehull == 0 && (pmove->flags & FL_DUCKING) == 0 && nullptr == pLadder && pmove->waterlevel == 0;

	if (tryAutoDuck && pmove->onground != -1)
	{
		// same logic as in PM_CategorizePosition()
		Vector point;
		VectorCopy(pmove->origin, point);
		point[2] -= 2.f;
		auto tr = pmove->PM_PlayerTrace(pmove->origin, point, PM_NORMAL, -1);
		// don't auto crouch if surface isn't horizontal
		if (tr.fraction >= 1.f || tr.plane.normal[2] < 1.f)
		{
			tryAutoDuck = false;
		}
	}
	if (tryAutoDuck)
	{
		// Backup original origin and velocity (velocity gets changed to add in gravity!)
		Vector original_origin;
		Vector original_velocity;
		VectorCopy(pmove->origin, original_origin);
		VectorCopy(pmove->velocity, original_velocity);

		// Move standing up
		PM_HandleMovement(pLadder);

		// Backup standmove origin
		Vector standmove_origin;
		Vector standmove_velocity;
		VectorCopy(pmove->origin, standmove_origin);
		VectorCopy(pmove->velocity, standmove_velocity);

		// Restore original origin and velocity
		VectorCopy(original_origin, pmove->origin);
		VectorCopy(original_velocity, pmove->velocity);
		PM_CatagorizePosition();

		// Set crouching
		pmove->usehull = 1;
		pmove->flags |= FL_DUCKING;
		pmove->origin[2] = pmove->origin[2] + pmove->player_mins[0][2] - pmove->player_mins[1][2];

		// Make sure we stay onground when switching to crouching
		if (pmove->onground != -1) // upstream: IsOnGround()
		{
			PM_CatagorizePosition();
			int tries = 0;
			while (pmove->onground == -1 && tries < 20) // upstream: !IsOnGround()
			{
				pmove->origin[2] -= 0.1f;
				PM_CatagorizePosition();
				tries++;
			}
			// if we can't get properly back on ground, smth is wrong, and we cannot reliably do auto-crouch (steep ground or whatever), cancel and use standing movement
			if (pmove->onground == -1) // upstream: !IsOnGround()
			{
				VectorCopy(standmove_origin, pmove->origin);
				VectorCopy(standmove_velocity, pmove->velocity);
				pmove->usehull = 0;
				pmove->flags &= ~FL_DUCKING;
				PM_CatagorizePosition();
				return;
			}
		}

		// Move crouching
		PM_HandleMovement(pLadder);

		// Backup duckmove origin
		Vector duckmove_origin;
		VectorCopy(pmove->origin, duckmove_origin);

		// Get distance moved ducking and standing
		Vector standmove_dist;
		Vector duckmove_dist;
		VectorSubtract(standmove_origin, original_origin, standmove_dist);
		VectorSubtract(duckmove_origin, original_origin, duckmove_dist);

		// Restore standing
		pmove->usehull = 0;
		pmove->flags &= ~FL_DUCKING;

		// Use ducking if we walked 50% further ducking than standing (happens on entering something where we must duck)
		if (duckmove_dist.Length2D() > standmove_dist.Length2D() * 1.5f)
		{
			// check if we can actually stand at the ducking origin!
			Vector standing_ducking_origin;
			VectorCopy(duckmove_origin, standing_ducking_origin);
			standing_ducking_origin[2] = standing_ducking_origin[2] + pmove->player_mins[1][2] - pmove->player_mins[0][2];
			if (pmove->PM_TestPlayerPosition(standing_ducking_origin, nullptr) < 0)
			{
				// use standing origin, if we can actually stand at the ducking origin!
				VectorCopy(standmove_origin, pmove->origin);
				VectorCopy(standmove_velocity, pmove->velocity);
			}
			else
			{
				// ducking origin is further away, and we can't stand here, really use ducking origin and stay ducked
				pmove->usehull = 1;
				pmove->flags |= FL_DUCKING;
				VectorCopy(duckmove_origin, pmove->origin);
			}
		}
		else // Use standing otherwise
		{
			VectorCopy(standmove_origin, pmove->origin);
			VectorCopy(standmove_velocity, pmove->velocity);
		}
	}
	else
	{
		PM_HandleMovement(pLadder);
	}

	PM_CatagorizePosition();
}

// PM_PlayerMove: VR unstuck handling, returns true when the player can't move this frame
bool PM_VRCheckStuck(qboolean server)
{
	if (!VRGlobalGetNoclipMode() && pmove->movetype != MOVETYPE_NONE)
	{
		bool dontUnstuck = false;
		int stuckent = pmove->PM_TestPlayerPosition(pmove->origin, nullptr);
		if (stuckent > 0)
		{
			// Tell server dll that we got stuck in an entity - server dll can then determine if this entity should kill us or not
			dontUnstuck = VRNotifyStuckEnt(pmove->player_index, pmove->physents[stuckent].info);
		}
		if (PM_CheckStuck() || stuckent >= 0)
		{
			// When we're stuck, move away if our move direction is going away from whatever we're stuck on
			// (Only unstuck us on server, to not interfere with dontUnstuck, which is only set by server)
			if ((0 != server || 0 != pmove->server) && !dontUnstuck)
			{
				if (!PM_TryUnstuck())
				{
					return true;
				}
			}
			else
			{
				return true;
			}
		}
	}
	return false;
}

// PM_WaterMove: sinking and swimming up/down with the jump and duck buttons
void PM_VRWaterWishVelocity(Vector& wishvel)
{
	if (0 == pmove->cmd.forwardmove && 0 == pmove->cmd.sidemove && 0 == pmove->cmd.upmove && (pmove->oldbuttons & (IN_JUMP | IN_DUCK)) == 0)
	{
		if (IsInWaterAndIsGravityDisabled())
		{
			wishvel[2] = 0;
		}
		else
		{
			wishvel[2] -= 60; // drift towards bottom
		}
	}
	else
	{
		// Go straight up by upmove amount.
		wishvel[2] += pmove->cmd.upmove;
		if ((pmove->oldbuttons & IN_JUMP) != 0)
		{
			wishvel[2] += 320.f;
		}
		if ((pmove->oldbuttons & IN_DUCK) != 0)
		{
			wishvel[2] -= 320.f;
		}
	}
}

// End of PM_Duck: slower movement and the duck hull while ducking
void PM_VRDuckSpeedAndHull()
{
	if ((pmove->flags & FL_DUCKING) != 0)
	{
		pmove->cmd.forwardmove *= 0.333;
		pmove->cmd.sidemove *= 0.333;
		pmove->cmd.upmove *= 0.333;
		pmove->usehull = 1;
	}
	else
	{
		pmove->usehull = 0;
	}
}
