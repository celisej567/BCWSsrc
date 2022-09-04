//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef TERRAINMODMGR_SHARED_H
#define TERRAINMODMGR_SHARED_H
#ifdef _WIN32
#pragma once
#endif


#define MIN_TMOD_RADIUS		1
#define MAX_TMOD_RADIUS		1000

class CTerrainMorphParams
{
public:
	Vector	m_vecNormal;

	float	m_flDeltaZ;
	float	m_flDieTime;
	float	m_flRadius;
	float	m_flDeltaRadius;
	float	m_flStrength;
	float	m_flDeltaStrength;

	float	m_flDuration;
	float	m_flStartTime;
	float	m_flStartRadius;
	float	m_flGoalRadius;

	float	m_flFraction;
};

#endif // TERRAINMODMGR_SHARED_H
