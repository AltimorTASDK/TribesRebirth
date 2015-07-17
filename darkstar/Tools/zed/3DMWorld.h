// 3DMWorld.h : header file
//

#ifndef _3DMWORLD_H_
#define _3DMWORLD_H_

#include "constructivebsp.h"
#include "THREDDoc.h"
#include "ts_Material.h"


#define MAX_3DM_VERTS	80000
#define MAX_3DM_POLYS	40000

/////////////////////////////////////////////////////////////////////////////
// C3DMWorld window
class CBspNode;


class C3DMWorld
{
	struct GrayPolygon
    {
		CString TextureName;
        int     TextureID;
		int     NumberOfPoints;
        Point2I mTextureSize;
        Point2I mTextureOffset;
        Point2F mTextureUV[MAX_POLYGON_VERTS];
		int     Points[MAX_POLYGON_VERTS];
        UInt32  volumestate;
        UInt8  mTextureScaleShift;
        bool   mApplyAmbient;

        double  constA, constB, constC, constD;
	};

public:

   static Vector< int > matMap;
   
	// our verts
	int NumberOfVerts;
	ThredPoint Verts[MAX_3DM_VERTS];
	// our polys
	int NumberOfPolys;
	GrayPolygon Polygons[MAX_3DM_POLYS];

    CTHREDDoc*      doc;
    TSMaterialList* matlist;

	// functions we need
	C3DMWorld();
	int	AddVert(ThredPoint& Vert);
    void AddPolygon(ThredPolygon& Poly, int TextureID, Point2I tshift, Point3F normal, double planeoffset);
	void DumpWorld(CString& Filename);
	void Export3DM(CBspNode* TreeTop, CString& Filename, CTHREDDoc* pDoc);
	void MakeVertsAndPolys(CBspNode* Node);
};

/////////////////////////////////////////////////////////////////////////////


#endif
