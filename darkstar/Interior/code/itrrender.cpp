//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#include <g_surfac.h>
#include <g_bitmap.h>
#include <ts.h>
#include "itrmetrics.h"
#include "itrgeometry.h"
#include "itrlighting.h"
#include "itrinstance.h"
#include "itrrender.h"
#include "gOGLSfc.h"
#include "gOGLTx.h"
#include "m_base.h"

//----------------------------------------------------------------------------
ITRBitVector ITRRender::surfaceSet;
ITRBitVector ITRRender::planeSet;
ITRBitVector ITRRender::pointSet;

UInt32 ITRRender::PrefInteriorTextureDetail = 0;   // 0 = highest -> 8 == lowest

namespace OpenGL {

void
externCheckCache(OpenGL::Surface* pSurface);

}


namespace {

int getPower(int x)
{
   // Returns 2^n (the highest bit).
   AssertFatal( x >= 0, "itrRender: negative value passed to getPower()" );
   int i = 0;
   if (x)
      do
         i++;
      while (x >>= 1);
   return i;
}

}; // namespace {} 


int ITRRender::getOutsideVisibility(const Point3F& in_cameraPos, ITRInstance* inst)
{
   instance = inst;
   geometry = instance->getGeometry();

   int leafIndex;
   if ((leafIndex = geometry->externalLeaf(in_cameraPos)) == 0)
      leafIndex = findLeaf(in_cameraPos);
   ITRGeometry::BSPLeafWrap leafWrap(geometry, leafIndex);

   int camPosOutsideBits = geometry->getOutsideBits(leafWrap);

   return camPosOutsideBits;
}


void ITRRender::render(TSRenderContext& rc,
                       ITRInstance*     inst)
{
   // Make sure our pref is in range...
   if (PrefInteriorTextureDetail < 0)      PrefInteriorTextureDetail = 0;
   else if (PrefInteriorTextureDetail > 8) PrefInteriorTextureDetail = 8;

   instance = inst;
   geometry = instance->getGeometry();
   lighting = instance->getLighting();
   materialList = instance->getMaterialList();
   renderContext = &rc;
   
   textureScale = 1.0f / geometry->textureScale;

   //TSRenderInterface::getLocked()->setHazeLevel(0);
   renderContext = &rc;
   
   camera = rc.getCamera();
   pointArray = rc.getPointArray();
   gfxSurface = rc.getSurface();

   gfxSurface->setFillMode(GFX_FILL_TWOPASS);
   gfxSurface->setShadeSource(GFX_SHADE_NONE);
   gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
   gfxSurface->setTransparency(false);

   // DMM - State order dependancy fix
   extern void GFXShadeHazeChanged(GFXSurface*);
   GFXShadeHazeChanged(gfxSurface);


   pointArray->reset();
   pointArray->useTextures(geometry->point2List.address());
   pointArray->useIntensities(false);
   pointArray->useTextures(true);
   pointArray->setVisibility( TS::ClipMask );

   // If the interior was built at high detail level (ie. shared vertices inserted)
   //  then we must render it using perspective correction.  If not, try to render
   //  it using trifans, and non-perspective renderers...
   //
   if (geometry->testFlag(ITRGeometry::LowDetailInterior) == false)
      gfxSurface->setTexturePerspective(true);
   else
      gfxSurface->setTexturePerspective(false);

   // Get camera position
   TMat3F tco = camera->getTOC();
   tco.inverse();
   cameraPos = tco.p;

   //
   int leafIndex;
   if ((leafIndex = geometry->externalLeaf(cameraPos)) == 0) {
      leafIndex = findLeaf(cameraPos);
   }
   ITRGeometry::BSPLeafWrap leafWrap(geometry, leafIndex);
   outsideBits = geometry->getOutsideBits(leafWrap);

   

#ifdef DEBUG
   ITRMetrics.render.currentLeaf = leafIndex;
   ITRMetrics.render.outsideBits = outsideBits;
#endif

   surfaceSet.setSize((geometry->surfaceList.size() >> 3) + 1);
   surfaceSet.clear(geometry->surfaceList.size());
   
   planeSet.setSize((geometry->planeList.size() >> 3) + 1);
   planeSet.clear(geometry->planeList.size());

   pointSet.setSize((geometry->point3List.size() >> 3) + 1);
   pointSet.clear(geometry->point3List.size());

   pointMap.setSize(geometry->point3List.size());
   
   leafVisible(leafWrap);
   processPVS(leafWrap);
   if(camera->getCameraType() == TS::PerspectiveCameraType)
      processPlanes(cameraPos);
   processSurfaces();
   processPoints();

   if (dynamic_cast<OpenGL::Surface*>(gfxSurface) == NULL) {
      renderSurfaces();
   } else {
//      OpenGL::Surface* pOGLSurface = static_cast<OpenGL::Surface*>(gfxSurface);
//      if (pOGLSurface->getTextureCache()->supportsSGIMultitexture() == true) {
         renderSurfacesOGL(false);
         renderSurfacesOGL(true);
//      } else {
//
//       // First, build the handles, then render in two
//       //  passes...
//       renderSurfacesOGL(false);
//       renderSurfacesOGLSP();
//      }

      OpenGL::externCheckCache(dynamic_cast<OpenGL::Surface*>(gfxSurface));
   }
}


//----------------------------------------------------------------------------
// Flag the data in this leaf as visible.
//
void ITRRender::leafVisible(ITRGeometry::BSPLeafWrap& leafWrap)
{
   ITRMetrics.render.incLeafs();
   surfaceSet.uncompress(&geometry->bitList[leafWrap.getSurfaceIndex()],
                         leafWrap.getSurfaceCount());
   planeSet.uncompress(&geometry->bitList[leafWrap.getPlaneIndex()],
                       leafWrap.getPlaneCount());
}


//----------------------------------------------------------------------------
// Find the leaf that contains the given point.
//
int ITRRender::findLeaf(const Point3F& p,int nodeIndex)
{
   if (nodeIndex < 0) {
      return -(nodeIndex+1);
   }
   //
   ITRGeometry::BSPNode& node = geometry->nodeList[nodeIndex];
   if (geometry->planeList[node.planeIndex].whichSide(p) == TPlaneF::Inside)
      return findLeaf(p,node.front);
   return findLeaf(p,node.back);
}


//----------------------------------------------------------------------------
// Process all the leafs marked as visible from the given leaf.
//
void ITRRender::processPVS(ITRGeometry::BSPLeafWrap& leafWrap)
{
   if (leafWrap.isSolid()) {
      // Solid leaves have no pvs, and therefore, nothing is visible
      //  from them
      return;
   }

   UInt8* pbegin = &geometry->bitList[leafWrap.getPVSIndex()];
   UInt8* pend = pbegin + leafWrap.getPVSCount();
   for (ITRCompressedBitVector::iterator itr(pbegin,pend); ++itr; )
      // Don't bother processing any of the outside entries,
      // they are alway empty.
      if (*itr >= ITRGeometry::ReservedOutsideLeafs) {
         ITRGeometry::BSPLeafWrap vleafWrap(geometry, *itr);
         AssertFatal(vleafWrap.isSolid() == false,
                     "Should never be a solid node in PVS set");
         // Make sure the leaf bounding sphere intersects the
         // camera viewcone.
         const Box3F* pBox;
         vleafWrap.getBoundingBox(pBox);
         SphereF sphere;
         Point3F diag;
         sphere.center = diag = pBox->fMax;
         diag -= pBox->fMin;
         sphere.center += pBox->fMin;
         sphere.center *= 0.5f;
         sphere.radius = diag.lenf();
         if (camera->testVisibility(sphere) != TS::ClipNoneVis)
            leafVisible(vleafWrap);
      }
}


//----------------------------------------------------------------------------
// Remove backfaced planes from the bit set
//
void ITRRender::processPlanes(const Point3F& cp)
{
   for (ITRBitVector::iterator itr(planeSet); ++itr; ) {
      ITRMetrics.render.incPlanes();
      if (geometry->planeList[*itr].whichSide(cp) != TPlaneF::Inside)
         itr.clear();
   }
}  


//----------------------------------------------------------------------------
// Removed backfaced surfaces from the set and build a set of
// points to be transformed
//
void ITRRender::processSurfaces()
{
   for (ITRBitVector::iterator itr(surfaceSet); ++itr; ) {
      ITRGeometry::Surface& surface = geometry->surfaceList[*itr];
      if (!surface.planeFront ^ planeSet.test(surface.planeIndex))
         pointSet.uncompress(&geometry->bitList[surface.pointIndex],
            surface.pointCount);
      else
         itr.clear();
   }
}


//----------------------------------------------------------------------------
// Transform all the points in the point set
//
void ITRRender::processPoints()
{
   for (ITRBitVector::iterator itr(pointSet); ++itr; ) {
      AssertFatal((*itr) < geometry->point3List.size(),
                  "Point out of bounds...");

      Point3F& pp = geometry->point3List[*itr];
      PointM& pm = pointMap[*itr];
      pm.index = pointArray->addPoint(pp);
      pm.distance = m_dist(pp,cameraPos);
   }
}

//----------------------------------------------------------------------------

void ITRRender::renderSurfaces()
{
   static float textureScaleTable[ 1 << ITRGeometry::Surface::textureScaleBits ];
   
   // generate the scale table
   int size = 1 << ( ITRGeometry::Surface::textureScaleBits - 1 );
   for( int i = 0; i < size; i++ ) {
      textureScaleTable[ i + size ] = textureScale * ( 1 << i );
      textureScaleTable[ i ]        = textureScale / ( 1 << i );
   }
   
   instance->startRender ( *renderContext );
   
   for (ITRBitVector::iterator itr(surfaceSet); ++itr; ) {
      ITRMetrics.render.incSurfaces();
      ITRGeometry::Surface& surface = geometry->surfaceList[*itr];

      GFXTextureHandle gfxHandle;
      gfxHandle.key[0] = ((instance->m_instanceKey << 24) |
                          UInt32(instance->getDetailLevel()));
      gfxHandle.key[1] = *itr;

      ITRInstance::Surface& isurface = *instance->getSurface(*itr);
      if (gfxSurface->setTextureHandle(gfxHandle)) {
         // Make sure light map hasn't changed
         if (!isurface.isLightMapValid()) {
            ITRMetrics.render.incTextureCache();
            GFXLightMap* lightMap = instance->buildLightMap(gfxSurface,*itr, 
                           textureScaleTable[ surface.textureScaleShift ] );
            gfxSurface->handleSetLightMap(
               instance->getLightScale(),lightMap);
            // let buildLightMap() handle this:
            // isurface.setLightMapValid();
         }
         
         // Make sure the texture hasn't changed
         if (!isurface.isTextureValid()) {
            ITRMetrics.render.incTextureCache();
            gfxSurface->handleSetTextureMap(
               const_cast<GFXBitmap*>((*materialList)[surface.material].getTextureMap()));
            isurface.setTextureValid();
         }
      }
      else {
         ITRMetrics.render.incTextureCache();
         GFXLightMap* lightMap = instance->buildLightMap(gfxSurface,*itr, 
                           textureScaleTable[ surface.textureScaleShift ] );
         int maxMip = geometry->highestMipLevel;
         gfxSurface->registerTexture(gfxHandle,
            (surface.textureSize.x+1) << maxMip,
            (surface.textureSize.y+1) << maxMip,
            surface.textureOffset.x,surface.textureOffset.y,
            instance->getLightScale(),lightMap,
            const_cast<GFXBitmap*>((*materialList)
               [surface.material].getTextureMap()),0);
         // let buildLightMap() handle this:
         // isurface.setLightMapValid();
         isurface.setTextureValid();
      }

      //
      static TS::VertexIndexPair ilist[200];
      AssertFatal(surface.vertexCount < 200,
         "ITRRender::renderSurfaces: Poly vertex count too high");
      if (surface.vertexCount >= 200)
         continue;

      ITRGeometry::Vertex* vertex = &geometry->vertexList[surface.vertexIndex];
      TS::TransformedVertex* va = &pointArray->getTransformedVertex(0);

      // Build list of vertices for 3Space and find closest
      // vertex to the camera.
      float distance = 1.0E20f;
      int clipAnd = TS::ClipMask;
      int clipOr = 0;
      for (int i = 0; i < surface.vertexCount; i++) {
         register ITRGeometry::Vertex& vp = vertex[i];
         register PointM& pm = pointMap[vp.pointIndex];
         ilist[i].fTextureIndex = vp.textureIndex;
         ilist[i].fVertexIndex = pm.index;

         if (pm.distance < distance)
            distance = pm.distance;

         register int status = va[pm.index].fStatus;
         clipOr |= status;
         clipAnd &= status;
      }

      // Early out testing and clip flags for 3Space
      if (clipAnd)
         continue;
      pointArray->setVisibility(clipOr);
   
      // Determim mip-level to use based on closest poly vertex
      int mipLevel;
      if (distance > 0.0f && distance != 1e20f) {
         float projRadius = camera->projectRadius(distance, textureScaleTable[surface.textureScaleShift]);
         mipLevel = getPower(int(float(1.0f / projRadius)));
      } else {
         mipLevel = 0;
      }
      mipLevel += PrefInteriorTextureDetail;

//    int mipLevel = (distance > 0.0f && distance != 1e20f) ? getPower(int(1.0f /
//       (camera->projectRadius(distance,textureScaleTable[ surface.textureScaleShift ])))): 0;
////        (camera->projectRadius(distance,textureScale )))): 0;

      // Make sure the miplevel is in range.  Note: this is a little confusing:
      //  highestMipLevel is actually the _smallest_ number allowed...
      //
      if (mipLevel < geometry->highestMipLevel)
         mipLevel = geometry->highestMipLevel;

#ifdef DEBUG
      int bd = (*materialList)[surface.material].
         getTextureMap()->detailLevels - 1;
      if (mipLevel > bd)
         mipLevel = bd;
#endif
      gfxSurface->handleSetMipLevel(mipLevel);

      // 
      ITRMetrics.render.incPolys();
      pointArray->drawPoly(surface.vertexCount,ilist,0);
   }
}


void ITRRender::renderSurfacesOGL(const bool in_renderPass)
{
   static float textureScaleTable[ 1 << ITRGeometry::Surface::textureScaleBits ];
   
   // generate the scale table
   int size = 1 << ( ITRGeometry::Surface::textureScaleBits - 1 );
   for( int i = 0; i < size; i++ ) {
      textureScaleTable[ i + size ] = textureScale * ( 1 << i );
      textureScaleTable[ i ]        = textureScale / ( 1 << i );
   }
   
   if (in_renderPass == false)
      instance->startRender ( *renderContext );
   
   for (ITRBitVector::iterator itr(surfaceSet); ++itr; ) {
      ITRMetrics.render.incSurfaces();
      ITRGeometry::Surface& surface = geometry->surfaceList[*itr];

      GFXTextureHandle gfxHandle;
      gfxHandle.key[0] = ((instance->m_instanceKey << 24) |
                          UInt32(instance->getDetailLevel()));
      gfxHandle.key[1] = *itr;

      ITRInstance::Surface& isurface = *instance->getSurface(*itr);
      if (gfxSurface->setTextureHandle(gfxHandle)) {
         if (in_renderPass == false) {
            // Make sure light map hasn't changed
            if (!isurface.isLightMapValid()) {
               ITRMetrics.render.incTextureCache();
               GFXLightMap* lightMap = instance->buildLightMap(gfxSurface,*itr, 
                              textureScaleTable[ surface.textureScaleShift ] );
               gfxSurface->handleSetLightMap(
                  instance->getLightScale(),lightMap);
               // let buildLightMap() handle this:
               // isurface.setLightMapValid();
            }
         
            // Make sure the texture hasn't changed
            if (!isurface.isTextureValid()) {
               ITRMetrics.render.incTextureCache();
               gfxSurface->handleSetTextureMap(
                  const_cast<GFXBitmap*>((*materialList)[surface.material].getTextureMap()));
               isurface.setTextureValid();
            }
         }
      }
      else {
         ITRMetrics.render.incTextureCache();
         GFXLightMap* lightMap = instance->buildLightMap(gfxSurface,*itr, 
                           textureScaleTable[ surface.textureScaleShift ] );
         int maxMip = geometry->highestMipLevel;
         gfxSurface->registerTexture(gfxHandle,
            (surface.textureSize.x+1) << maxMip,
            (surface.textureSize.y+1) << maxMip,
            surface.textureOffset.x,surface.textureOffset.y,
            instance->getLightScale(),lightMap,
            const_cast<GFXBitmap*>((*materialList)
               [surface.material].getTextureMap()),0);
         // let buildLightMap() handle this:
         // isurface.setLightMapValid();
         isurface.setTextureValid();
      }

      if (in_renderPass == true) {
         //
         static TS::VertexIndexPair ilist[200];
         AssertFatal(surface.vertexCount < 200,
            "ITRRender::renderSurfaces: Poly vertex count too high");
         if (surface.vertexCount >= 200)
            continue;

         ITRGeometry::Vertex* vertex = &geometry->vertexList[surface.vertexIndex];
         TS::TransformedVertex* va = &pointArray->getTransformedVertex(0);

         // Build list of vertices for 3Space and find closest
         // vertex to the camera.
         float distance = 1.0E20f;
         int clipAnd = TS::ClipMask;
         int clipOr = 0;
         for (int i = 0; i < surface.vertexCount; i++) {
            register ITRGeometry::Vertex& vp = vertex[i];
            register PointM& pm = pointMap[vp.pointIndex];
            ilist[i].fTextureIndex = vp.textureIndex;
            ilist[i].fVertexIndex = pm.index;

            if (pm.distance < distance)
               distance = pm.distance;

            register int status = va[pm.index].fStatus;
            clipOr |= status;
            clipAnd &= status;
         }

         // Early out testing and clip flags for 3Space
         if (clipAnd)
            continue;
         pointArray->setVisibility(clipOr);
   
         // 
         ITRMetrics.render.incPolys();
         pointArray->drawPoly(surface.vertexCount,ilist,0);
      }
   }
}

void ITRRender::renderSurfacesOGLSP()
{
   using namespace OpenGL;

   instance->startRender ( *renderContext );
   
   Surface* pSurface               = static_cast<Surface*>(gfxSurface);
   Surface::TextureCache* pTxCache = pSurface->getTextureCache();
   HandleCache* pHCache            = pSurface->getHandleCache();

   gfxSurface->setFillMode(GFX_FILL_TEXTUREP1);

//	Int16		__cdecl m_fpuGetControlState();
//	void		__cdecl m_fpuSetControlState(Int16 state);
   Int16 storeFPUState = m_fpuGetControlState();

   for (ITRBitVector::iterator itr(surfaceSet); ++itr; ) {
      ITRMetrics.render.incSurfaces();
      ITRGeometry::Surface& surface = geometry->surfaceList[*itr];

      GFXTextureHandle gfxHandle;
      gfxHandle.key[0] = ((instance->m_instanceKey << 24) |
                          UInt32(instance->getDetailLevel()));
      gfxHandle.key[1] = *itr;

      if (pSurface->setTextureHandle(gfxHandle) == false) {
         AssertFatal(false, "This should never happen");
      }

      // Here we are rendering the textures
      //
      HandleCacheEntry* pCEntry = pHCache->getCurrentEntry();

      if (pTxCache->setTexture(pCEntry->pBmp->getCacheInfo()) == false) {
         AssertFatal(0, "this should never happen");
      }

      static TS::VertexIndexPair ilist[200];
      AssertFatal(surface.vertexCount < 200, "ITRRender::renderSurfaces: Poly vertex count too high");
      if (surface.vertexCount >= 200)
         continue;

      ITRGeometry::Vertex* vertex = &geometry->vertexList[surface.vertexIndex];
      TS::TransformedVertex* va = &pointArray->getTransformedVertex(0);

      // Build list of vertices for 3Space and find closest
      // vertex to the camera.
      float distance = 1.0E20f;
      int clipAnd = TS::ClipMask;
      int clipOr = 0;
      for (int i = 0; i < surface.vertexCount; i++) {
         register ITRGeometry::Vertex& vp = vertex[i];
         register PointM& pm = pointMap[vp.pointIndex];
         ilist[i].fTextureIndex = vp.textureIndex;
         ilist[i].fVertexIndex = pm.index;

         if (pm.distance < distance)
            distance = pm.distance;

         register int status = va[pm.index].fStatus;
         clipOr |= status;
         clipAnd &= status;
      }

      // Early out testing and clip flags for 3Space
      if (clipAnd)
         continue;
      pointArray->setVisibility(clipOr);
   
      m_fpuSetControlState(storeFPUState);

      // 
      ITRMetrics.render.incPolys();
      pointArray->drawPoly(surface.vertexCount,ilist,0);
   }

   pSurface->setFillMode(GFX_FILL_LIGHTMAP);
   pSurface->setHazeSource(GFX_HAZE_NONE);

   for (ITRBitVector::iterator itr2(surfaceSet); ++itr2; ) {
      ITRMetrics.render.incSurfaces();
      ITRGeometry::Surface& surface = geometry->surfaceList[*itr2];

      GFXTextureHandle gfxHandle;
      gfxHandle.key[0] = ((instance->m_instanceKey << 24) | UInt32(instance->getDetailLevel()));
      gfxHandle.key[1] = *itr2;

      if (pSurface->setTextureHandle(gfxHandle) == false) {
         AssertFatal(false, "This should never happen");
      }

      // Here we are rendering the lightmaps
      //
      HandleCacheEntry* pCEntry = pHCache->getCurrentEntry();

      if (pTxCache->setLightmap(*(pCEntry->pLightmapCacheInfo)) == false) {
         AssertFatal(0, "this should never happen");
      }

      static TS::VertexIndexPair ilist[200];
      if (surface.vertexCount >= 200)
         continue;

      ITRGeometry::Vertex* vertex = &geometry->vertexList[surface.vertexIndex];
      TS::TransformedVertex* va = &pointArray->getTransformedVertex(0);

      // Build list of vertices for 3Space and find closest
      // vertex to the camera.
      float distance = 1.0E20f;
      int clipAnd = TS::ClipMask;
      int clipOr = 0;
      for (int i = 0; i < surface.vertexCount; i++) {
         register ITRGeometry::Vertex& vp = vertex[i];
         register PointM& pm = pointMap[vp.pointIndex];
         ilist[i].fTextureIndex = vp.textureIndex;
         ilist[i].fVertexIndex = pm.index;

         if (pm.distance < distance)
            distance = pm.distance;

         register int status = va[pm.index].fStatus;
         clipOr |= status;
         clipAnd &= status;
      }

      // Early out testing and clip flags for 3Space
      if (clipAnd)
         continue;
      pointArray->setVisibility(clipOr);
   
   m_fpuSetControlState(storeFPUState);

      pointArray->drawPoly(surface.vertexCount,ilist,0);
   }
}
