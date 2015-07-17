#ifndef _GFXMETRICS_H_
#define _GFXMETRICS_H_

class GFXMetricsInfo
{
   public:
	int emittedPolys;		// number of polygons passed in to emitPoly
	int renderedPolys;		// number of polys not totally occluded
	int numLightMaps;		// total # of lightMaps allocated.
	int lightMapsInUse;		// number of lightMaps currently in use.
	int recachedSurfaces;	// number of surfaces recached.
	int backfacedPolys;		// number of polygons that fail the backface test
	int numSharedEdges;		// number of edges shared.
	int numSpansUsed;		// number of spans allocated for the frame
	int numEdgesUsed;		// number of edges in polygons.
	int numPixelsRendered;
   int textureSpaceUsed;
   int textureBytesDownloaded;
   int numTexturesDownloaded;
   int numCreatedTextures;
   int numReleasedTextures;
   int numTMU0Wraps;
   int numTMU1Wraps;
   int numPaletteDLs;

	void incEmittedPolys()          { emittedPolys++; }
	void incBackfacedPolys()        { backfacedPolys++; }
	void incRenderedPolys()         { renderedPolys++; }
	void incRecachedSurfaces()      { recachedSurfaces++; }
	void incSharedEdges()           { numSharedEdges++; }
	void incUsedEdges()             { numEdgesUsed++; }
   void incNumTexturesDownloaded() { numTexturesDownloaded++; }
   void incCreatedTextures()       { numCreatedTextures++; }
   void incReleasedTextures()      { numReleasedTextures++; }
   
   void addToTextureSU(const UInt32 in_su) { textureSpaceUsed += in_su; }
   void addToTextureBytesDL(const UInt32 in_bd) { textureBytesDownloaded += in_bd; }
   
	void reset() { numPixelsRendered =
                          numEdgesUsed      =
                          numSpansUsed      =
                          numSharedEdges    =
                          backfacedPolys    =
                          emittedPolys      =
                          renderedPolys     =
                          recachedSurfaces  = 0;
                          numTexturesDownloaded =
                          numReleasedTextures   =
                          numCreatedTextures    =
                          numPaletteDLs         = 0; }
};

class GFXPrefInfo
{
   public:
   bool useBanshee;
   bool useLowRes3D;
   bool waitForVSync;

   GFXPrefInfo() {
      useBanshee   = false;
      useLowRes3D  = false;
      waitForVSync = true;
   }
};

extern GFXMetricsInfo GFXMetrics;
extern GFXPrefInfo    GFXPrefs;

#endif