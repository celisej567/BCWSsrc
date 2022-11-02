//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef WEAPON_PIPE_H
#define WEAPON_PIPE_H

#include "basebludgeonweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

#ifdef HL2MP
#error weapon_pipe.h must not be included in hl2mp. The windows compiler will use the wrong class elsewhere if it is.
#endif

#define	PIPE_RANGE	75.0f
#define	PIPE_REFIRE	0.4f

//-----------------------------------------------------------------------------
// CWeaponCrowbar
//-----------------------------------------------------------------------------

class CWeaponPipe : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS(CWeaponPipe, CBaseHLBludgeonWeapon);

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponPipe();

	float		GetRange(void)		{ return	PIPE_RANGE; }
	float		GetFireRate(void)		{ return	PIPE_REFIRE; }

	void		AddViewKick(void);
	float		GetDamageForActivity(Activity hitActivity);

	virtual int WeaponMeleeAttack1Condition(float flDot, float flDist);
	virtual	void	SecondaryAttack(void);



	// Animation event
	virtual void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);



#ifdef MAPBASE
	// Don't use backup activities
	acttable_t		*GetBackupActivityList() { return NULL; }
	int				GetBackupActivityListCount() { return 0; }
#endif

private:
	// Animation event handlers
	void HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	Activity		ChooseIntersectionPointAndActivity(trace_t& hitTrace, const Vector& mins, const Vector& maxs, CBasePlayer* pOwner);
	void CWeaponPipe::Hit(trace_t& traceHit, Activity nHitActivity, bool bIsSecondary);
	void			Swing(int bIsSecondary);

	virtual void	ItemPostFrame(void);

	float	AtkPressStart = 0;
	float	AtkPressEnd = 0;
	float	AtkDifference = 1;
};

#endif // WEAPON_CROWBAR_H