//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _ITRMETRICS_H_
#define _ITRMETRICS_H_

//Includes

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

#include <types.h>

struct ITRMetricsInfo
{
	struct Render {
		int leafs;
		int surfaces;
		int planes;
		int polys;
		int outsideBits;
      int currentLeaf;
		int textureCache;
		int pixels;
      int numInteriorLinks;

      int surfTransformCacheHits;
      int surfTransformCacheMisses;
		//
		void incLeafs() { leafs++; }
		void incSurfaces() { surfaces++; }
		void incPlanes(int x = 1) { planes += x; }
		void incPolys() { polys++; }
		void incTextureCache() { textureCache++; }

		void incSurfTransformCacheHits()   { surfTransformCacheHits++; }
		void incSurfTransformCacheMisses() { surfTransformCacheMisses++; }

		void reset();
	} render;
	struct Collision {
		int nodes;
		int leafs;
		int surfaces;
		int planes;
		int polys;
		//
		void incNodes() { nodes++; }
		void incLeafs() { leafs++; }
		void incSurfaces() { surfaces++; }
		void incPlanes(int x = 1) { planes += x; }
		void incPolys() { polys++; }
		void reset();
	} collision;
   
   int numRenderedInteriors;
   void reset();
};

extern ITRMetricsInfo ITRMetrics;

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_ITRMETRICS_H_
