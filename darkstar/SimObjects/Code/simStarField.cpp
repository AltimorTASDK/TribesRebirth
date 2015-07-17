//--------------------------------------------------------------------------- 
//
// simStarfield.cpp
//
//--------------------------------------------------------------------------- 

#include <sim.h>
#include <g_pal.h>
#include <g_bitmap.h>
#pragma warn -inl
#pragma warn -aus

#include <ts.h>
#include <simLightGrp.h>
#include <simLight.h>
#include <simTerrain.h>
#include <SimTagDictionary.h>
#include <editor.strings.h>
#include "simSky.h"
#include "simStarField.h"
#include "d_caps.h"
#include "g_surfac.h"

//--------------------------------------------------------------------------- 

#define DEGRAD (M_PI/180.0)
#define RADDEG (180.0/M_PI)

float STARFIELD_DISTANCE = STARFIELD_FRONT_DISTANCE;

//--------------------------------------------------------------------------- 

static int getRegion(float az, float inc)
{
   // the last region is reserved for all stars under the equator
   if (inc < 0.0f)
      return REGION_COUNT;
      
   int a = (int)az/AZ_REGION_SIZE;     
   int i = (int)inc/INC_REGION_SIZE;
   return(a + i*AZ_REGION_STRIDE);
}

float SimStarField::PrefStars = 1.0f;


//--------------------------------------------------------------------------- 

SimStarField::SimStarField()
{
   globeLines = false;
   initialized = false;
   inFrontOfSky = true;
   visible = true;
   itype = Background;
   sortValue = STARFIELD_FRONT_SORTVALUE;
   colors[0].set(1.0f, 1.0f, 1.0f);
   colors[1].set(0.5f, 0.5f, 0.5f);
   colors[2].set(0.25f, 0.25f, 0.25f);
   netFlags.set(SimNetObject::ScopeAlways);
   netFlags.set(SimNetObject::Ghostable);
   bottomVisible = false;
}

//--------------------------------------------------------------------------- 

void SimStarField::toggleGlobeLines()
{
   globeLines = !globeLines; 
   initialized = false;
}

//--------------------------------------------------------------------------- 

void SimStarField::inspectWrite(Inspect *inspect)
{
	Parent::inspectWrite(inspect);

   inspect->write(IDITG_STARS_IN_FRONT, inFrontOfSky);

   Point3F b(colors[0].red, colors[0].green, colors[0].blue);
   inspect->write(IDITG_SKY_BRIGHTSTAR, b);
   Point3F m(colors[1].red, colors[1].green, colors[1].blue);
   inspect->write(IDITG_SKY_MEDIUMSTAR, m);
   Point3F d(colors[2].red, colors[2].green, colors[2].blue);
   inspect->write(IDITG_SKY_MEDIUMSTAR, d);
}   

//--------------------------------------------------------------------------- 

void SimStarField::setSortValue()
{
   if (inFrontOfSky) {
      // swap sky & star distances back to default
      STARFIELD_DISTANCE = STARFIELD_FRONT_DISTANCE;
      sortValue = STARFIELD_FRONT_SORTVALUE;
   }
   else {
      STARFIELD_DISTANCE = STARFIELD_BACK_DISTANCE;
      sortValue = STARFIELD_BACK_SORTVALUE;
   }
}

//--------------------------------------------------------------------------- 

void SimStarField::inspectRead(Inspect *inspect)
{
	Parent::inspectRead(inspect);

   inspect->read(IDITG_STARS_IN_FRONT, inFrontOfSky);
   setSortValue();
   Point3F b;
   inspect->read(IDITG_SKY_BRIGHTSTAR, b);
   colors[0].set(b.x, b.y, b.z);
   Point3F m;
   inspect->read(IDITG_SKY_MEDIUMSTAR, m);
   colors[1].set(m.x, m.y, m.z);
   Point3F d;
   inspect->read(IDITG_SKY_MEDIUMSTAR, d);
   colors[2].set(m.x, m.y, m.z);
   initialized = false;
   setMaskBits(Modified);
}   

//--------------------------------------------------------------------------- 

bool SimStarField::onAdd()
{
	if (!Parent::onAdd())
		return false;
	if (!manager->isServer())
		addToSet(SimRenderSetId);
	initialized = false;
   setSortValue();
	return true;
}	

//--------------------------------------------------------------------------- 

bool SimStarField::processQuery(SimQuery *query)
{
   switch (query->type) {
      case SimRenderQueryImageType: {
         SimRenderQueryImage *q = static_cast<SimRenderQueryImage *>(query);
         q->image[0] = this;
         q->count = 1;
         break;
      }
      default:
         return false;
   }
   return true;
}

//--------------------------------------------------------------------------- 

DWORD SimStarField::packUpdate(Net::GhostManager *, DWORD, BitStream *stream)
{
   stream->writeFlag(visible);
   stream->writeFlag(bottomVisible);
   stream->write(sizeof(colors), (Int8 *)&colors);
   stream->writeFlag(inFrontOfSky);
   return 0;
}

//--------------------------------------------------------------------------- 

void SimStarField::unpackUpdate(Net::GhostManager *, BitStream *stream)
{
   visible = stream->readFlag();
   bottomVisible = stream->readFlag();
   stream->read(sizeof(colors), (Int8 *)&colors);
   inFrontOfSky = stream->readFlag();
   setSortValue();
   initialized = false;
}

//-------------------------------------------------------------------------- 

//-------------------------------------------------------------------------- 

void SimStarField::initPersistFields()
{
   //Parent::initPersistFields();
   addField("inFrontOfSky", TypeBool, Offset(inFrontOfSky,SimStarField));
   addField("colors", TypePoint3F, Offset(colors, SimStarField), 3);
}

Persistent::Base::Error 
SimStarField::read(StreamIO &sio, int version, int u)
{
   Persistent::Base::Error error = SimNetObject::read(sio, version, u);
   if (error != Persistent::Base::Ok) 
      return error;

   if (version == 0)
   {
      int oldMaxStars;
      sio.read(sizeof(oldMaxStars), (Int8 *)&oldMaxStars);
   }
   if (version >= 2)
      sio.read(&inFrontOfSky);

   sio.read(sizeof(colors), (Int8 *)&colors);
	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}

//--------------------------------------------------------------------------- 

Persistent::Base::Error 
SimStarField::write(StreamIO &sio, int version, int u)
{
   Persistent::Base::Error error = SimNetObject::write(sio, version, u);
   if (error != Persistent::Base::Ok) 
      return error;

   sio.write(inFrontOfSky); 
   sio.write(sizeof(colors), (Int8 *)&colors);
	return (sio.getStatus() == STRM_OK) ? Ok : WriteError;
}

int SimStarField::version()
{
   return (2);
}   


//--------------------------------------------------------------------------- 
void SimStarField::setBottomVisible(bool _visible)
{
   bottomVisible = _visible;
   setMaskBits(Modified);
}
   
//--------------------------------------------------------------------------- 
void SimStarField::setVisibility(bool _visible)
{
   visible = _visible;
   setMaskBits(Modified);
}
 
//--------------------------------------------------------------------------- 

void SimStarField::init(GFXPalette *palette)
{
   // lookup desired star colors in the palette
   if (palette) {
      paletteColors[0] = palette->GetNearestColor(
         255.0f*colors[0].red, 255.0f*colors[0].green, 255.0f*colors[0].blue);
      paletteColors[1] = palette->GetNearestColor(
         255.0f*colors[1].red, 255.0f*colors[1].green, 255.0f*colors[1].blue);
      paletteColors[2] = palette->GetNearestColor(
         255.0f*colors[2].red, 255.0f*colors[2].green, 255.0f*colors[2].blue);
   }
   
   int i, starCt;
   float c;

   int starCount = MAX_STARS;

   for (int r = 0; r < REGION_COUNT; r++) 
      regions[r].stars.clear();
       
   if (globeLines)
   {
      for (float az_ = 0.0f; az_ < 360.0f; az_ += 15.0f)
         for (float inc_ = -90.0f; inc_ < 90.0f; inc_ += 2.0f) {
            // throw out stars below the equator
            float inc = inc_*DEGRAD;  
            float az  = az_*DEGRAD;
            int r = getRegion(az, inc);
            c    = STARFIELD_DISTANCE*m_cos(inc);
            float b = 3;
            regions[r].stars.push_back(Star(Point3F(c*m_cos(az), 
                                                    c*m_sin(az), 
                                                    STARFIELD_DISTANCE*m_sin(inc)), b));
         }
      for (float inc_ = -90.0f; inc_ < 90.0f; inc_ += 20.0f)
         for (float az_ = 0.0f; az_ < 360.0f; az_ += 5.0f) {
            // throw out stars below the equator
            float inc = inc_*DEGRAD;  
            float az  = az_*DEGRAD;
            int r = getRegion(az, inc);
            c    = STARFIELD_DISTANCE*m_cos(inc);
            float b = 3;
            regions[r].stars.push_back(Star(Point3F(c*m_cos(az), 
                                                    c*m_sin(az), 
                                                    STARFIELD_DISTANCE*m_sin(inc)), b));
         }
   }
   else
   {
      // 2*starCount so that stars below the equator are created too
      for (i = starCt = 0; starCt < 2*starCount; i++) 
      {
         float az, inc;
         int b;

         // generate a random star
         #if 0
         az  = float(rand()%3600)*0.1;
         inc = float(rand()%1000)*0.001; 
         inc = inc*inc*inc*90.0f;
         #endif
         
         float len;
         Point3F p;
         while (1) {
            p.x = float(rand()%2000 - 1000);
            p.y = float(rand()%2000 - 1000);
            p.z = float(rand()%2000 - 1000);
            //p.z = float(rand()%20000) - 10000;
            if (p.x != 0.0f || p.y != 0.0f) {
               len = p.lenf();
               if (len <= 1000)
                  break;
            }
         }
         az = m_atan(p.x, p.y);         
         inc = m_asin(p.z/len);
         
         
         int bright = rand()%30;
         if (bright < 1) b = 0;
         else if (bright < 1) b = 1;
         else if (bright < 9) b = 2;
         else if (bright < 23) b = 3;
         else b = 4;

         // throw out stars below the equator
         float degAz = RADDEG*az;
         if (degAz < 0.0f)
            degAz += 360.0f;
         int r = getRegion(degAz, RADDEG*inc);
         c    = STARFIELD_DISTANCE*m_cos(inc);
         regions[r].stars.push_back(Star(Point3F(c*m_cos(az), 
                                                 c*m_sin(az), 
                                                 STARFIELD_DISTANCE*m_sin(inc)), b));
         starCt++;
      }
   }
   
   // create the control point table
   int regionNo = 0;
   RegionPoint point;
   for (int inc = 0; inc < 90; inc += INC_REGION_SIZE) 
      for (int az = 0; az < 360; az += AZ_REGION_SIZE) {
         // find the regions that intersect this point
         for (int i = 0; i < 4; i++) point.regions[i] = -1;
         int j = 0;
         point.regions[j++] = regionNo;
         if (az == 0) point.regions[j++] = regionNo + AZ_REGION_STRIDE - 1;
         else         point.regions[j++] = regionNo - 1;
         if (inc > 0) {
            point.regions[j++] = regionNo - AZ_REGION_STRIDE;
            if (az == 0) point.regions[j++] = regionNo - 1;
            else         point.regions[j++] = regionNo - AZ_REGION_STRIDE - 1;
         }
         c = STARFIELD_DISTANCE*m_cos(DEGRAD*inc);
         point.pos = Point3F(c*m_cos(DEGRAD*az), 
                             c*m_sin(DEGRAD*az), 
                             STARFIELD_DISTANCE*m_sin(DEGRAD*inc));
         controlPoints[regionNo++] = point;         
      }

   // create a special case control point for the celestial north pole
   c   = STARFIELD_DISTANCE*m_cos(DEGRAD*90);
   point.pos = Point3F(c*m_cos(DEGRAD*0), c*m_sin(DEGRAD*0), STARFIELD_DISTANCE*m_sin(DEGRAD*90));
   controlPoints[REGION_COUNT] = point;         
   
   initialized = true;
}

//--------------------------------------------------------------------------- 

void SimStarField::regionVisible(int region)
{
   AssertFatal(region >= 0 && region <= REGION_COUNT, "attempt to write to bad region");
   regions[region].visible = true;
}

//--------------------------------------------------------------------------- 

void SimStarField::render(TSRenderContext &rc)
{
   if (IsZero(PrefStars) || !visible)
      return;

   GFXSurface *gfxSurface = rc.getSurface();
   if (!initialized)
      init(gfxSurface->getPalette());

   TSCamera *camera = rc.getCamera();
   if (!dynamic_cast<TSPerspectiveCamera *>(camera))
      return;

   float visibleDistance;
   SimTerrain *terrain = static_cast<SimTerrain *>
      (manager->findObject(SimTerrainId));
   if (terrain)
      visibleDistance = terrain->getVisibleDistance();
   else
      visibleDistance = 10000.0f;

   gfxSurface->setHazeSource(GFX_HAZE_NONE);
   gfxSurface->setShadeSource(GFX_SHADE_NONE);
   gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
	gfxSurface->setFillMode(GFX_FILL_CONSTANT);
   gfxSurface->setTexturePerspective(FALSE);
	gfxSurface->setTransparency(FALSE);
   Point3F displacement = camera->getTCW().p;
   displacement.z *= -(STARFIELD_DISTANCE - visibleDistance)/visibleDistance;

   if (globeLines) {
      for (int i = 0; i < REGION_COUNT; i++)
         regions[i].visible = true;
      regions[REGION_COUNT].visible = bottomVisible;
   }
   else 
   {
      for (int i = 0; i < REGION_COUNT; i++)
         regions[i].visible = false;
   
      // the last region is reserved for stars under the equator, and is only
      // rendered when these are manually set to visible
      regions[REGION_COUNT].visible = bottomVisible;
   
      // find horiz & vert fov
      const float invNearDist = 1.0f/camera->getNearDist();
      float hFov = 2.0f*(atan(camera->getWorldViewport().lowerR.x*invNearDist));
      float vFov = 2.0f*(atan(camera->getWorldViewport().upperL.y*invNearDist));
      
      if (hFov > DEGRAD*(float(AZ_REGION_SIZE)) && vFov > DEGRAD*(float(INC_REGION_SIZE)))
      {
         // field of view is larger than a region, we can determine which regions
         // are visible by projecting control points
    
         // Find visible regions of stars
         Region *c = regions, *d = regions + REGION_COUNT;
         for (int i = 0; i < REGION_COUNT + 1; i++) {
            RegionPoint &cp = controlPoints[i];
            TS::TransformedVertex v;
            Point3F p = cp.pos;
            p += displacement;
            if (camera->transformProject(p, &v) == TS::TransformedVertex::Projected) 
               // found a visible control point, mark intersecting regions visible
               if (i < REGION_COUNT)
                  for (int j = 0; j < 4; j++) {
                     int p = cp.regions[j];
                     if (p != -1) 
                        //regions[p].visible = true;
                        regionVisible(p);
                  }
               else
                  // controlPoints[REGION_COUNT] is a special case of regions 
                  // that intersect the celestial north pole
                  for (int i = REGION_COUNT - AZ_REGION_STRIDE; i < REGION_COUNT; i++)
                     //regions[i].visible = true;
                     regionVisible(i);
         }
      }
      else {
         // determine which regions are visible by casting a ray from the
         // camera, the region it hit's plus that region's neighbors are visible
         Vector3F y;
         camera->getTCW().getRow(1, &y);

         // this routine flips out if y.z < 0 ...
         if (y.z <= 0)
         {
            y.z = 0.001f;
            y.normalize();
         }

         float az, inc;
         if (y.z == 1.0f) {
            az = 0.0f;
            inc = 90.0f;
         }
         else {
            az = RADDEG*m_atan(y.x, y.y);
            if (az < 0.0f)
               az += 360.0f;
            inc = RADDEG*m_asin(y.z);
         }
         // find the region the camera hits
         #define Rows (90/INC_REGION_SIZE)
         int n = getRegion(az, inc);
         int row = n/AZ_REGION_STRIDE;
         int rowMin = row*AZ_REGION_STRIDE;
         int rowMax = rowMin + (AZ_REGION_STRIDE - 1);

         regions[n].visible = true;
         // mark neighbors to left & right
         if (n == rowMin) 
            regionVisible(rowMax);   //regions[rowMax].visible = true;
         else 
            regionVisible(n - 1);    //regions[n - 1].visible = true;
         if (n == rowMax)
            regionVisible(rowMin);   //regions[rowMin].visible = true;
         else
            regionVisible(n + 1);    //regions[n + 1].visible = true;

         if (row != Rows -1) {
            // not top row, mark neighbors above
            regions[n + AZ_REGION_STRIDE].visible = true;
            if (n == rowMin) 
               regionVisible(rowMax + AZ_REGION_STRIDE);  //regions[rowMax + AZ_REGION_STRIDE].visible = true;
            else 
               regionVisible((n - 1) + AZ_REGION_STRIDE); //regions[(n - 1) + AZ_REGION_STRIDE].visible = true;
            if (n == rowMax)
               regionVisible(rowMin + AZ_REGION_STRIDE);  //regions[rowMin + AZ_REGION_STRIDE].visible = true;
            else
               regionVisible(n + 1 + AZ_REGION_STRIDE);   //regions[n + 1 + AZ_REGION_STRIDE].visible = true;
         } 
         if (row != 0) {
            // not bottom row, mark neighbors below
            regionVisible(n - AZ_REGION_STRIDE);  //regions[n - AZ_REGION_STRIDE].visible = true;
            if (n == rowMin) 
               regionVisible(rowMax - AZ_REGION_STRIDE);  //regions[rowMax - AZ_REGION_STRIDE].visible = true;
            else 
               regionVisible((n - 1) - AZ_REGION_STRIDE); //regions[(n - 1) - AZ_REGION_STRIDE].visible = true;
            if (n == rowMax)
               regionVisible(rowMin - AZ_REGION_STRIDE);  //regions[rowMin - AZ_REGION_STRIDE].visible = true;
            else
               regionVisible((n + 1) - AZ_REGION_STRIDE); //regions[(n + 1) - AZ_REGION_STRIDE].visible = true;
         }
      }      
   }

   // Render regions of stars that were found to be visible
   for (int r = 0; r <= REGION_COUNT; r++) 
   {
      if (!regions[r].visible)
         continue;

      Region &region = regions[r];
      int starsToRender = (int)(region.stars.size()*PrefStars);
      for (int i = 0; i < starsToRender; i++) 
      {
         Point3F p = region.stars[i].pos;
         p += displacement;
         TS::TransformedVertex v;
         if (camera->transformProject(p, &v) == TS::TransformedVertex::Projected)
         {
            int s;
   			int color;
            switch (region.stars[i].brightness)
            {
               case 0:  color = paletteColors[0];  s = 2;  break;
               case 1:  color = paletteColors[1];  s = 2;  break;
               case 2:  color = paletteColors[0];  s = 1;  break;
               case 3:  color = paletteColors[1];  s = 1;  break;
               case 4:  color = paletteColors[2];  s = 1;  break;
            }            
            float y = v.fPoint.y;
            if (y < 5) continue;
            float x = v.fPoint.x;
            if (x < 5) continue;
		   	if(s > 1)
			   	gfxSurface->drawRect3d_f(&RectI(x-s+1,y-s+1,x,y), v.fPoint.z, color);
   			else
	   			gfxSurface->drawPoint3d(&Point2I(x, y), v.fPoint.z, color);
         }
      }
   }
}

