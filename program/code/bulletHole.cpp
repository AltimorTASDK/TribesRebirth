//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <g_surfac.h>
#include "bulletHole.h"
#include "simResource.h"
#include <ts.h>
#include <simterrain.h>

const char* BulletHole::sm_pBulletHolePrefString = "pref::Effects::BulletHoleTimeout";
float BulletHole::sm_bulletHoleTimeout           = 30.0f;

const Point2F BulletHole::sm_pTextureCoords[] = {
 Point2F(0, 0), Point2F(1, 0), Point2F(0.5, 1)
};

struct BulletHoleInfo {
   const char* pBmpName;
   float       holeSize;
};

BulletHoleInfo g_bulletHoleInfo[] = {
   { "bullethole.bmp", 0.15 }
};


BulletHole::BulletHole(const Point3F& in_rPoint,
                       const Point3F& in_rNormal,
                       const UInt8    in_bulletHoleIndex)
 : m_timeOutTime(getBulletHoleTimeout()),
   m_bulletHoleIndex(in_bulletHoleIndex)
{
   m_renderImage.buildPoints(in_rPoint, in_rNormal, in_bulletHoleIndex);
}

bool
BulletHole::onAdd()
{
   if (Parent::onAdd() == false)
      return false;
   
   m_timeOutTime += manager->getCurrentTime();
   
   // Load the resource for the bitmap...
   //
   ResourceManager* rm = SimResource::get(manager);
   m_resBmp = rm->load(g_bulletHoleInfo[m_bulletHoleIndex].pBmpName, true);
   if (bool(m_resBmp) != true) {
      AssertWarn(0, "Couldn't load bullethole bitmap...");
      return false;
   }
   
   Box3F bbox(m_renderImage.centerPoint,
              m_renderImage.centerPoint);
   float factor = g_bulletHoleInfo[m_bulletHoleIndex].holeSize;
   bbox.fMin -= Point3F(factor, factor, factor);
   bbox.fMax += Point3F(factor, factor, factor);
   setBoundingBox(bbox);

   AssertFatal(getContainer() == NULL, "Cannot be in a container at this point...");
   SimContainer* pRoot = findObject(manager, SimRootContainerId, pRoot);
   AssertFatal(pRoot != NULL, "No root container...");
   pRoot->addObject(this);

   addToSet(SimTimerSetId);

   SimTerrain *terrain = static_cast<SimTerrain *>
                         (manager->findObject(SimTerrainId));

   m_renderImage.rTerrainPerspectiveDistance = terrain->getPerspectiveDistance();
   return true;
}


bool
BulletHole::processQuery(SimQuery* query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);

     default:
      return Parent::processQuery(query);
   }
}


bool
BulletHole::processEvent(const SimEvent* event)
{
   switch (event->type) {
      onEvent(SimTimerEvent);

     default:
      return Parent::processEvent(event);
   }
}


bool
BulletHole::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   query->count    = 1;
   query->image[0] = &m_renderImage;

   m_renderImage.pBitmap = m_resBmp;
   
   return true;
}


bool
BulletHole::onSimTimerEvent(const SimTimerEvent*)
{
   if (manager->getCurrentTime() > m_timeOutTime)
      deleteObject();

   return true;
}


//------------------------------------------------------------------------------
//--------------------------------------
// RenderImage...
//--------------------------------------
//
void
BulletHole::BulletHoleRenderImage::buildPoints(const Point3F& in_rCenter,
                                               const Point3F& in_rNormal,
                                               const UInt8    in_holeIndex)
{
   centerPoint = in_rCenter;
   normal      = in_rNormal;

   Point3F perpVec1;
   Point3F perpVec2;
   Point3F mCVec(0, 0, 1);
   if (in_rNormal.z == 1.0f ||
       in_rNormal.z == -1.0f)
      mCVec = Point3F(1, 0, 0);
   m_cross(mCVec,    in_rNormal, &perpVec1);
   m_cross(perpVec1, in_rNormal, &perpVec2);
   
   Point3F randPVec1, randPVec2;

   static Random s_random;
   float randAngle = s_random.getFloat() * M_PI;

   randPVec1 = perpVec1 * float(sin(randAngle)) + perpVec2 * cos(randAngle);
   randPVec2 = perpVec2 * sin(M_PI - randAngle) + perpVec1 * cos(M_PI - randAngle);

   AssertFatal(fabs(m_dot(randPVec1, randPVec2)) < 0.01, "Not perp?");

   perpVec1 = randPVec1;
   perpVec2 = randPVec2;

   float holeSize = g_bulletHoleInfo[in_holeIndex].holeSize;

   // Ok, now we have a set of orthonormal basic vectors, build a 2 unit tri just a bit
   //  of the surface...
   //
   worldPts[0] = in_rCenter + (in_rNormal * 0.025) + (perpVec1 * holeSize);
   worldPts[1] = in_rCenter + (in_rNormal * 0.025) - (perpVec1 * holeSize) + (perpVec2 * holeSize);
   worldPts[2] = in_rCenter + (in_rNormal * 0.025) - (perpVec1 * holeSize) - (perpVec2 * holeSize);
}


void
BulletHole::BulletHoleRenderImage::render(TSRenderContext& rc)
{
   Vector3F distance  = centerPoint - rc.getCamera()->getTCW().p;
   float    rDistance = distance.lenf();

   // A kludge to fix the terrain-growing, floating-bullethole 
   // problem.  If the distance between the camera and the
   // bullethole is greater than the terrain's perspective
   // distance, don't draw the bullet hole.
   if (rDistance < rTerrainPerspectiveDistance)
   {
      GFXSurface *gfxSurface = rc.getSurface();
      gfxSurface->setHazeSource(GFX_HAZE_NONE);
      gfxSurface->setShadeSource(GFX_SHADE_CONSTANT);
      gfxSurface->setConstantShade(1.0f);
      gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
	   gfxSurface->setFillMode(GFX_FILL_TEXTURE);
	   gfxSurface->setFillColor(Int32(255));
	   gfxSurface->setTransparency(true);
      gfxSurface->setZTest(GFX_ZTEST);
      gfxSurface->setTextureMap(pBitmap);

      TS::PointArray *pointArray = rc.getPointArray();
      pointArray->useIntensities(false);
      pointArray->useTextures(sm_pTextureCoords);
      pointArray->useTextures(true);
	   pointArray->setVisibility(TS::ClipMask);
      pointArray->useHazes(false);

      pointArray->reset();
      int offset = pointArray->addPoints(3, worldPts);

      TS::VertexIndexPair pPolyVerts[3];
      pPolyVerts[0].fVertexIndex  = offset + 0;
      pPolyVerts[0].fTextureIndex = 0;
      pPolyVerts[1].fVertexIndex  = offset + 1;
      pPolyVerts[1].fTextureIndex = 1;
      pPolyVerts[2].fVertexIndex  = offset + 2;
      pPolyVerts[2].fTextureIndex = 2;
      pointArray->drawPoly(3, pPolyVerts, 0);
      pPolyVerts[0].fVertexIndex = offset + 2;
      pPolyVerts[1].fVertexIndex = offset + 1;
      pPolyVerts[2].fVertexIndex = offset + 0;
      pointArray->drawPoly(3, pPolyVerts, 0);
      gfxSurface->setZTest(GFX_ZTEST_AND_WRITE);
      pointArray->reset();
   }
}

void
BulletHole::BulletHoleRenderImage::setSortValue(TSCamera* io_pCamera)
{
   Point3F pCam = centerPoint;
   float nd = io_pCamera->getNearDist();

   sortValue = (pCam.y <= nd) ? 0.99f : (nd / pCam.y);
}

