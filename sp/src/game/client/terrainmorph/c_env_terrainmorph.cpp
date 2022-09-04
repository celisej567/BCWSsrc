

#include "cbase.h"
#include "terrainmorph/terrainmodmgr_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_TerraMorph : public C_BaseEntity
{
	DECLARE_CLASS(C_TerraMorph, C_BaseEntity);
public:
	DECLARE_CLIENTCLASS();
	C_TerraMorph();
	~C_TerraMorph();
private:
	CTerrainMorphParams m_Params;

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

IMPLEMENT_CLIENTCLASS_DT(C_TerraMorph, DT_TerrainMorphing, CTerrainMorph)
RecvPropFloat(RECVINFO(m_flDeltaZ)),
END_RECV_TABLE()

C_TerraMorph::C_TerraMorph()
{

}

C_TerraMorph::~C_TerraMorph()
{

}