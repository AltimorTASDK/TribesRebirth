/* --------------------------------------------------------------------------- 

   simVolumetric.cpp  

----------------------------------------------------------------------------- */ 

//       COMMON\SIM\INC:
// #include "simLog.h"
// #include "commonEditor.strings.h"
#include "esfObjectTypes.h"

// #include <darkstar.strings.h>
// #include <editor.strings.h>
#include <commonEditor.strings.h>

//    simObjects\INC:
#include <MissionEditor.h>
#include <SimTerrain.h>

//    sim\Inc:
#include <simPersistTags.h>
#include <simMovement.h>
#include <d_caps.h>
#include "g_surfac.h"

#include "simVolumetric.h"
#include "simDamageEv.h"
#include "simResource.h"

// These will tell us when the container has been resized
   
/*-----------------------------------------------------------------------**
**                                                                       **
**               SimVolumetric::BoxRenderDmlImage                         **
**                                                                       **
**-----------------------------------------------------------------------*/

inline void SimVolumetric::BoxRenderDmlImage::
clipHaze(TS::TransformedVertex &tv, Point3F point)
{
   float rFactor; 

   tv.fDist = m_distf(point, cameraPos);

   if (tv.fDist > rVisibleDistance)
   {
      tv.fStatus |= TS::ClipFarSphere;
   }

   rFactor = (tv.fDist - rHazeDistance) * rHazeScale - 1.0;

   if (tv.fDist > rHazeDistance)
   {
      tv.fColor.haze = 1.0 - (rFactor * rFactor);
   }
   else
   {
      tv.fColor.haze = 0.0001f;
   }
}

float octpts[8][2] =
{
   { 0, 1 },
   { .707f, .707f },
   { 1, 0 },
   { .707f, -.707f },
   { 0, -1 },
   { -.707f, -.707f },
   { -1, 0 },
   { -.707f, .707f }
};

Point2F octoPoints[16];
Point2F clipPoints[16];

void SimVolumetric::BoxRenderDmlImage::
drawSquare(Point3F a, Point3F b, Point3F /*c*/, Point3F d, TS::PointArray *pa, WhichAxis /*PLANE*/)
{
   // build a coordinate system for the poly water surface:

   Point3F fupv, ffdv, frtv;
   ffdv = b - a;
   frtv = d - a;

   float fdvlen = ffdv.len();
   float rtvlen = frtv.len();
   
   ffdv *= 1 / fdvlen;
   frtv *= 1 / rtvlen;

   m_cross(frtv, ffdv, &fupv);
   float dist = m_dot(cameraPos - a, fupv);

   // can't draw it - outside vis distance.
   if(dist >= rVisibleDistance)
      return;

   Point3F rCenter = cameraPos - a;
   Point2F center(m_dot(frtv, rCenter) / rtvlen, m_dot(ffdv, rCenter) / fdvlen);

   float radius = m_sqrt(rVisibleDistance * rVisibleDistance - dist * dist);
   if(radius < 1)
      return;

   Point2F scales(radius / rtvlen, radius / fdvlen);

   int i;
   for(i = 0; i < 8; i++)
   {
      octoPoints[i].set(octpts[i][0] * scales.x + center.x,
                        octpts[i][1] * scales.y + center.y);
   }   

   // now clip the octogon to the square (0,0) (1,1)

   int srcPoints = 8, destPoints = 0;
   int last;
   Point2F *src = octoPoints;
   Point2F *dest = clipPoints;
   for(last = srcPoints - 1, i = 0 ;i < srcPoints; last = i++)
   {
      if(src[last].x < 0)
      {
         if(src[i].x >= 0)
            dest[destPoints++].set(0, (src[i].y - src[last].y) *
               (-src[last].x / (src[i].x - src[last].x)) + src[last].y);
      }
      else
      {
         if(src[i].x >= 0)
            dest[destPoints++] = src[last];
         else
         {
            dest[destPoints++] = src[last];
            dest[destPoints++].set(0, (src[i].y - src[last].y) *
               (-src[last].x / (src[i].x - src[last].x)) + src[last].y);
         }
      }
   }
   if(destPoints == 0)
      return;

   srcPoints = destPoints;
   destPoints = 0;
   src = clipPoints;
   dest = octoPoints;

   for(last = srcPoints - 1, i = 0 ;i < srcPoints; last = i++)
   {
      if(src[last].x > 1)
      {
         if(src[i].x <= 1)
            dest[destPoints++].set(1, (src[i].y - src[last].y) *
               (1 - src[last].x / (src[i].x - src[last].x)) + src[last].y);
      }
      else
      {
         if(src[i].x <= 1)
            dest[destPoints++] = src[last];
         else
         {
            dest[destPoints++] = src[last];
            dest[destPoints++].set(1, (src[i].y - src[last].y) *
               (1 - src[last].x / (src[i].x - src[last].x)) + src[last].y);
         }
      }
   }
   if(destPoints == 0)
      return;

   srcPoints = destPoints;
   destPoints = 0;
   src = octoPoints;
   dest = clipPoints;

   for(last = srcPoints - 1, i = 0 ;i < srcPoints; last = i++)
   {
      if(src[last].y < 0)
      {
         if(src[i].y >= 0)
            dest[destPoints++].set((src[i].x - src[last].x) *
               (-src[last].y / (src[i].y - src[last].y)) + src[last].x, 0);
      }
      else
      {
         if(src[i].y >= 0)
            dest[destPoints++] = src[last];
         else
         {
            dest[destPoints++] = src[last];
            dest[destPoints++].set((src[i].x - src[last].x) *
               (-src[last].y / (src[i].y - src[last].y)) + src[last].x, 0);
         }
      }
   }
   if(destPoints == 0)
      return;

   srcPoints = destPoints;
   destPoints = 0;
   src = clipPoints;
   dest = octoPoints;

   for(last = srcPoints - 1, i = 0 ;i < srcPoints; last = i++)
   {
      if(src[last].y > 1)
      {
         if(src[i].y <= 1)
            dest[destPoints++].set((src[i].x - src[last].x) *
               (1 - src[last].y / (src[i].y - src[last].y)) + src[last].x, 1);
      }
      else
      {
         if(src[i].y <= 1)
            dest[destPoints++] = src[last];
         else
         {
            dest[destPoints++] = src[last];
            dest[destPoints++].set((src[i].x - src[last].x) *
               (1 - src[last].y / (src[i].y - src[last].y)) + src[last].x, 1);
         }
      }
   }
   // destPoints, all in octoPoints.

   if(destPoints == 0)
      return;
   TS::VertexIndexPair v[16];
   
   ffdv *= fdvlen;
   frtv *= rtvlen;

   for(i = 0; i < destPoints; i++)
      v[i].fVertexIndex = pa->addPoint((frtv * octoPoints[i].x) + (
            ffdv * octoPoints[i].y) + a);
   pa->drawPoly ( destPoints, v, 0 );

   return;
/*
   I1 = pa->addPoint(P1);
   I2 = pa->addPoint(P2);
   I3 = pa->addPoint(P3);
   I4 = pa->addPoint(P4);

   set_text_coords(P1, P2, P3, P4, PLANE);

   clipHaze(pa->getTransformedVertex(I1), P1); 
   clipHaze(pa->getTransformedVertex(I2), P2); 
   clipHaze(pa->getTransformedVertex(I3), P3); 
   clipHaze(pa->getTransformedVertex(I4), P4);

   // structure defined in ts3\inc\ts_vertex.h:
   TS::VertexIndexPair v[3];

   v[0].fVertexIndex = I1;             // doc: only can texture three point
   v[1].fVertexIndex = I2;             //      polygons.  
   v[2].fVertexIndex = I4;
   v[0].fTextureIndex = 0;
   v[1].fTextureIndex = 1;
   v[2].fTextureIndex = 3;
   pa->drawPoly ( 3, v, 0 );
   
   v[0].fVertexIndex = I2;
   v[1].fVertexIndex = I3;
   v[2].fVertexIndex = I4;
   v[0].fTextureIndex = 1;
   v[1].fTextureIndex = 2;
   v[2].fTextureIndex = 3;
   pa->drawPoly ( 3, v, 0 );*/
}


/*
        Z axis 

          ^
          |
          |
             f --------- g
            /|          /|
           / |         / |
          b -+------- c  |
          |  |        |  |
          |  |        |  |
          |  e -------+- h
          | /         | /
          |/          |/
          a --------- d     ------> X axis
*/

static Point2F textCoord[4];



void SimVolumetric::BoxRenderDmlImage::set_text_coords ( 
         const Point3F & a, const Point3F & b, 
         const Point3F & c, const Point3F & d, 
         WhichAxis  axis 
               )
{
#  define SET_BOX_COORDS(c1,c2)   {     \
         textCoord[0].x = a.c1 * textureScale;       \
         textCoord[0].y = a.c2 * textureScale;       \
         textCoord[1].x = b.c1 * textureScale;       \
         textCoord[1].y = b.c2 * textureScale;       \
         textCoord[2].x = c.c1 * textureScale;       \
         textCoord[2].y = c.c2 * textureScale;       \
         textCoord[3].x = d.c1 * textureScale;       \
         textCoord[3].y = d.c2 * textureScale;   }
            
            
   if ( flags.test(CANWRAP) )
   {
      if ( axis == XY )
      {
         SET_BOX_COORDS(x,y)
      }
      else if ( axis == YZ )
      {
         SET_BOX_COORDS(y,z)
      }
      else
      {
         SET_BOX_COORDS(z,x)
      }
   }
}

void SimVolumetric::BoxRenderDmlImage::buildTopView(Point3F   P0, 
                                                    Point3F   P1)
{
   // Figure out what the partitioning is
   int xPartition = (P1.x - P0.x) / iSubDivisions;
   int yPartition = (P1.y - P0.y) / iSubDivisions;

   if (tvView)
   {
      for (int i = 0; i < iOldDivs; i ++)
         delete [] tvView[i];
      delete [] tvView;
   }

   // Keep track of new dimensions
   oldP0 = P0;
   oldP1 = P1;
   iOldDivs = iSubDivisions;

   // Dynamically create a 2D array
   tvView = (TopView **)(new TopView[iSubDivisions]);

   for (int k = 0; k < iSubDivisions; k ++)
      tvView[k] = (TopView *)(new TopView[iSubDivisions]);

   for (int i = 0; i < iSubDivisions; i ++)
   {
      for (int j = 0; j < iSubDivisions; j ++)
      {
         TopView &tvTmp = tvView[i][j];

         tvTmp.a = Point3F(P0.x + (i * xPartition),       
                           P0.y + (j * yPartition),       P1.z); // b
         tvTmp.b = Point3F(P0.x + (i * xPartition),       
                           P0.y + ((j + 1) * yPartition), P1.z); // f
         tvTmp.c = Point3F(P0.x + ((i + 1) * xPartition), 
                           P0.y + ((j + 1) * yPartition), P1.z); // g
         tvTmp.d = Point3F(P0.x + ((i + 1) * xPartition), 
                           P0.y + (j * yPartition),       P1.z); // c
      }
   }
}

void SimVolumetric::BoxRenderDmlImage::drawCube (
   const Point3F &P0, const Point3F &P1,
   TSRenderContext &rc, bool /*inside*/, bool fSoftware)
{
   SimTerrain     *terrain;
   TMat3F          mat;
   BitSet16        faces = flags;
   TS::PointArray *pointArray = rc.getPointArray();

   // Locate the vertices of our cube
   const Point3F &a = P0;
   const Point3F  b = Point3F(P0.x, P0.y, P1.z);
   const Point3F  c = Point3F(P1.x, P0.y, P1.z);
   const Point3F  d = Point3F(P1.x, P0.y, P0.z);
   const Point3F  e = Point3F(P0.x, P1.y, P0.z);
   const Point3F  f = Point3F(P0.x, P1.y, P1.z);
   const Point3F &g = P1;
   const Point3F  h = Point3F(P1.x, P1.y, P0.z);

   // Get some values for visibility and hazing from the terrain
   terrain = static_cast<SimTerrain *>
             (manager->findObject(SimTerrainId));

   if (terrain)
   {
      rHazeDistance    = terrain->getHazeDistance();
      rVisibleDistance = terrain->getVisibleDistance();
   }
   else
   {
      rHazeDistance    =  750.00f;
      rVisibleDistance = 1500.00f;
   }

   // Some computations to compute our haze values
   rDelta     = rVisibleDistance - rHazeDistance;
   rHazeScale = 1.0f / (rDelta + rDelta * 0.02f);

   // Need the camera location in OBJECT space
   mat       = rc.getCamera()->getTOC();
   cameraPos = mat.inverse().p;

   // Set the yon plane
   pointArray->setFarDist(rVisibleDistance);

   // Only re-calculate the points for the water cube when the dimensions 
   // of the cube or number of subdivisions are changed
   if (fSoftware == false && 
      (oldP0 != P0 || oldP1 != P1 || iOldDivs != iSubDivisions))
   {
      buildTopView(P0, P1);
   }

//   rc.getSurface()->setFillColor(COLOR_WATER_FB, ALPHAPAL_INDEX);
   rc.getSurface()->setFillColor(COLOR_WATER_FB);

   if (faces.test(FRONT))
      drawSquare(a, b, c, d, pointArray, ZX);
   
   if (faces.test(BACK))
      drawSquare(e, h, g, f, pointArray, ZX);
       
   //rc.getSurface()->setFillColor(COLOR_WATER_LR, ALPHAPAL_INDEX);
   rc.getSurface()->setFillColor(COLOR_WATER_LR);

   if (faces.test(LEFT))
      drawSquare(a, e, f, b, pointArray, YZ);

   if (faces.test(RIGHT))
      drawSquare(d, c, g, h, pointArray, YZ);
   
//   rc.getSurface()->setFillColor(COLOR_WATER_TD, ALPHAPAL_INDEX);
   rc.getSurface()->setFillColor(COLOR_WATER_TD);

   if (faces.test(DOWN))
      drawSquare(a, d, h, e, pointArray, XY);

   if (faces.test(TOP))
      drawSquare(b, f, g, c, pointArray, XY);
}


/* Need to set itype and sort key, the latter only in hardware.  
*/

void SimVolumetric::BoxRenderDmlImage::render(TSRenderContext &rc)
{
   bool            fSoftware;
   GFXSurface     *gfxSurface = rc.getSurface();
   TS::PointArray *pointArray = rc.getPointArray();
   
   itype = Normal;

   // only can wrap textures with certain hardware CAPabilities. 
   flags.clear(CANWRAP);
           
   pointArray->reset();
   pointArray->useIntensities(false);
   pointArray->setVisibility (TS::ClipMask);

   gfxSurface->setZTest(GFX_ZTEST_AND_WRITE);
   gfxSurface->setTextureWrap(false);

   // Set up the gfx depending on whether simple color box or animating texture.  
   if (texturing() && Dml != NULL && Dml->getMaterialsCount())
   {
      // HARDWARE
      if ( gfxSurface->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA )
      {
         fSoftware = false;
         itype = Translucent;
         flags.set(CANWRAP);
         
         gfxSurface->setFillMode(GFX_FILL_TEXTURE);
         gfxSurface->setTexturePerspective(true);
         gfxSurface->setTextureMap((*Dml)[curBmpIndex].getTextureMap());
         
         gfxSurface->setHazeSource(GFX_HAZE_VERTEX);
         gfxSurface->setShadeSource(GFX_SHADE_NONE);
         gfxSurface->setAlphaSource(GFX_ALPHA_CONSTANT);
         gfxSurface->setConstantAlpha(transLevel);

         pointArray->useTextures(textCoord);
         pointArray->useTextures(true);
      }
      // SOFTWARE
      else
      {
         fSoftware = true;
         gfxSurface->setFillMode(GFX_FILL_CONSTANT);
         gfxSurface->setTexturePerspective(false);

         gfxSurface->setHazeSource(GFX_HAZE_NONE);
         gfxSurface->setShadeSource(GFX_SHADE_NONE);
         gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
         //gfxSurface->setAlphaSource(GFX_ALPHA_FILL);

         pointArray->useTextures(false);
      }
   }
   // NO DML?
   else
   {
      fSoftware = true;
      gfxSurface->setFillMode(GFX_FILL_CONSTANT);
      gfxSurface->setTexturePerspective(false);
      
      gfxSurface->setHazeSource(GFX_HAZE_NONE);
      gfxSurface->setShadeSource(GFX_SHADE_NONE);
      gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
      gfxSurface->setTransparency(false);

      pointArray->useTextures(false);
   }

   Point3F  org ( 0, 0, 0 );
   Point3F  pointMin = transform.p;
   Point3F  pointMax = dimensions;
   
   pointMax += pointMin;
   Box3F bbox(pointMin, pointMax);

   Point3F pos = transform.p;

   rc.getCamera()->pushTransform(transform);
   drawCube ( org, dimensions, rc, bbox.contains(rc.getCamera()->getTCW().p), fSoftware);
   rc.getCamera()->popTransform();
}


SimVolumetric::BoxRenderDmlImage::BoxRenderDmlImage()
{ 
   curBmpIndex      = 0;
   Dml              = NULL;
   flags            = ALL;
   itype            = Translucent;
   transLevel       = 0.2f;
   textureScale     = 0.01f;
   tvView           = NULL;
   iOldDivs         = 0;
   iSubDivisions    = 1;
   rHazeDistance    = 1500.00f;
   rVisibleDistance = 1500.00f;
   rDelta           = 0.0f;
   rHazeScale       = 0.0f;

   oldP0.set(0.0f, 0.0f, 0.0f);
   oldP1.set(0.0f, 0.0f, 0.0f);
   cameraPos.set(0.0f, 0.0f, 0.0f);
}

SimVolumetric::BoxRenderDmlImage::~BoxRenderDmlImage()
{
   if (tvView)
   {
      for (int i = 0; i < iSubDivisions; i ++)
         delete [] tvView[i];
      delete [] tvView;

      tvView = NULL;
   }
}

/*----------------------------------------------------------------------**
**                                                                      **
**                       Sim Volumetric                                  **
**                                                                      **
**          Init / construct / destruct / ... methods                   **
**                                                                      **
**----------------------------------------------------------------------*/


#define DEFAULT_RADIUS    60

SimVolumetric::SimVolumetric ()
{
   float   rad = DEFAULT_RADIUS;
    
    // some default box sizes
   flags = 0;
   image.boxExtra = 0.0;  
   image.dimensions.set ( +rad, +rad, rad / 2.0 );

   initTransform (); 

   set_bounding_box ();
   
   netFlags.set(SimNetObject::Ghostable);
   netFlags.set(SimNetObject::ScopeAlways);

   
   // effects on players / items / .. inside us:
   currentVelocity.set ( 0, 0, 0 );
   currentDrag = 0.0;
   containerDrag = 0.0;
   density = 1.0f;
   damagePerSec = 0.0;
   
   volType = IDITG_SV_TYPE_WATER;
   
   dmlFileName = NULL;
}

void SimVolumetric::getAlphaBlend(TMat3F &objectPos, ColorF *alphaColor, float *alphaBlend)
{
   objectPos;
   alphaColor->red = 0;
   alphaColor->green = .3f;
   alphaColor->blue = 1.0f;
   *alphaBlend = .2f;
}


SimVolumetric::~SimVolumetric()
{
}


Box3F SimVolumetric::get_display_box ( void )
{
   Point3F  pointMin = image.transform.p;
   Point3F  pointMax = image.dimensions;
   
   pointMax += pointMin;
   return Box3F ( pointMin, pointMax );
}


void SimVolumetric::set_bounding_box ()
{
   Box3F  box_in_world_space = get_display_box ();
   
   box_in_world_space.fMin -= image.boxExtra;
   box_in_world_space.fMax += image.boxExtra;
   
   setBoundingBox ( box_in_world_space );
}



void SimVolumetric::initTransform ( const Point3F * pos )
{
   EulerF      nulRot ( 0, 0, 0 );        // rotation is always zero.  
   Point3F     location;
   
   if ( pos == NULL )
      location.set ( 0, 0, 0 );
   else
      location = *pos;
      
   image.transform.set ( nulRot, location );
}


void SimVolumetric::syncBoxes()
{
   // set up collision image box.  is this only needed for mission editing?  
   set_bounding_box ();
}


// Set the file name and set mask bits if it's a name change.  Also return true if so.  
//
bool SimVolumetric::setFileName ( const char *fileName )
{
   delete [] dmlFileName;
   dmlFileName = NULL;

   setMaskBits ( Edited );

   if ( fileName != NULL && fileName[0] )
   {
      dmlFileName = new char[strlen(fileName) + 1];
      strcpy ( dmlFileName, fileName );
      return true;
   }

   return false;
}



// only do on ghost?  
void SimVolumetric::loadDml ( void )
{ 
   image.texturesOff ();
    
   if ( dmlFileName != NULL )
   {
      ResourceManager *rm = SimResource::get(manager);

      matList = rm->load ( dmlFileName, true );       // loads just names

      if ( bool(matList) )
      {
         matList->load ( *rm, true );            // this loads contents 
         if ( matList->isLoaded() )
         {
            image.setDml ( matList );
            image.texturesOn ();                // set to off normally. 
         }
      }
   }
   
   if ( ! image.texturing () )
      image.setDml ( NULL );
}




/*--------------------------------------------------------------------------*
**                                                                         **
**                   Handle the inspector read and write.                  **
**                     Mission editing handling...                         **
**                                                                         **
**-------------------------------------------------------------------------*/

   
bool SimVolumetric::enable_editing ()
{
   if ( mode == PLAYING )
   {
      mode = EDITING;
      
      // WANT TO TURN ON THOSE FACES WHICH ARE INVISIBLE AND PUT 
      // A LOUD COLOR ON THEM.  ??
      
      // image.texturesOff ();
      return ( true );
   }
   return ( false );
}

   
bool SimVolumetric::enable_playing ()
{
   if ( mode == EDITING )
   {
      mode = PLAYING;
      // image.texturesOn ();
      return ( true );
   }
   return ( false );
}


const char * SimVolumetric::fetchTagString ( Int32 Id ) 
{
   return ( SimTagDictionary::getString(manager, Id) );
}


static  const  Inspect::IString   vol_type_filter = "IDITG_SV_TYPE*";


// These flags must be in order of the bits in flag array.  
//    Figure out from string which bits are wanted.  Used to toggle
//       faces.
//
UInt16 SimVolumetric::string_to_bits ( const char * str )
{
   UInt16         bits = 0;
   const char     *on = fetchTagString ( IDITG_SV_TOGGLE_PARSE_UPPER );
   Int16          len = strlen ( on );

   for ( Int16 i = 0; i < len; i++ )
      if ( strchr ( str, on [ i ] ) != NULL )
         bits |= BIT(i);

   return ( bits );
}

// Modify a string on the bits selected.  For each OFF flag - convert that 
// letter to lower case.  A lowercase array is used to avoid tolower() 
// for the sake of globalization.
//
char * SimVolumetric::bits_to_string ( char modify [], UInt16 bits )
{
   const char     *on = fetchTagString ( IDITG_SV_TOGGLE_PARSE_UPPER );
   const char     *off = fetchTagString ( IDITG_SV_TOGGLE_PARSE_LOWER );
   Int16          len = strlen ( on );
   char           *cp;
                       
   for ( Int16 i = 0; i < len; i++ )
      if ( ! ( bits & BIT(i) ) )
         if ( (cp = strchr ( modify, on[i] )) != NULL )
            *cp = off [ i ];

   return ( modify );
}


void SimVolumetric::inspectWrite ( Inspect* inspect )
{
   Parent::inspectWrite(inspect);

   char stringBuffer [ Inspect::MAX_STRING_LEN ];
   
   // temporary strings...
   inspect->write(IDITG_SV_POSITION, fetchTransform ().p);
   inspect->write(IDITG_SV_DIMENSIONS, image.dimensions);
   
   inspect->write(IDITG_SV_TEXTURE_SCALE, image.textureScale);
   inspect->write(IDITG_SV_TRANS_LEVEL, image.transLevel);
   inspect->write(IDITG_SV_ANIM_SCALE, animTimeScale);
   
   // check out the flags array.
   {
      strcpy ( stringBuffer, fetchTagString(IDITG_SV_TOGGLE_STR));
      bits_to_string ( stringBuffer, image.flagsGet());
      inspect->write ( IDITG_SV_TOGGLES, stringBuffer);
   }
   
   inspect->write(IDITG_SV_OBJECT_TYPE, false, vol_type_filter, volType);
   inspect->write(IDITG_SV_CURRENT_VEL, currentVelocity);
   inspect->write(IDITG_SV_CURRENT_DRAG, currentDrag);
   inspect->write(IDITG_SV_DENSITY, density);
   inspect->write(IDITG_SV_DAMAGE, damagePerSec);
   inspect->write(IDITG_SV_CONTAINER_DRAG, containerDrag);
   inspect->write(IDITG_SV_BOX_EXTRA, image.boxExtra);
   inspect->write(IDITG_SV_BOX_DIVISIONS, image.iSubDivisions);
   inspect->write(IDITG_SV_DML_FNAME, dmlFileName != NULL ? dmlFileName : "");
}

void SimVolumetric::inspectRead(Inspect *inspect)
{
   Parent::inspectRead (inspect);

   char     stringBuffer[Inspect::MAX_STRING_LEN];
   BitSet16 newFlags;
   Point3F  position;
  
   setMaskBits(Edited | Moved);

   inspect->read(IDITG_SV_POSITION, position);
   inspect->read(IDITG_SV_DIMENSIONS, image.dimensions);

   inspect->read(IDITG_SV_TEXTURE_SCALE, image.textureScale);
   inspect->read(IDITG_SV_TRANS_LEVEL, image.transLevel);
   inspect->read(IDITG_SV_ANIM_SCALE, animTimeScale);

   // check out the flags array.
   {
      inspect->read(IDITG_SV_TOGGLES, stringBuffer);
      newFlags = string_to_bits(stringBuffer);
   }
  
   inspect->read(IDITG_SV_OBJECT_TYPE, NULL, NULL, volType);
   inspect->read(IDITG_SV_CURRENT_VEL, currentVelocity);
   inspect->read(IDITG_SV_CURRENT_DRAG, currentDrag);
   inspect->read(IDITG_SV_DENSITY, density);
   inspect->read(IDITG_SV_DAMAGE, damagePerSec);
   damagePerSec = 0.0;   
   inspect->read(IDITG_SV_CONTAINER_DRAG, containerDrag);
   inspect->read(IDITG_SV_BOX_EXTRA, image.boxExtra);
   inspect->read(IDITG_SV_BOX_DIVISIONS, image.iSubDivisions);

  // dml file name.    
   inspect->read ( IDITG_SV_DML_FNAME, stringBuffer );
   {
      char    * fname = dmlFileName != NULL ? dmlFileName : "";
      if ( stricmp ( fname, stringBuffer ) )
          setFileName ( stringBuffer );
   }

   if ( image.transLevel > 1.0 )
      image.transLevel = 1.0;
   else if ( image.transLevel < 0 )
      image.transLevel = 0;
   if ( animTimeScale < 0.0 )
      animTimeScale = 0.0;
   
      
   if ( image.flagsGet () != newFlags )
   {
      image.flagClr ( BoxRenderDmlImage::ALL );
      image.flagSet ( newFlags );
   }
   
   // now handle the position
   initTransform ( position );

   syncBoxes ();
}



/*------------------------------------------------------------------------**
**                                                                        **
**            Handle events and queries                                   **
**                                                                        **
**------------------------------------------------------------------------*/



bool SimVolumetric::onSimCollisionImageQuery(SimCollisionImageQuery* /*query*/)
{
   // query->count = 1;
   // query->image[0] = &collisionImage;
   // collisionImage.transform = getTransform();

   return false;
}


bool SimVolumetric::onSimObjectTransformQuery(SimObjectTransformQuery *query)
{
   query->tmat = fetchTransform();
   
   return true;
}


bool SimVolumetric::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   // Figure out which bitmap here and set that.  

   if ( image.texturing () )
   { 
      int   numBitmaps;
      if ( bool ( numBitmaps = matList->getMaterialsCount () ) )
      {
         // I know there's a better way here:
         float   tval = (manager->getCurrentTime() * animTimeScale);
         long    round_down = (long)tval;
         tval -= round_down;
         tval *= numBitmaps;
         round_down = (long)tval % numBitmaps;
         image.setBmpIndex ( (Int16) round_down );
      }
   }
   
   // Need to set itype, and sort key.  

   query->count = 1;
   query->image[0] = &image;

   return true;
}


bool SimVolumetric::onSimImageTransformQuery ( SimImageTransformQuery* query )
{
   query->transform = fetchTransform();
   return true;
}


bool SimVolumetric::onSimObjectTransformEvent ( const SimObjectTransformEvent * event )
{
   setMaskBits ( Moved );
   
   image.transform = event->tmat;
   return true;   
}

bool SimVolumetric::onAdd()
{
    if ( ! Parent::onAdd() ) 
        return false; 

   image.manager = manager;

   AssertFatal ( manager, "SimVolumetric::onAdd no manager" );
    
   // add to root container, 
   SimContainer *root = NULL;
   
   root = findObject(manager, SimRootContainerId, root);
   AssertFatal ( root, "SimVolumetric::onAdd: no root container" );

   // if ( isGhost () )
      //  ;
       // containerMask = SimCameraObjectType;
   // else 
      containerMask = ~ ( SimMarkerObjectType     |   SimTerrainObjectType    |
                           SimDefaultObjectType    |   SimInteriorObjectType   |
                           SimMissionObjectType    |   SimShapeObjectType      |
                           SimContainerObjectType  |   SimCameraObjectType );

   //      containerMask = -1;
   // FearPlayerObjectType for debugging.
   //      containerMask = (1 << 30);       

   root->addObject ( this );

   loadDml ();
  
   syncBoxes ();
 
   setMaskBits ( Edited | Moved );
    
   return true;
}


bool SimVolumetric::onSimEditEvent(const SimEditEvent *event)
{
   // See simObjects\code\MissionEditor.cpp for posting of event.  
   if( event->editorId == SimMissionEditorId )
   {
      if( event->state )      // if mission editor being turned on
      {
         if ( mode != EDITING )
            enable_editing();
      }
      else         // editing going offline.
      {
         if ( mode == EDITING )
            enable_playing ();
     }
     return true;
  }
         
  return false;
}


#define  InsureForceApply        (1 << 7)
#define  GetPctMask              (InsureForceApply-1)
#define  OneHundredPct           GetPctMask
#define  TimeBetweenWakeups      0.50


#if   0           // OLD_TIMER_CODE
void SimVolumetric::setWakeUp ( float timeHence )
{
   SimTimerEvent::post( this, (float)manager->getCurrentTime() + timeHence );
}

bool SimVolumetric::addObject ( SimContainer * obj )
{
   if ( ! flags.test(Awake) )
   {
      flags.set ( Awake );
      setWakeUp ( 0.0 );
   }
   return ( Parent::addObject ( obj ) );
}


// Find all intersecting objects, if any, and send them messages about drag
//  and such.  If there are no objects inside, then we go back to sleep.
//
bool SimVolumetric::onSimTimerEvent ( const SimTimerEvent * /*event*/ )
{
   if ( damagePerSec  &&  !objectList.empty () )
   {
      SimContainerList::iterator     itr;
      for ( itr = objectList.begin(); itr != objectList.end(); itr++ )
      {
         SimContainer    *obj = *itr;
         int               iPct = ( obj->getContainerData() & GetPctMask );
         
         if ( iPct )
         {
            SimMovement       * mvObj = dynamic_cast<SimMovement *> (obj);

            if ( mvObj != NULL )
            {
               SimDamageEvent    dmgEvent;
               
               dmgEvent.damageValue = damagePerSec * TimeBetweenWakeups;
               dmgEvent.damageValue *= (float)iPct;
               dmgEvent.damageValue /= (float)OneHundredPct;

               dmgEvent.damageType    = 0;
               dmgEvent.collisionInfo = NULL;
               dmgEvent.origin        = getId();
               dmgEvent.mv.set(0, 0, 0);
               
               mvObj->processEvent ( &dmgEvent );
            }
         }
      }
      setWakeUp ( TimeBetweenWakeups );
   }
   else
      flags.clear ( Awake );

   return true;
}
#endif

bool SimVolumetric::processQuery(SimQuery *query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);
      onQuery(SimCollisionImageQuery);
      onQuery(SimObjectTransformQuery);
   }
   return false;
}

bool SimVolumetric::processEvent(const SimEvent *event)
{
   switch (event->type) {
      onEvent(SimObjectTransformEvent);
      // onEvent(SimTimerEvent);
      onEvent(SimEditEvent);
   }
   return false;
}


// Apply our influences for the object and leave a computed percentage in
//    the object's containerData field.  
// 
void SimVolumetric::applyInfluence ( SimMovement * mobj )
{
   float       fPercent = 1.0;
   int         newPct, oldPct = (mobj->getContainerData() & GetPctMask);
   Box3F       checkBox  = get_display_box ();
   Box3F       mobjBox = mobj->getBoundingBox ();
   bool        min_inside = checkBox.contains ( mobjBox.fMin );
   bool        max_inside = checkBox.contains ( mobjBox.fMax );
   
   if ( min_inside && max_inside )
   {
      newPct = OneHundredPct;
   }
   else if ( ! min_inside && ! max_inside )
   {
      newPct = 0;
      fPercent = 0.0;
   }
   else     // figure out percentage overlap.  rely on knowing that min or max 
   {        // is inside to keep the math down.  

      #define  CHECK_AXIS(v,a)                                                \
      {                                                                       \
         if ( mobjBox.fMin.a < checkBox.fMin.a )                              \
            v *= ((mobjBox.fMax.a - checkBox.fMin.a) / mobjBox.len_##a() );   \
         else if ( mobjBox.fMax.a > checkBox.fMax.a )                         \
            v *= ((checkBox.fMax.a - mobjBox.fMin.a) / mobjBox.len_##a() );   \
      }

      CHECK_AXIS ( fPercent, x );
      CHECK_AXIS ( fPercent, y );
      CHECK_AXIS ( fPercent, z );

      newPct = fPercent * OneHundredPct;
      AssertFatal ( newPct >= 0 && newPct <= OneHundredPct, "SimVolumetric:bad math" );
   }

   if ( newPct != oldPct || (mobj->getContainerData() & InsureForceApply) )
   {
      // we're guaranteed that the pct will be set the first time since below
      // it sets the insure flag when we enter.  
      mobj->setContainerData ( newPct );

      mobj->setContainerDrag ( containerDrag * fPercent );
      mobj->setContainerCurrent(currentVelocity * fPercent, currentDrag * fPercent);

      // Buoyancy
      if (density != 0.0f) { 
         Point3F  vec;
         if (mobj->getForce(SimMovementGravityForce,&vec)) {
            // Density * volume * gravity
	         float scaledDensity = density * fPercent;
            vec *= -scaledDensity / mobj->getDensity();
            mobj->addForce(SimMovementBuoyancyForce,vec,true);
         }
      }
   }
}


// Container database virtuals
void SimVolumetric::enterContainer(SimContainer* obj)
{
	Parent::enterContainer(obj);
   if (SimMovement* mobj = dynamic_cast<SimMovement*>(obj)) 
   {
      mobj->setContainerData ( InsureForceApply );
      applyInfluence ( mobj );
   }
}

void SimVolumetric::leaveContainer(SimContainer* obj)
{
   if (SimMovement* mobj = dynamic_cast<SimMovement*>(obj))
      mobj->removeForce(SimMovementBuoyancyForce);
	Parent::leaveContainer(obj);
}

bool SimVolumetric::updateObject ( SimContainer* obj )
{
   if (SimMovement* mobj = dynamic_cast<SimMovement*>(obj))
      applyInfluence ( mobj );


   return Parent::updateObject ( obj );
}



/*-------------------------------------------------------------------------**
**                                                                         **
**               Handle the pack and unpack updates to                     **
**                 refresh the ghosts                                      **
**                                                                         **
**-------------------------------------------------------------------------*/


DWORD SimVolumetric::packUpdate(Net::GhostManager *, DWORD mask, BitStream *stream)
{
   stream->writeBits ( NumberOfNetFlags, &mask );

   if ( mask & Moved )
   {
      Point3F  pos = fetchTransform().p;
      stream->write ( sizeof(pos), &pos );
   }

   if ( mask & Edited ) 
   {
       stream->write ( sizeof(image.dimensions), (void *) & image.dimensions );
       stream->write ( image.textureScale );
       stream->write ( image.transLevel );
       stream->write ( image.boxExtra );
       stream->write ( image.iSubDivisions );
       stream->write ( animTimeScale );
     
      // material name
      {
         char    *fname = dmlFileName != NULL ? dmlFileName : "";
         stream->writeString ( fname ); 
      }

      // toggles:
      {
         BitSet16    toggles = image.flagsGet () & BoxRenderDmlImage::ALL;
         stream->writeBits ( 6, &toggles );
      }

      // current, density, damage
      {
         stream->write ( sizeof(currentVelocity), &currentVelocity );
         stream->write ( currentDrag );
         stream->write ( containerDrag );
         stream->write ( density );
         // stream->write ( damagePerSec );
      }
   }

   return 0;
}


void SimVolumetric::unpackUpdate(Net::GhostManager *, BitStream *stream)
{
   char        stringBuffer [ Inspect::MAX_STRING_LEN ];
   BitSet16    mask = 0;

   stream->readBits ( NumberOfNetFlags, &mask );

   if ( mask & Moved )
   {
      Point3F  pos;
      stream->read ( sizeof(pos), &pos );
      initTransform ( pos );
      syncBoxes ();
   }

   if ( mask & Edited ) 
   {
      stream->read ( sizeof(image.dimensions), (void *) & image.dimensions );
      syncBoxes();
      
      stream->read ( &image.textureScale );
      stream->read ( &image.transLevel );
      stream->read ( &image.boxExtra );
      stream->read ( &image.iSubDivisions );
      stream->read ( &animTimeScale );

      // material name
      {
         stream->readString ( stringBuffer );
         setFileName ( stringBuffer );
         if ( manager != NULL )
            loadDml ();
      }

      // toggles:
      {
         Int16    toggles = 0;
         stream->readBits ( 6, &toggles );
         image.flagClr ( BoxRenderDmlImage::ALL );
         image.flagSet ( toggles );
      }
   
      // current, density, damage
      {
         stream->read ( sizeof(currentVelocity), &currentVelocity );
         stream->read ( &currentDrag );
         stream->read ( &containerDrag );
         stream->read ( &density );
         // stream->read ( &damagePerSec );
      }
   }
}


/*----------------------------------------------------------------------**
**                                                                      **
**               Persistence methods                                     **
**                                                                      **
**----------------------------------------------------------------------*/

Persistent::Base::Error SimVolumetric::write(StreamIO &sio, int a, int b)
{
   Persistent::Base::Error error = Parent::write(sio, a, b);
   if (error != Persistent::Base::Ok) 
      return error;
      
   Point3F     position = fetchTransform().p;
   
   sio.write ( sizeof(position), &position );
   sio.write ( sizeof(image.dimensions), &(image.dimensions) );
   
   sio.write ( sizeof(image.textureScale), (void *)&image.textureScale );
   sio.write ( sizeof(image.transLevel), (void *)&image.transLevel );
   sio.write ( sizeof(animTimeScale), (void *)&animTimeScale );
   
   // fields which affect objects inside:
   sio.write ( sizeof(damagePerSec), (void *) &damagePerSec );
   sio.write ( sizeof(currentVelocity), (void *) &currentVelocity );
   sio.write ( sizeof(currentDrag), (void *) &currentDrag );
   sio.write ( sizeof(containerDrag), (void *) &containerDrag );
   sio.write ( sizeof(density), (void *) &density );
   sio.write ( sizeof(image.boxExtra), (void *) &image.boxExtra );
   sio.write ( sizeof(image.iSubDivisions), (void *)&image.iSubDivisions );

   sio.writeString ( dmlFileName );

   // toggles
   {
      UInt16   toggles = image.flagsGet () & BoxRenderDmlImage::ALL;
      sio.write ( sizeof(toggles), (void *) &toggles );
   }

   sio.write ( sizeof(volType), (void *) &volType );

   return (sio.getStatus() == STRM_OK) ? Ok : WriteError;
}

Persistent::Base::Error SimVolumetric::read(StreamIO &sio, int a, int b)
{
   Persistent::Base::Error error = Parent::read(sio, a, b);
   if (error != Persistent::Base::Ok) 
      return error;
   

   // handle the transform matrix and size.  
   {    
      Point3F     position;

      sio.read ( sizeof(position), &position );
      sio.read ( sizeof(image.dimensions), &(image.dimensions) );
      
      initTransform ( position );

      // m.set ( nulRot, position );
      // processEvent ( &SimObjectTransformEvent(m) );
   }

   sio.read ( sizeof(image.textureScale), (void *)&image.textureScale );
   sio.read ( sizeof(image.transLevel), (void *)&image.transLevel );
   
   sio.read ( sizeof(animTimeScale), (void *)&animTimeScale );
   
   // fields which affect objects:
   sio.read ( sizeof(damagePerSec), (void *) &damagePerSec );
   sio.read ( sizeof(currentVelocity), (void *) &currentVelocity );
   sio.read ( sizeof(currentDrag), (void *) &currentDrag );
   sio.read ( sizeof(containerDrag), (void *) &containerDrag );
   sio.read ( sizeof(density), (void *) &density );
   sio.read ( sizeof(image.boxExtra), (void *) &image.boxExtra );
   sio.read ( sizeof(image.iSubDivisions), (void *)&image.iSubDivisions );

   if (dmlFileName == NULL)
      dmlFileName = new char[256];
   sio.readString ( dmlFileName );     // should clear if none.  

   // toggles
   {
      UInt16   toggles;
      sio.read ( sizeof(toggles), (void *) &toggles );
      image.flagClr ( BoxRenderDmlImage::ALL );
      image.flagSet ( toggles );
   }
   
   sio.read ( sizeof(volType), (void *) &volType );

   return (sio.getStatus() == STRM_OK) ? Ok : ReadError;
}


/*
    features for the volumetrics to support:
        client only:
            bubbles
            palette changes when camera is inside 
        server
            damage
            projectiles behave differently, as well as player(s).  
            apply forces - slow down, wind, etc.  
            complex polytopes - optional feature.  

    do list:  
        - collision queries and events.
        - insure art is mip-mapped
        - send less data on packUpdate / unpackUpdate.  = faster startup.

        Polytopes are lo-pri.  One idea is to use ZED shapes to get a BSP to do volume 
            checking, and maybe even displays - put a translucent bmp on them.  

    notes:  
        the only containers in the game:  terrain, interior, 
               XY spacial database.  
*/



