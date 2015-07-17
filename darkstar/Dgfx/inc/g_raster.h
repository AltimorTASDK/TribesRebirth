//================================================================
//   
// $Workfile:   g_raster.h  $
// $Version$
// $Revision:   1.5  $
//   
// DESCRIPTION:
//   
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _G_RASTER_H_
#define _G_RASTER_H_

#include <types.h>
#include <m_rect.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif


//------------------------------ 

// Raster flag definitions

//bits 2,3,4 are used as an index into a jump table. DO NOT change their order
//DO NOT use bits 0,1
#define RASTER_DEAD     (1<<2)   // TRUE if we don't want to draw this one
#define RASTER_LCLIPPED (1<<3)   // TRUE has been left clipped and lx, rx are filled
#define RASTER_RCLIPPED (1<<4)   // TRUE has been right clipped and lx, rx are filled
#define RASTER_STEP_Y   (1<<5)   // TRUE if step to next Y position
#define RASTER_UNUSED   (1<<31)  // TRUE if not been used (used by debug libraries)

//------------------------------ 
//
//    The raster structure/type defines all of the information
//    needed to fully render one line of an image (except for
//    the texture map and/or actual color data being drawn).
//    Not all renderers will use all of this data -- only the
//    relevant columns should be filled, as needed.
//
// NOTES:
//
//    STEP_Y - This is a pre-increment.  The first raster in
//             a series across X will have this set, the others not.
//
// BE SURE TO TRACK CHANGES IN g_raster.inc!!!
//
struct GFXRaster
{
   Int32   f;       // Bit-field Flags.
   Int32 x;       // X position on the screen.
   // Number of steps from Left to Right [x...x+cnt]
   Int32 cnt;     // cnt = (width of raster) - 1

   // GFXRaster linkage.
   GFXRaster *next;

   // Clipping
   Int32 lx;      // Left-X from clipping operation.
   Int32 rx;      // Right-X from clipping operation.

   // Initial values
   Fix16 u;       // X-index into a texture map.  (Also known as R)
   Fix16 su;      // Step in u.

   Fix16 v;       // Y-index into a texture map.  (Also known as S)
   Fix16 sv;      // Step in v.

   Fix16 w;       // Perspective correction factor.
   Fix16 sw;      // Step in w.

   // Scale or Step values
                  // Step in X is implied, == 1.0
   Fix16 sz;      // Step in Z.
   union
   {
      Fix16 ss;      // Step in Shade.
      Fix16 sr;
   };
   Fix16 z;       // Z position, increasing away from the viewplane.
   union
   {
      Fix16 s;       // Shade value, 0==Black
      Fix16 r;
   };

   Fix16 g;
   Fix16 b;
   Fix16 sg;
   Fix16 sb;
};

//------------------------------ 

struct GFXRasterList
{
   Int32       top_y;         // Y value of first Raster.
   Int32       height;        //  = (Height of rasterized object - 1)
   Int32       maxRasters;    // Number of Rasters [0...max) in table.
   Int32       unusedRasters; // Number of unused Rasters in table.
   GFXRaster   *freeRaster;   // Pointer to first free Raster.
   GFXRaster   *topRaster;    // Pointer to top_y Raster. (initially eq list)
   GFXRaster   *list;         // Pointer to head of Raster array.

   float       su;            // Raster list step in U
   float       sv;            // Raster list step in V
   float       sw;            // Raster list step in W
   union
   {
   Int32       ss;            // Raster list step in Shade
   Int32       sr;
   };

   Int32       sg;
   Int32       sb;
   Int32       sz;            // Raster list step in Z
   
   Int32       color;         // color value for non-textured
   BYTE        *texmap;       // texture map
   BYTE        *shademap;     // shade map
   BYTE        *transmap;     // translucency table
   
   Int32       hazeShift;     // haze shift value
                              // for hazing
   Int32       *redMap;       // red lighting map for 16-bit color
   Int32       *greenMap;
   Int32       *blueMap;
   
   ~GFXRasterList();
};

#define ACTIVE_S  (1<<0)      // Gouraud shading
#define ACTIVE_UV (1<<1)      // Texture mapping
#define ACTIVE_W  (1<<2)      // Perspective-correction
#define ACTIVE_Z  (1<<3)      // Z-Buffering

//------------------------------ 

struct GFXEdge
{
   Int32   lx;
   Int32   rx;

   GFXEdge *next;
};

//------------------------------ 

class GFXSurface;

struct GFXEdgeList
{
   Int32       top_y;         // Y value of first Edge.
   Int32       height;        // Height of the edge table
   Int32       maxEdges;      // Number of Edges in the table[0..max)
   Int32       unusedEdges;   // Number of unused edges in the table
   GFXEdge     *freeEdge;     // Pointer to the first free Edge
   GFXEdge     **list;         // Pointer to array of Edges.
   GFXEdge     *edgeList;     // edge list
   Int32       maxHeight;      // maximum number of scanlines in the table...
                              // 0 if this was a loaded edge table.
   Int32       usedHeight;    // how many of the list array have been
                              // used.  Initially == height
   Int32       *packedEdgeList; // packed representation of the
                                // edge list.
                              
   GFXEdgeList();
   GFXEdgeList(int maxHeight, int maxEdges);
   ~GFXEdgeList();
   
   void clear();
   void pack();
   void unpack();
   Bool createFromSurface(GFXSurface *sfc, Int32 bkColor);
   Bool generateSubEdgeList(GFXEdgeList *sub, RectI *bounds);
   
   static GFXEdgeList* load(const char* in_filename);
   Bool read(const char* in_filename);
   Bool write(const char* in_filename);
};

struct EdgeListUsage
{
   Int32 freedEdges;
   Int32 newUsedEdges;
   Int32 numOrgEdges;
   Int32 numSubEdges;
};

extern EdgeListUsage gfxEdgeUsage;

//------------------------------ 

// THIS BELONGS IN MATH OR TYPES
#define FIX16_SHIFT        (16)
#define FIX16_TO_INT(i)    (((i)<0)?(-((-(i))>>FIX16_SHIFT)):((i)>>FIX16_SHIFT))
#define INT_TO_FIX16(i)		((i)<<FIX16_SHIFT)

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif
