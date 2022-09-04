
#include "terrainmorph.h"


public void ApplyTerrainMorph(ITerrainMod* pMod)
{
#if 1

	int nVertCount = CDispCollTree().GetSize();
	for (int iVert = 0; iVert < nVertCount; ++iVert)
	{
		Vector pos;
		pMod->ApplyMod(CDispCollTree().m_aVerts[iVert] + Vector(0, 0, 10), CDispCollTree().m_aVerts[iVert]);
		//pMod->ApplyMod(m_aVerts[iVert], m_aVerts[iVert]);
	}

	// Setup/create the leaf nodes first so the recursion can use this data to stop.
	CDispCollTree().AABBTree_CreateLeafs();

	// Generate bounding boxes.
	//AABBTree_GenerateBoxes();

	// Create the bounding box of the displacement surface + the base face.
	CDispCollTree().AABBTree_CalcBounds();
#endif

}