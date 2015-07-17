//------------------------------------------------------------------------------
//
// simMapView.cpp
//
//------------------------------------------------------------------------------

#include <sim.h>
#include <ts.h>
#include <g_bitmap.h>
#include <grdFile.h>
#include <simTerrain.h>
#include "SimMapCanvas.h"
#include "g_surfac.h"

//------------------------------------------------------------------------------

#define VN_FARDIST   "SimMapView::FarDist"
#define VN_ZDIST     "SimMapView::ZDist"
#define VN_ORTHO     "SimMapView::OrthoView"

#define Min(x,y) ((x) < (y) ? (x) : (y))
#define Max(x,y) ((x) > (y) ? (x) : (y))
#define Abs(x)   ((x) > 0 ? (x) : (-(x)))

//------------------------------------------------------------------------------

void SimMapCanvas::mapOverlay()
{
   if (mouseState == LDown)
      pCanvas->getSurface()->drawRect2d(&mouseRect, 255);
}

void SimMapCanvas::render()
{
   if (refreshCt)
   {
      // refresh terrain image buffer if necessary
      Parent::render();
      GFXBitmap bitmap;
      pCanvas->getSurface()->getBitmap(&bitmap);
      if ((imageBuffer->getWidth() != bitmap.getWidth()) ||
          (imageBuffer->getHeight() != bitmap.getHeight()))
      {
         delete imageBuffer;
         imageBuffer = GFXBitmap::create(bitmap.getWidth(), bitmap.getHeight());
      }

      refreshCt = 0;
      memcpy(imageBuffer->pBits, bitmap.pBits, 
         bitmap.getStride()*bitmap.getHeight());
   }
   else
      pCanvas->getSurface()->drawBitmap2d(imageBuffer, &Point2I(0, 0));

   mapOverlay();
}

//------------------------------------------------------------------------------

bool SimMapCanvas::processEvent(const SimEvent *event)
{
	switch (event->type) 
	{
		case SimAddNotifyEventType:
         init();
         break;
         
      default: 
         break;
   }
   return Parent::processEvent(event);
}

//------------------------------------------------------------------------------

bool SimMapCanvas::processQuery(SimQuery *query)
{
   switch (query->type)
   {
		case SimCameraQueryType: 
		{
         CMDConsole *cp = CMDConsole::getLocked();
         float zDist     = cp->getFloatVariable(VN_ZDIST);
         float farPlane  = cp->getFloatVariable(VN_FARDIST);
         bool  orthoView = cp->getBoolVariable(VN_ORTHO);

         RectF wr = worldRect.last();
         Point3F position((wr.upperL.x + wr.lowerR.x)/2.0,
                          (wr.upperL.y + wr.lowerR.y)/2.0, zDist);
         TMat3F m;
         m.identity();
         m.postTranslate(position);
         m.preRotateX(-90 * float(M_PI/180.0));

			SimCameraQueryInfo* qp = static_cast<SimCameraQueryInfo*>(query);
			qp->cameraInfo.fov            = 0.6f;
			qp->cameraInfo.nearPlane      = 1.0f;
			qp->cameraInfo.farPlane       = farPlane;
         qp->cameraInfo.tmat           = m;

      	if (orthoView)
         {
            Point2F vpDim((wr.lowerR.x - wr.upperL.x)/2.0,
                                (wr.upperL.y - wr.lowerR.y)/2.0);
            RectF viewport(-vpDim.x, vpDim.y, vpDim.x, -vpDim.y);
            qp->cameraInfo.projectionType = SimOrthographicProjection;
            qp->cameraInfo.worldBounds = viewport;
         }
         else
            qp->cameraInfo.projectionType = SimPerspectiveProjection;

			return true;
		}
   }
   return false;
}

//------------------------------------------------------------------------------

void SimMapCanvas::init()
{
   // find out how big the world is (in world coords obviously)
	SimTerrain *terrain = dynamic_cast<SimTerrain *>(manager->findObject(SimTerrainId));
   GridFile *pGridFile = terrain->getGridFile();

   // find out how big one grid point is
   int gridPointSize = (1 << pGridFile->getScale());
   
   GridBlock *gridBlock;
   Point2I    gridSize = pGridFile->getSize();
   worldSize = Point2F(0.0, 0.0);
   for (int row = 0; row < gridSize.y; row++) {
      gridBlock = pGridFile->getBlock(Point2I(0, row));
      worldSize.y += gridPointSize*(gridBlock->getSize().y);
   }
   for (int col = 0; col < gridSize.x; col++) {
      gridBlock = pGridFile->getBlock(Point2I(col, 0));
      worldSize.x += gridPointSize*(gridBlock->getSize().x);
   }     

   // make the viewport big enough to fit the entire world
   RectF   worldVP;
   Point3F upperL(0.0f, worldSize.y, 0.0f), upperL0;
   Point3F lowerR(worldSize.x, 0.0f, 0.0f), lowerR0;
   SimObjectTransformQuery q;
   if (terrain->processQuery(&q)) {
      // transform viewport from object to world coords
      m_mul(upperL, q.tmat, &upperL0);
      m_mul(lowerR, q.tmat, &lowerR0);
   }
   else {
      upperL0 = upperL;
      lowerR0 = lowerR;
   }
   worldVP.upperL = Point2F(upperL0.x, upperL0.y);
   worldVP.lowerR = Point2F(lowerR0.x, lowerR0.y);
   worldRect.push_back(worldVP);

   // setup some console vars if they don't exist yet
   CMDConsole *cp = CMDConsole::getLocked();
   if (strcmp(cp->getVariable(VN_FARDIST), "") == 0)
      cp->setVariable(VN_FARDIST, "100000.0");
   if (strcmp(cp->getVariable(VN_ZDIST), "") == 0)
      cp->setVariable(VN_ZDIST, "4096.0");
   if (strcmp(cp->getVariable(VN_ORTHO), "") == 0)
      cp->setVariable(VN_ORTHO, "true");

   Point2I imageSize = pCanvas->getClientSize();
   imageBuffer = GFXBitmap::create(imageSize.x, imageSize.y);

   refreshCt++;
}

//------------------------------------------------------------------------------

SimMapCanvas::~SimMapCanvas() 
{
   if (imageBuffer)
      delete imageBuffer;
}

SimMapCanvas::SimMapCanvas() 
{
   imageBuffer = NULL;
   mouseState  = Idle;
   refreshCt   = 0;
}

//------------------------------------------------------------------------------

void SimMapCanvas::onMouseMove(int x, int y, UINT keyFlags)
{
   keyFlags;
   if (mouseState == LDown) {
      float dx = x - mouseRect.upperL.x;
      float dy = y - mouseRect.upperL.y, d;
      if (Abs(dx) > Abs(dy)) d = dx;
      else d = dy;
      mouseRect.lowerR.x = mouseRect.upperL.x + d;
      mouseRect.lowerR.y = mouseRect.upperL.y + d;
   }
}

//------------------------------------------------------------------------------

void SimMapCanvas::onLButtonDown(BOOL, int x, int y, UINT)
{
   mouseState = LDown;
   mouseRect = RectI(x, y, x, y);
}

void SimMapCanvas::onLButtonUp(int, int, UINT)
{
   if (mouseState != LDown)
      return;

   if (mouseRect.upperL == mouseRect.lowerR) {
      mouseState = Idle;
      return;
   }

   mouseState = Idle;
   RectF wvp(worldPosition(mouseRect.upperL), 
             worldPosition(mouseRect.lowerR));
   wvp.upperL.x = Min(wvp.upperL.x, wvp.lowerR.x);
   wvp.lowerR.x = Max(wvp.upperL.x, wvp.lowerR.x);
   wvp.upperL.y = Max(wvp.upperL.y, wvp.lowerR.y);
   wvp.lowerR.y = Min(wvp.upperL.y, wvp.lowerR.y);
   worldRect.push_back(wvp);
   refreshCt++;
}

//------------------------------------------------------------------------------

void SimMapCanvas::onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   fDoubleClick, x, y, keyFlags;
}

void SimMapCanvas::onRButtonUp(int x, int y, UINT flags)
{
   x, y, flags;
   if (worldRect.size() > 1)
      worldRect.pop_back();
   refreshCt++;
}

//------------------------------------------------------------------------------

Point2F SimMapCanvas::worldPosition(Point2I p)
{
   Point2I screenSz = pCanvas->getClientSize();
   // flip y screen coords 
   p.y = (screenSz.y - 1) - p.y;
   RectF wvp = worldRect.last();
   Point2F wp;
   wp.x = wvp.upperL.x + ((float)p.x/screenSz.x)*(wvp.lowerR.x - wvp.upperL.x);
   wp.y = wvp.lowerR.y + ((float)p.y/screenSz.y)*(wvp.upperL.y - wvp.lowerR.y);
   return wp;
}

Point2I SimMapCanvas::screenPosition(const Point2F p)
{
   Point2I screenSz = pCanvas->getClientSize();
   RectF wvp = worldRect.last();
   Point2F fp;
   fp.x = (float)screenSz.x*((p.x - wvp.upperL.x)/(wvp.lowerR.x - wvp.upperL.x));
   fp.y = (float)screenSz.y*((p.y - wvp.lowerR.y)/(wvp.upperL.y - wvp.lowerR.y));
   Point2I sp(fp.x, fp.y);
   // flip y screen coords 
   sp.y = (screenSz.y - 1) - sp.y;
   return sp;
}

//------------------------------------------------------------------------------

