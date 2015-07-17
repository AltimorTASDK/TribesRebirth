//--------------------------------------------------------------------------- 
//
// simStarField.h
//
//--------------------------------------------------------------------------- 

#ifndef _SIMSTARFIELD_H_
#define _SIMSTARFIELD_H_

#include <simNetObject.h>
#include <simRenderGrp.h>
#include <simLight.h>
#include <ts_shapeInst.h>
#include <move.h>

//--------------------------------------------------------------------------- 
//
// The celestial globe is divided into regions, the width and height of
// each region is define by AZ_REGION_SIZE and INC_REGION_SIZE (degrees)
// respectively.
//
// At creation time, the points at the intersections of the regions are
// cached (StarField::controlPoints).  At render time, these control points
// are projected, after which each region is visited, if a control point
// of the region 

#define AZ_REGION_SIZE   30
#define AZ_REGION_STRIDE (360/AZ_REGION_SIZE)
#define INC_REGION_SIZE  30
// the region at the very end is for stars below the equator
#define REGION_COUNT     (AZ_REGION_STRIDE*(90/INC_REGION_SIZE))

//--------------------------------------------------------------------------- 

class SimStarField : public SimNetObject, SimRenderImage
{
   private:
      typedef SimNetObject Parent;
      
   protected:
      enum NetMaskBits
      {
         Modified = 1,
      };

      struct Star
      {
         Point3F pos;
         UInt8   brightness;
         Star(const Point3F &p, const UInt8 b) { pos = p;  brightness = b; }
      };
      struct Region
      {
         Bool visible;
         Vector<Star> stars;
      };
      struct RegionPoint
      {
         int regions[4];
         Point3F pos;
      };

      enum { MAX_STARS = 1500 };
      ColorF colors[3];
      
      int paletteColors[3]; 
      bool globeLines;
      bool initialized;
      bool inFrontOfSky;
      bool visible;
      bool bottomVisible;
      
      RegionPoint controlPoints[REGION_COUNT + 1];
      Region regions[REGION_COUNT + 1];

      void init(GFXPalette *palette);
      void setSortValue();
      void regionVisible(int region);
      
   public:
      static float PrefStars;

      SimStarField();
   	bool processQuery(SimQuery *);
		bool onAdd();
		void inspectRead(Inspect *inspector);
		void inspectWrite(Inspect *inspector);
      
      void render(TSRenderContext &rc);
      void toggleGlobeLines();
      void setBottomVisible(bool _visible);
      void setVisibility(bool _visible);
      
      DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
      void unpackUpdate(Net::GhostManager *gm, BitStream *stream);

      DECLARE_PERSISTENT(SimStarField);
      static void initPersistFields();
      Persistent::Base::Error read(StreamIO &, int version, int user);
      Persistent::Base::Error write(StreamIO &, int version, int user);
      int version();
};

//--------------------------------------------------------------------------- 

#endif
