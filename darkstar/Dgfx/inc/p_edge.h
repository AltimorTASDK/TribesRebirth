//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _P_EDGE_H_
#define _P_EDGE_H_

//Includes
#include <base.h>
#include <ml.h>

#include "d_defs.h"
#include "p_txcach.h"
#include "rn_manag.h"


#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

extern "C" const Point3F*       vList[];
extern "C" const Point2F*       tList[];
extern "C" const GFXColorInfoF* cList[];
extern "C" DWORD                kList[]; // key list.
extern "C" int vertexCount;

extern "C" int __cdecl DeltaCalcPoly(void *calcStruct);
extern "C" int __cdecl DeltaCalcTri(void *calcStruct);
extern "C" int __cdecl gfx_scan_edges(int dstY);
extern "C" int __cdecl gfx_scan_edges_z(int dstY);
extern "C" int __cdecl gfx_add_edges(void *surface);
extern "C" void __cdecl GFXRenderSpan();

struct span_t {
   union {
      struct {
         unsigned int count : 11;
         unsigned int x : 11;
         unsigned int y : 10;
      } sp;
      DWORD num;
   } u;
};

extern "C" void __cdecl NewRasterizeSpans(DWORD firstSpan, DWORD lastSpan);

struct GFXPolyStore
{
   int xstart, xend;
   GFXPolyStore *myPoly;
   GFXPolyStore *nextPoly;

	DWORD spanList;
	DWORD curblock; // current block of spans (4 to a block)

	float curW;
	DWORD flags;

	// step constants
	double dwdx, dwdy, w0;
	double dudx, dudy, u0;
	double dvdx, dvdy, v0;
	double dsdx, dsdy, s0;
	BYTE*  shadeMap;
	float  shadeScale;
	BYTE*  textureMap;
	Int32  textureMapStride;
	Int32  textureMapHeight;
	
   Int32 color;
   
	GFXTextureCacheEntry *tce;
   
   // For devices that use the GFX Spans (RRedline for now) to include
   //  their own renderstate data w/ the poly...
   //
	rendererManager::renderSpanFunc drawFunc;

   DWORD         deviceRenderFlags;
   GFXColorInfoF deviceColorInfo;
   float         deviceShade;
   DWORD         __cacheAlignmentPadding;
};

struct ClippedPolyStore
{
   int xstart, xend;
   GFXPolyStore *myPoly;
   GFXPolyStore *nextPoly;
};

// IMPORTANT!!!!
// flags 1 and 2 will be zero for all clipped spans
// the clipped span fags are in the pointer in the next
// clip span  
// Ask Mark F before adding new flags.


#define FLAG_ALWAYS_BEHIND    1
#define FLAG_TRANSPARENT	   2
#define FLAG_BACKGROUND_FILL  4
#define FLAG_ALREADYDRAWN     8

extern int numPolys;
#define MAX_POLYS          3000
#define MAX_SCREEN_HEIGHT  2048
#define MAX_SPANS          30000
#define MAX_EDGES          5000
#define MAX_CLIP_SPANS     128

struct edge_t {
	edge_t	*pnext, *pprev;
	Int32	lasty;
	Int32	x;
	Int32	dx;
	Int32	leading;
	GFXPolyStore *ptrailsurf, *pleadsurf;
};
extern "C" span_t  *spanListPtr;
extern "C" edge_t  *edges;

extern "C" edge_t  *newedges[MAX_SCREEN_HEIGHT];

// Head and tail for the active edge list
extern "C" edge_t  edgehead;
extern "C" edge_t  edgetail;

// Edge used as sentinel of new edge lists
extern "C" edge_t  maxedge;

// Head/tail/sentinel/background surface of active surface stack
extern "C" GFXPolyStore  surfstack;

// pointers to next available surface and edge
extern "C" edge_t  *pavailedge;

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_P_EDGE_H_
