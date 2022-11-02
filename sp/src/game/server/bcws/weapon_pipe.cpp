//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Crowbar - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "soundent.h"
#include "basebludgeonweapon.h"
#include "vstdlib/random.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "weapon_pipe.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar    sk_plr_dmg_pipe("sk_plr_dmg_pipe", "10");
ConVar    sk_npc_dmg_pipe("sk_npc_dmg_pipe", "10");


ConVar    sk_plr_dmg_pipe_swing("sk_plr_dmg_pipe_swing", "20");
ConVar		sk_plr_dmg_pipe_mult("sk_plr_dmg_pipe_mult","2");

ConVar    sk_npc_dmg_pipe_swing("sk_npc_dmg_pipe_swing", "20");

//-----------------------------------------------------------------------------
// CWeaponCrowbar
//-----------------------------------------------------------------------------

static const Vector g_bludgeonMins(-16, -16, -16);
static const Vector g_bludgeonMaxs(16, 16, 16);


IMPLEMENT_SERVERCLASS_ST(CWeaponPipe, DT_WeaponPipe)
END_SEND_TABLE()

#ifndef HL2MP
LINK_ENTITY_TO_CLASS(weapon_pipe, CWeaponPipe);
PRECACHE_WEAPON_REGISTER(weapon_pipe);
#endif

acttable_t CWeaponPipe::m_acttable[] =
{
	{ ACT_MELEE_ATTACK1, ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE, ACT_IDLE_ANGRY_MELEE, false },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_MELEE, false },
#if EXPANDED_HL2_WEAPON_ACTIVITIES
	{ ACT_RUN, ACT_RUN_MELEE, false },
	{ ACT_WALK, ACT_WALK_MELEE, false },

	{ ACT_ARM, ACT_ARM_MELEE, false },
	{ ACT_DISARM, ACT_DISARM_MELEE, false },
#endif

#ifdef MAPBASE
	// HL2:DM activities (for third-person animations in SP)
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_MELEE, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_MELEE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_MELEE, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_MELEE, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_MELEE, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_MELEE, false },
#if EXPANDED_HL2DM_ACTIVITIES
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK2, ACT_HL2MP_GESTURE_RANGE_ATTACK2_MELEE, false },
	{ ACT_HL2MP_WALK, ACT_HL2MP_WALK_MELEE, false },
#endif
#endif
};

IMPLEMENT_ACTTABLE(CWeaponPipe);

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponPipe::CWeaponPipe(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponPipe::GetDamageForActivity(Activity hitActivity)
{
	if ((GetOwner() != NULL) && (GetOwner()->IsPlayer()))
		if (hitActivity == ACT_VM_HITCENTER2)
			return sk_plr_dmg_pipe_swing.GetFloat() * (AtkDifference * sk_plr_dmg_pipe_mult.GetFloat());
		else
			return sk_plr_dmg_pipe.GetFloat();

	if (hitActivity == ACT_VM_HITCENTER2)
		return sk_npc_dmg_pipe_swing.GetFloat();
	else
		return sk_npc_dmg_pipe.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponPipe::AddViewKick(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat(1.0f, 2.0f);
	punchAng.y = random->RandomFloat(-2.0f, -1.0f);
	punchAng.z = 0.0f;

	pPlayer->ViewPunch(punchAng);
}


//-----------------------------------------------------------------------------
// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
//-----------------------------------------------------------------------------
ConVar sk_pipe_lead_time("sk_pipe_lead_time", "0.9");

int CWeaponPipe::WeaponMeleeAttack1Condition(float flDot, float flDist)
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
	CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecVelocity;
	vecVelocity = pEnemy->GetSmoothedVelocity();

	// Project where the enemy will be in a little while
	float dt = sk_pipe_lead_time.GetFloat();
	dt += random->RandomFloat(-0.3f, 0.2f);
	if (dt < 0.0f)
		dt = 0.0f;

	Vector vecExtrapolatedPos;
	VectorMA(pEnemy->WorldSpaceCenter(), dt, vecVelocity, vecExtrapolatedPos);

	Vector vecDelta;
	VectorSubtract(vecExtrapolatedPos, pNPC->WorldSpaceCenter(), vecDelta);

	if (fabs(vecDelta.z) > 70)
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	Vector vecForward = pNPC->BodyDirection2D();
	vecDelta.z = 0.0f;
	float flExtrapolatedDist = Vector2DNormalize(vecDelta.AsVector2D());
	if ((flDist > 64) && (flExtrapolatedDist > 64))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	float flExtrapolatedDot = DotProduct2D(vecDelta.AsVector2D(), vecForward.AsVector2D());
	if ((flDot < 0.7) && (flExtrapolatedDot < 0.7))
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}


//-----------------------------------------------------------------------------
// Animation event handlers
//-----------------------------------------------------------------------------
void CWeaponPipe::HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
		// Trace up or down based on where the enemy is...
		// But only if we're basically facing that direction
		Vector vecDirection;
		AngleVectors(GetAbsAngles(), &vecDirection);

		CBaseEntity* pEnemy = pOperator->MyNPCPointer() ? pOperator->MyNPCPointer()->GetEnemy() : NULL;
		if (pEnemy)
		{
			Vector vecDelta;
			VectorSubtract(pEnemy->WorldSpaceCenter(), pOperator->Weapon_ShootPosition(), vecDelta);
			VectorNormalize(vecDelta);

			Vector2D vecDelta2D = vecDelta.AsVector2D();
			Vector2DNormalize(vecDelta2D);
			if (DotProduct2D(vecDelta2D, vecDirection.AsVector2D()) > 0.8f)
			{
				vecDirection = vecDelta;
			}
		}

		Vector vecEnd;
		VectorMA(pOperator->Weapon_ShootPosition(), 50, vecDirection, vecEnd);
		CBaseEntity* pHurt = pOperator->CheckTraceHullAttack(pOperator->Weapon_ShootPosition(), vecEnd,
			Vector(-16, -16, -16), Vector(36, 36, 36), sk_npc_dmg_pipe.GetFloat(), DMG_CLUB, 0.75);

		// did I hit someone?
		if (pHurt)
		{
			// play sound
			WeaponSound(MELEE_HIT);

			// Fake a trace impact, so the effects work out like a player's crowbaw
			trace_t traceHit;
			UTIL_TraceLine(pOperator->Weapon_ShootPosition(), pHurt->GetAbsOrigin(), MASK_SHOT_HULL, pOperator, COLLISION_GROUP_NONE, &traceHit);
			ImpactEffect(traceHit);
		}
		else
		{
			WeaponSound(MELEE_MISS);
		}


}


//-----------------------------------------------------------------------------
// Animation event
//-----------------------------------------------------------------------------
void CWeaponPipe::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_MELEE_HIT:
		HandleAnimEventMeleeHit(pEvent, pOperator);
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

void CWeaponPipe::SecondaryAttack()
{
	//SendWeaponAnim(ACT_VM_HITCENTER2);

	//Setup our next attack times
	//m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	//m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}

Activity CWeaponPipe::ChooseIntersectionPointAndActivity(trace_t& hitTrace, const Vector& mins, const Vector& maxs, CBasePlayer* pOwner)
{
	int			i, j, k;
	float		distance;
	const float* minmaxs[2] = { mins.Base(), maxs.Base() };
	trace_t		tmpTrace;
	Vector		vecHullEnd = hitTrace.endpos;
	Vector		vecEnd;

	distance = 1e6f;
	Vector vecSrc = hitTrace.startpos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	UTIL_TraceLine(vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
	if (tmpTrace.fraction == 1.0)
	{
		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 2; j++)
			{
				for (k = 0; k < 2; k++)
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					UTIL_TraceLine(vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
					if (tmpTrace.fraction < 1.0)
					{
						float thisDistance = (tmpTrace.endpos - vecSrc).Length();
						if (thisDistance < distance)
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		hitTrace = tmpTrace;
	}


	return ACT_VM_HITCENTER;
}

//------------------------------------------------------------------------------
// Purpose: Implement impact function
//------------------------------------------------------------------------------
void CWeaponPipe::Hit(trace_t& traceHit, Activity nHitActivity, bool bIsSecondary)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	//Do view kick
	AddViewKick();

	//Make sound for the AI
	CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, traceHit.endpos, 400, 0.2f, pPlayer);

	// This isn't great, but it's something for when the crowbar hits.
	//pPlayer->RumbleEffect(RUMBLE_AR2, 0, RUMBLE_FLAG_RESTART);

	CBaseEntity* pHitEntity = traceHit.m_pEnt;

	//Apply damage to a hit target
	if (pHitEntity != NULL)
	{
		Vector hitDirection;
		pPlayer->EyeVectors(&hitDirection, NULL, NULL);
		VectorNormalize(hitDirection);

		CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_CLUB);

		if (pPlayer && pHitEntity->IsNPC())
		{
			// If bonking an NPC, adjust damage.
			info.AdjustPlayerDamageInflictedForSkillLevel();
		}

		CalculateMeleeDamageForce(&info, hitDirection, traceHit.endpos);

		pHitEntity->DispatchTraceAttack(info, hitDirection, &traceHit);
		ApplyMultiDamage();

		// Now hit all triggers along the ray that... 
		TraceAttackToTriggers(info, traceHit.startpos, traceHit.endpos, hitDirection);

		if (ToBaseCombatCharacter(pHitEntity))
		{
			gamestats->Event_WeaponHit(pPlayer, !bIsSecondary, GetClassname(), info);
		}
	}

	// Apply an impact effect
	ImpactEffect(traceHit);


}

//------------------------------------------------------------------------------
// Purpose : Starts the swing of the weapon and determines the animation
// Input   : bIsSecondary - is this a secondary attack?
//------------------------------------------------------------------------------
void CWeaponPipe::Swing(int bIsSecondary)
{
	trace_t traceHit;

	// Try a ray
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

	Vector swingEnd = swingStart + forward * GetRange();
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
	Activity nHitActivity = !bIsSecondary ? ACT_VM_HITCENTER : ACT_VM_HITCENTER2;

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_CLUB);
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward);
	TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);

	if (traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.732f * 16;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
		if (traceHit.fraction < 1.0 && traceHit.m_pEnt)
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize(vecToTarget);

			float dot = vecToTarget.Dot(forward);

			// YWB:  Make sure they are sort of facing the guy at least...
			if (dot < 0.70721f)
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				nHitActivity = ChooseIntersectionPointAndActivity(traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner);
			}
		}
	}

	if (!bIsSecondary)
	{
		m_iPrimaryAttacks++;
	}
	else
	{
		m_iSecondaryAttacks++;
	}

	gamestats->Event_WeaponFired(pOwner, !bIsSecondary, GetClassname());

	// -------------------------
	//	Miss
	// -------------------------
	if (traceHit.fraction == 1.0f)
	{
		nHitActivity = bIsSecondary ? ACT_VM_MISSCENTER2 : ACT_VM_MISSCENTER;

		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetRange();

#ifdef MAPBASE
		// Sound has been moved here since we're using the other melee sounds now
		WeaponSound(SINGLE);
#endif

		// See if we happened to hit water
		//ImpactWater(swingStart, testEnd);
	}
	else
	{
#ifdef MAPBASE
		// Other melee sounds
		if (traceHit.m_pEnt && traceHit.m_pEnt->IsWorld())
			WeaponSound(MELEE_HIT_WORLD);
		else if (traceHit.m_pEnt && !traceHit.m_pEnt->PassesDamageFilter(triggerInfo))
			WeaponSound(MELEE_MISS);
		else
			WeaponSound(MELEE_HIT);
#endif

		Hit(traceHit, nHitActivity, bIsSecondary ? true : false);
	}

	// Send the anim
	if(!bIsSecondary)
	SendWeaponAnim(nHitActivity);

	//Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

#ifndef MAPBASE
	//Play swing sound
	WeaponSound(SINGLE);
#endif
}

void CWeaponPipe::ItemPostFrame()
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	BaseClass::ItemPostFrame();

	if (!pOwner)
		return;

	if (m_flNextSecondaryAttack > gpGlobals->curtime)
		return;


	//On Mouse2 pressed
	if (pOwner->m_afButtonPressed & IN_ATTACK2)
	{
		SendWeaponAnim(ACT_VM_HITCENTER2_START);
		AtkPressStart = gpGlobals->curtime;
		m_flNextPrimaryAttack = gpGlobals->curtime + 999999999;
	}

	//On Mouse2 released
	if (pOwner->m_afButtonReleased & IN_ATTACK2)
	{
		AtkPressEnd = gpGlobals->curtime;
		if (AtkPressEnd - AtkPressStart < 0.6)
		{
			SendWeaponAnim(ACT_VM_IDLE);
			return;
		}
		else if (AtkPressEnd - AtkPressStart > 3)
			AtkDifference = 3;
		else
			AtkDifference = AtkPressEnd - AtkPressStart;



		SendWeaponAnim(ACT_VM_HITCENTER2_END);
		Swing(true);
		//Msg("Delay is: %f\n", AtkDifference);
	}

}

