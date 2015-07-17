//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <ts_pointArray.h>
#include <g_surfac.h>
#include <simResource.h>

#include "projAttTrail.h"

#define ALPHAPAL_INDEX (1976)

SwooshTrail::SwooshTrail(const float  in_totalLength,
                         const UInt32 in_numSections,
                         const float  in_initialTrailWidth)
 : m_totalLength(in_totalLength),
   m_numSections(in_numSections),
   m_lengthTrigger(m_totalLength / float(m_numSections)),
   m_initialWidth(in_initialTrailWidth),
   m_numActiveSections(0),
   m_rendering(false)
{
   m_pointChain = new PointBar[in_numSections + 1];
   memset(m_pointChain, 0xcd, sizeof(PointBar) * (in_numSections + 1));
   
   itype = SimRenderImage::Translucent;
}

SwooshTrail::~SwooshTrail()
{
   delete [] m_pointChain;
   m_pointChain      = NULL;
   m_numActiveSections = 0;
}

//--------------------------------------  This is pretty slow, maybe update?
//
void
SwooshTrail::updateTransform(const TMat3F& in_newTransform,
                             const float   /*in_newTime*/)
{
   if (m_numActiveSections != 0) {
      m_rendering = true;
      
      // First, determine the length of the new first section based on the new point.
      //
      Point3F firstSectionVec = in_newTransform.p - m_pointChain[1].centerPoint;
      float   firstSectionLen = firstSectionVec.len();
      
      if (firstSectionLen <= m_lengthTrigger) {
         // Section is within the allowed boundaries, we just need to update the first and
         //  last points, and readjust the widths.  See below for comments on this
         //  process...
      
         // Update the first point...
         //
         PointBar& rBarFront = m_pointChain[0];
         rBarFront.centerPoint = in_newTransform.p;
         const RMat3F& rMat = static_cast<const RMat3F&>(in_newTransform);
         m_mul(Point3F(1, 0, 0), rMat, &rBarFront.unitLeft);
         rBarFront.leftPoint  = rBarFront.centerPoint + (rBarFront.unitLeft * m_initialWidth);
         rBarFront.rightPoint = rBarFront.centerPoint - (rBarFront.unitLeft * m_initialWidth);
         
         // Update the widths...
         //
         Point3F* pBeginPoint = &rBarFront.centerPoint;
         float    totalLength = 0.0f;
         for (UInt32 i = 1; i <= m_numActiveSections - 1; i++) {
            PointBar& rCurrent = m_pointChain[i];
            Point3F& rEndPoint = m_pointChain[i].centerPoint;
            totalLength       += (rEndPoint - *pBeginPoint).len();
            
            float width = (1.0f - (totalLength / m_totalLength)) * m_initialWidth;
            rCurrent.leftPoint  = rCurrent.centerPoint + (rCurrent.unitLeft * width);
            rCurrent.rightPoint = rCurrent.centerPoint - (rCurrent.unitLeft * width);
            
            pBeginPoint = &rEndPoint;
         }

         if (m_numActiveSections > 1) {
            // Update the last point.  (It's the penultimate supper!)
            //
            PointBar& rBarPenultimate = m_pointChain[m_numActiveSections - 1];
            PointBar& rBarBack        = m_pointChain[m_numActiveSections];
         
            Point3F diffVec = rBarBack.centerPoint - rBarPenultimate.centerPoint;
            float diffLen = diffVec.len();
            float targetLen = (m_lengthTrigger * (m_numActiveSections - 1)) - totalLength;
            diffVec *= targetLen / diffLen;
            rBarBack.centerPoint = rBarPenultimate.centerPoint + diffVec;
         }
      } else {
         // First section exceeded the length restrictions, we need to update all the
         //  sections.
         
         // First, slide the buffer back...
         //
         PointBar* pTemp = new PointBar[m_numSections + 1];
         if (m_numActiveSections < m_numSections) {
            memcpy(pTemp, m_pointChain, sizeof(PointBar) * (m_numActiveSections + 1));
            memcpy(&m_pointChain[1], pTemp, sizeof(PointBar) * (m_numActiveSections + 1));
            m_numActiveSections += 1;
         } else {
            memcpy(pTemp, m_pointChain, sizeof(PointBar) * (m_numActiveSections));
            memcpy(&m_pointChain[1], pTemp, sizeof(PointBar) * (m_numActiveSections));
            AssertFatal(m_numActiveSections <= m_numSections, "over flow");
         }
         delete [] pTemp;
         
         // Create a new bar for the front...
         //
         PointBar& rBarFront = m_pointChain[0];
         rBarFront.centerPoint = in_newTransform.p;
         const RMat3F& rMat = static_cast<const RMat3F&>(in_newTransform);
         m_mul(Point3F(1, 0, 0), rMat, &rBarFront.unitLeft);
         rBarFront.leftPoint  = rBarFront.centerPoint + (rBarFront.unitLeft * m_initialWidth);
         rBarFront.rightPoint = rBarFront.centerPoint - (rBarFront.unitLeft * m_initialWidth);
         
//         // Readjust the length of the second section...
//         //
//         PointBar& rBarSecond = m_pointChain[1];
//         PointBar& rBarThird  = m_pointChain[2];
//         Point3F diffVec = rBarSecond.centerPoint - rBarThird.centerPoint;
//         float diffLen = diffVec.len();
//         diffVec *= (m_lengthTrigger / diffLen);
//         rBarSecond.centerPoint = rBarThird.centerPoint + diffVec;

         // Pinch off the (possibly new) back end...
         //
         m_pointChain[m_numActiveSections].leftPoint  = m_pointChain[m_numActiveSections].centerPoint;
         m_pointChain[m_numActiveSections].rightPoint = m_pointChain[m_numActiveSections].centerPoint;
         
         // And readjust the widths as above...
         //
         Point3F* pBeginPoint = &rBarFront.centerPoint;
         float    totalLength = 0.0f;
         for (UInt32 i = 1; i <= m_numActiveSections - 1; i++) {
            PointBar& rCurrent = m_pointChain[i];
            Point3F& rEndPoint = m_pointChain[i].centerPoint;
            totalLength       += (rEndPoint - *pBeginPoint).len();
            
            float width = (1.0f - (totalLength / m_totalLength)) * m_initialWidth;
            rCurrent.leftPoint  = rCurrent.centerPoint + (rCurrent.unitLeft * width);
            rCurrent.rightPoint = rCurrent.centerPoint - (rCurrent.unitLeft * width);
            
            pBeginPoint = &rEndPoint;
         }
      }
   } else {
      // This is the first update.  We need to set both the first and second sections to
      //  the same value, leaving m_rendering false.
      //
      PointBar& rBarFront = m_pointChain[0];
      PointBar& rBarBack  = m_pointChain[1];
      
      rBarFront.centerPoint = in_newTransform.p;
      
      // The projectile will be pointing down the positive Y axis in object space...
      //
      const RMat3F& rMat = static_cast<const RMat3F&>(in_newTransform);
      m_mul(Point3F(1, 0, 0), rMat, &rBarFront.unitLeft);
      
      rBarFront.leftPoint  = rBarFront.centerPoint + (rBarFront.unitLeft * m_initialWidth);
      rBarFront.rightPoint = rBarFront.centerPoint - (rBarFront.unitLeft * m_initialWidth);

      rBarBack.centerPoint = rBarFront.centerPoint;
      rBarBack.leftPoint   = rBarFront.centerPoint;
      rBarBack.rightPoint  = rBarFront.centerPoint;
      rBarBack.unitLeft    = rBarFront.unitLeft;
      
      m_numActiveSections = 1;
   }
}

void
SwooshTrail::setSortValue(TSCamera* io_pCamera)
{
   // This call is only important if the object is translucent...
   //
   if (itype == SimRenderImage::Translucent &&
       (m_numActiveSections != 0 &&
        m_rendering == true)) {
      // Set the sort value to the back point of the chain...
      //
      PointBar& rBar = m_pointChain[m_numActiveSections];
      Point3F pCam;
      float nd = io_pCamera->getNearDist();
      m_mul(rBar.centerPoint, io_pCamera->getTWC(), &pCam);
      sortValue = (pCam.y <= nd) ? 0.99f : nd/pCam.y;
   } else {
		sortValue = 0.0f;
   }
}

bool
SwooshTrail::getBoundingBox(Box3F& /*out_rBox*/)
{
   // The attached trails are designed to be embeddeded in another object, and returned
   //  from _its_ renderQuery, so we need to make sure to account for the size of the
   //  trail in that objects bounding box.  if the trail has not yet been initialized,
   //  this function returns false.
   //
   return false;
}

void
SwooshTrail::render(TS::RenderContext& io_rRC)
{
   if (m_rendering == false)
      return;
   
   TS::PointArray* pArray = io_rRC.getPointArray();
   pArray->reset();

   GFXSurface* pSurface = io_rRC.getSurface();
   
   for (UInt32 i = 0; i < m_numActiveSections; i++) {
      PointBar& rBar = m_pointChain[i];
      
      // Enter the points into the pointArray...
      pArray->addPoint(rBar.leftPoint);
      pArray->addPoint(rBar.rightPoint);
   }
   // And the tail...
   pArray->addPoint(m_pointChain[m_numActiveSections].centerPoint);
   
   // Set up the rendering context...
   pArray->useTextures(false);
   pArray->useHazes(false);
   pArray->useIntensities(false);
   pArray->useBackFaceTest(false);
   
	pSurface->setFillMode(GFX_FILL_CONSTANT);
   pSurface->setHazeSource(GFX_HAZE_NONE);
   pSurface->setShadeSource(GFX_SHADE_NONE);
   pSurface->setAlphaSource(GFX_ALPHA_FILL);
	pSurface->setFillColor(Int32(253), ALPHAPAL_INDEX);
	pSurface->setTransparency(false);

   // And draw the polys...
   int temp;
   TS::VertexIndexPair ilist[3];
   for (UInt32 j = 0; j < (m_numActiveSections - 1); j++) {
      ilist[0].fVertexIndex = (j * 2);
      ilist[1].fVertexIndex = (j * 2) + 1;
      ilist[2].fVertexIndex = ((j + 1) * 2);
      pArray->drawPoly(3, ilist, 0);
      temp = ilist[0].fVertexIndex;
      ilist[0].fVertexIndex = ilist[2].fVertexIndex;
      ilist[2].fVertexIndex = temp;
      pArray->drawPoly(3, ilist, 0);
      
      ilist[0].fVertexIndex = (j * 2) + 1;
      ilist[1].fVertexIndex = ((j + 1) * 2);
      ilist[2].fVertexIndex = ((j + 1) * 2) + 1;
      pArray->drawPoly(3, ilist, 0);
      temp = ilist[0].fVertexIndex;
      ilist[0].fVertexIndex = ilist[2].fVertexIndex;
      ilist[2].fVertexIndex = temp;
      pArray->drawPoly(3, ilist, 0);
   }
   // And the tail poly...
   ilist[0].fVertexIndex = ((m_numActiveSections - 1) * 2);
   ilist[1].fVertexIndex = ((m_numActiveSections - 1) * 2) + 1;
   ilist[2].fVertexIndex = ((m_numActiveSections) * 2);
   pArray->drawPoly(3, ilist, 0);
   temp = ilist[0].fVertexIndex;
   ilist[0].fVertexIndex = ilist[2].fVertexIndex;
   ilist[2].fVertexIndex = temp;
   pArray->drawPoly(3, ilist, 0);
   pSurface->setAlphaSource(GFX_ALPHA_NONE);
}


SimpleSwooshTrail::SimpleSwooshTrail(const float in_length,
                                     const float in_trailWidth)
 : m_totalLength(in_length),
   m_initialWidth(in_trailWidth),
   m_set(false),
   m_rendering(false)
{
   itype = SimRenderImage::Translucent;
}

void
SimpleSwooshTrail::setResources(Resource<GFXBitmap>& rBitmap)
{
   AssertFatal(bool(rBitmap) == true, "Error, bogus resource passed");
   m_hBitmap = rBitmap;
}

void
SimpleSwooshTrail::loadResources(const char* in_pTextureName,
                                 SimManager* io_pManager)
{
   AssertFatal(io_pManager != NULL, "Must have manager to load resources");

   ResourceManager* rm = SimResource::get(io_pManager);
   AssertFatal(rm != NULL, "No resource manager in manager?");

   m_hBitmap = rm->load(in_pTextureName, true);
   AssertFatal(bool(m_hBitmap) == true, avar("Unable to load texture: %s", in_pTextureName));
}

SimpleSwooshTrail::~SimpleSwooshTrail()
{
   //
}

void
SimpleSwooshTrail::render(TS::RenderContext& io_rRC)
{
   if (m_rendering == true) {
      TS::PointArray* pArray = io_rRC.getPointArray();
      GFXSurface* pSurface   = io_rRC.getSurface();
      pArray->reset();
      
      float yVal = (m_lastTime * 5) - float(int(m_lastTime * 5));

      Point2F texturePoints[4];
      texturePoints[0].x = 1.0;
      texturePoints[0].y = (0.0 + yVal) / 2.0f;
      texturePoints[1].x = 0.0;
      texturePoints[1].y = (0.0 + yVal) / 2.0f;
      texturePoints[2].x = 0.0;
      texturePoints[2].y = (1.0 + yVal) / 2.0f;
      texturePoints[3].x = 1.0;
      texturePoints[3].y = (1.0 + yVal) / 2.0f;

      // Set up the rendering context...
      pArray->useTextures(true);
      pArray->useTextures(texturePoints);
      pArray->useHazes(false);
      pArray->useIntensities(false);
      pArray->useBackFaceTest(false);
      
   	pSurface->setFillMode(GFX_FILL_TEXTURE);
      pSurface->setHazeSource(GFX_HAZE_NONE);
      pSurface->setShadeSource(GFX_SHADE_NONE);
      pSurface->setAlphaSource(GFX_ALPHA_ADD);
      pSurface->setTextureMap(m_hBitmap);
   	pSurface->setTransparency(false);
      pSurface->setTexturePerspective(false);

      // Emit the points...
      //
      pArray->addPoint(m_rearPoint + (m_unitLeft * (m_initialWidth / 6.0f)));
      pArray->addPoint(m_rearPoint - (m_unitLeft * (m_initialWidth / 6.0f)));
      pArray->addPoint(m_centerPoint - (m_unitLeft * (m_initialWidth / 2.0f)));
      pArray->addPoint(m_centerPoint + (m_unitLeft * (m_initialWidth / 2.0f)));

      // Emit the polys...
      //
      TS::VertexIndexPair ilist[4];
      ilist[0].fVertexIndex  = 0;
      ilist[0].fTextureIndex = 0;
      ilist[1].fVertexIndex  = 1;
      ilist[1].fTextureIndex = 1;
      ilist[2].fVertexIndex  = 2;
      ilist[2].fTextureIndex = 2;
      ilist[3].fVertexIndex  = 3;
      ilist[3].fTextureIndex = 3;
      pArray->drawPoly(4, ilist, 0);
      ilist[0].fVertexIndex  = 0;
      ilist[0].fTextureIndex = 0;
      ilist[1].fVertexIndex  = 3;
      ilist[1].fTextureIndex = 3;
      ilist[2].fVertexIndex  = 2;
      ilist[2].fTextureIndex = 2;
      ilist[3].fVertexIndex  = 1;
      ilist[3].fTextureIndex = 1;
      pArray->drawPoly(4, ilist, 0);
   }
}

void
SimpleSwooshTrail::updateTransform(const TMat3F& in_newTransform,
                                   const float   in_newTime)
{
   m_lastTime = in_newTime;

   if (m_set == true) {
      m_rendering = true;

      m_centerPoint = in_newTransform.p;
      const RMat3F& rMat = static_cast<const RMat3F&>(in_newTransform);
      m_mul(Point3F(1.0, 0, 0), rMat, &m_unitLeft);
      
      Point3F diffVec = (m_rearPoint - m_centerPoint);
      float newLen    = diffVec.len();
      
      if (newLen > m_totalLength) {
         diffVec *= m_totalLength / newLen;
         m_rearPoint = m_centerPoint + diffVec;
      }
   } else {
      m_centerPoint = in_newTransform.p;
      m_rearPoint   = m_centerPoint;
      
      const RMat3F& rMat = static_cast<const RMat3F&>(in_newTransform);
      m_mul(Point3F(1.0, 0, 0), rMat, &m_unitLeft);
      
      m_set = true;
   }
}

void
SimpleSwooshTrail::setSortValue(TSCamera* io_pCamera)
{
   // This call is only important if the object is translucent...
   //
   if (itype == SimRenderImage::Translucent &&
       m_rendering == true) {
      // Set the sort value to the back point of the chain...
      //
      Point3F pCam;
      float nd = io_pCamera->getNearDist();
      m_mul(m_rearPoint, io_pCamera->getTWC(), &pCam);
      sortValue = (pCam.y <= nd) ? 0.99f : nd/pCam.y;
   } else {
		sortValue = 0.0f;
   }
}

bool
SimpleSwooshTrail::getBoundingBox(Box3F& /*out_rBox*/)
{
   return false;
}

