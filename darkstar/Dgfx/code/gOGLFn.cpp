//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <GL/gl.h>

#include "d_funcs.h"
#include "fn_all.h"
#include "g_pal.h"
#include "g_bitmap.h"
#include "gfxMetrics.h"
#include "p_txcach.h"

#include "gOGLSfc.h"
#include "gOGLTx.h"
//#include "gOGLTCache.h"
#include "r_clip.h"

#ifdef DEBUG
#define OGL_ERROR_CHECK(line) while(checkOGLError(line));
#else
#define OGL_ERROR_CHECK(line) ;
#endif

extern UInt32 g_texDownloadThisFrame;
extern UInt32 g_lmDownloadThisFrame;
extern UInt32 g_lmDownloadBytes;
extern UInt32 g_oglFrameKey;
extern UInt32 g_oglEntriesTouched;

extern UInt32 g_oglFrameKeyAccum;
extern UInt32 g_oglFrameKeyNum;
extern float  g_oglAverageFrameKey;

bool g_prefOGLNoAddFade = false;

namespace OpenGL {

inline bool
checkOGLError(const int in_lineNo)
{
#ifdef DEBUG
   GLenum error = glGetError();
   AssertWarn(error == GL_NO_ERROR, avar("%s: real line: %d", translateOpenGLError(error), in_lineNo));
   return error != GL_NO_ERROR;
#else
   return false;
#endif   
}


class HazeTriCache
{
   GLfloat m_vertexCache[1024 * 4];
   GLfloat m_hazeCache[1024];

   GLfloat m_vertexFanCache[1024 * 4];
   GLfloat m_hazeFanCache[1024];
   UInt32  m_fanCache[256];

   UInt32 m_currentVertex;

   UInt32 m_currentFanVertex;
   UInt32 m_numFans;

  public:
   HazeTriCache() : m_currentVertex(0), m_currentFanVertex(0), m_numFans(0) { }

   void addVertex(const float* in_v, const float in_haze);

   void addFanVertex(const float* in_v,
                     const float  in_haze);
   void emitFan();

   void flushCache(Surface*);
   void checkCache(Surface*);
};

inline void
HazeTriCache::addVertex(const float* in_v, const float in_haze)
{
   GLfloat* pVert = &m_vertexCache[m_currentVertex * 4];
   GLfloat* pHaze = &m_hazeCache[m_currentVertex];

   memcpy(pVert, in_v, sizeof(float) * 4);
   *pHaze = in_haze;

   m_currentVertex++;
}

inline void
HazeTriCache::addFanVertex(const float* in_v, const float in_haze)
{
   AssertFatal(m_currentFanVertex < 1023, "Huh?");

   GLfloat* pVert = &m_vertexFanCache[m_currentFanVertex * 4];
   GLfloat* pHaze = &m_hazeFanCache[m_currentFanVertex];

   memcpy(pVert, in_v, sizeof(float) * 4);
   *pHaze = in_haze;

   m_currentFanVertex++;
}

inline void
HazeTriCache::emitFan()
{
   m_fanCache[m_numFans] = m_currentFanVertex;
   m_numFans++;
}

void
HazeTriCache::checkCache(Surface* pSurface)
{
   if (m_currentVertex    >= 900 ||
       m_currentFanVertex >= 900)
      flushCache(pSurface);
}

void
HazeTriCache::flushCache(Surface* pSurface)
{
//   ColorF fillColor;
//   fillColor.set(pow(float(pColors[in_index].peRed)   / 255.0f, 1.0f / gamma),
//                 pow(float(pColors[in_index].peGreen) / 255.0f, 1.0f / gamma),
//                 pow(float(pColors[in_index].peBlue)  / 255.0f, 1.0f / gamma));


   Surface::TextureCache* pTxCache = pSurface->getTextureCache();
   GLfloat hazeColor[4] = {
      pow(pSurface->m_hazeColor.red,   1.0f / pSurface->getGamma()),
      pow(pSurface->m_hazeColor.green, 1.0f / pSurface->getGamma()),
      pow(pSurface->m_hazeColor.blue,  1.0f / pSurface->getGamma())
   };

   if (m_currentVertex != 0) {
      pTxCache->enableTexUnits(false);
      pTxCache->setBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glBegin(GL_TRIANGLES);
      for (UInt32 i = 0; i < m_currentVertex; i++) {
         hazeColor[3] = m_hazeCache[i];
         glColor4fv(hazeColor);
         glVertex4fv(&m_vertexCache[i * 4]);
      }
      glEnd();
   }
   m_currentVertex = 0;

   if (m_numFans != 0) {
      int vertexIndex = 0;
      for (UInt32 i = 0; i < m_numFans; i++) {
         glBegin(GL_TRIANGLE_FAN);
         for (UInt32 j = vertexIndex; j < m_fanCache[i]; j++) {
            hazeColor[3] = m_hazeFanCache[j];
            glColor4fv(hazeColor);
            glVertex4fv(&m_vertexFanCache[j * 4]);
         }
         glEnd();
         vertexIndex = m_fanCache[i];
      }
   }
   m_currentFanVertex = 0;
   m_numFans          = 0;

   OGL_ERROR_CHECK(__LINE__);
}

namespace {

HazeTriCache g_hazeTriCache;

const GLuint SGIS_TEXTURE_0 = 0x835E; //0x83C6
const GLuint SGIS_TEXTURE_1 = 0x835F; //0x83C7
UInt8 g_callbackBuffer[256*256];

const GLenum GL_CLAMP_TO_EDGE              = 0x812F;

inline void
transformLMap(Point2F&                io_rCoord,
              const HandleCacheEntry* in_pEntry)
{
   Point2F copy;
   copy.x = io_rCoord.x * in_pEntry->m_col0.x +
            io_rCoord.y * in_pEntry->m_col0.y +
            1.0f        * in_pEntry->m_col2.x;
   copy.y = io_rCoord.x * in_pEntry->m_col1.x +
            io_rCoord.y * in_pEntry->m_col1.y +
            1.0f        * in_pEntry->m_col2.y;

   io_rCoord = copy;
}

inline void
setupGLConstantIndexedColor(Surface* io_pSurface,
                            DWORD    in_color)
{
   PALETTEENTRY* pColors = io_pSurface->getPalette()->palette[0].color;
   Surface::TextureCache* pTxCache = io_pSurface->getTextureCache();
   float gamma = pTxCache->m_gamma;

   ColorF finalColor;
   finalColor.red   = float(pColors[in_color].peRed)   / 255.0f;
   finalColor.green = float(pColors[in_color].peGreen) / 255.0f;
   finalColor.blue  = float(pColors[in_color].peBlue)  / 255.0f;
   if (io_pSurface->m_shadeSource == GFX_SHADE_CONSTANT) {
      finalColor.red   *= io_pSurface->m_constantShadeColor.red;
      finalColor.green *= io_pSurface->m_constantShadeColor.green;
      finalColor.blue  *= io_pSurface->m_constantShadeColor.blue;
   }

   finalColor.set(pow(finalColor.red,   1.0f / gamma),
                  pow(finalColor.green, 1.0f / gamma),
                  pow(finalColor.blue,  1.0f / gamma));
   glColor4f(GLfloat(finalColor.red),
             GLfloat(finalColor.green),
             GLfloat(finalColor.blue),
             GLfloat(io_pSurface->m_constantAlpha));
}

inline void
setupGLConstantColor(Surface* io_pSurface)
{
   if (io_pSurface->m_alphaSource == GFX_ALPHA_NONE) {
      ColorF finalColor = io_pSurface->m_fillColor;
      if (io_pSurface->m_shadeSource == GFX_SHADE_CONSTANT) {
         finalColor.red   *= io_pSurface->m_constantShadeColor.red;
         finalColor.green *= io_pSurface->m_constantShadeColor.green;
         finalColor.blue  *= io_pSurface->m_constantShadeColor.blue;
      }

      glColor4f(GLfloat(finalColor.red),
                GLfloat(finalColor.green),
                GLfloat(finalColor.blue),
                GLfloat(io_pSurface->m_constantAlpha));
   } else {
      glColor4f(GLfloat(io_pSurface->m_fillColor.red),
                GLfloat(io_pSurface->m_fillColor.green),
                GLfloat(io_pSurface->m_fillColor.blue),
                GLfloat(io_pSurface->m_constantAlpha));
   }
}


UInt32
getNextPow2(UInt32 size)
{
   int oneCount = 0;
   int shiftCount = -1;
   while(size) {
      if(size & 1)
         oneCount++;
      shiftCount++;
      size >>= 1;
   }
   if(oneCount > 1)
      shiftCount++;
   return (1 << shiftCount);
}

float sg_lscale;
float sg_rscale;
float sg_tscale;
float sg_bscale;

void
bitmapDraw(GFXSurface*      io_pSurface,
           const GFXBitmap* in_pBM)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   Surface::TextureCache* pTxCache = pSurface->getTextureCache();

   if (pTxCache->setTexture(in_pBM->getCacheInfo(), 0) == false)
      pTxCache->cacheBitmap(in_pBM, in_pBM->getCacheInfo(), 0, true);

   pTxCache->enableTexUnits(true, false);
   pTxCache->setTransparent((in_pBM->attribute & BMA_TRANSPARENT) != 0);

   if ((in_pBM->attribute & BMA_TRANSLUCENT) != 0 ||
       pSurface->m_constantAlpha != 1.0f) {
      pTxCache->setBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   } else {
      pTxCache->setBlendMode(GL_ONE, GL_ZERO);
   }
   pTxCache->setTexMode(GL_MODULATE);

   glColor4f(GLfloat(1.0f), GLfloat(1.0f), GLfloat(1.0f), 
             GLfloat(pSurface->m_constantAlpha));
   glBegin(GL_TRIANGLE_FAN);
   for (int i = 0; i < pSurface->getCurrVertexIndex(); i++) {
      glTexCoord2fv((GLfloat*)&pSurface->m_pTexCoord0Array[i]);
      glVertex4fv((GLfloat*)&pSurface->m_pVertexArray[i]);
   }
   glEnd();

   pSurface->clearCurrVertexIndex();
}

bool setupBitmapPoints(GFXSurface* io_pSurface,
                       int x0, int y0,
                       int x1, int y1)
{
   sg_lscale = sg_tscale = 0.0f;
   sg_rscale = sg_bscale = 1.0f;

   if(io_pSurface->getFlags() & GFX_DMF_RCLIP) {
      RectI *cr = io_pSurface->getClipRect();
      float le = float(cr->upperL.x);
      float te = float(cr->upperL.y);
      float re = float(cr->lowerR.x) + 1.0f;
      float be = float(cr->lowerR.y) + 1.0f;

      if(x0 >= re || (x0 + x1) <= le || y0 >= be || (y0 + y1) <= te)
         return false;

      if(x0 < le)
         sg_lscale = (le - x0) / x1;
      if(y0 < te)
         sg_tscale = (te - y0) / y1;
      if(x0 + x1 > re)
         sg_rscale = (re - x0) / x1;
      if(y0 + y1 > be)
         sg_bscale = (be - y0) / y1;
   }

   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   DGLVertex4F* pVerts = pSurface->getCurrentVertex();

   pVerts[0].x = float(x0) + float(sg_lscale * x1);
   pVerts[0].y = float(y0) + float(sg_tscale * y1);
   pVerts[1].x = float(x0) + float(sg_rscale * x1);
   pVerts[1].y = float(y0) + float(sg_tscale * y1);
   pVerts[2].x = float(x0) + float(sg_rscale * x1);
   pVerts[2].y = float(y0) + float(sg_bscale * y1);
   pVerts[3].x = float(x0) + float(sg_lscale * x1);
   pVerts[3].y = float(y0) + float(sg_bscale * y1);

   for(int i = 0; i < 4; i++) {
      pVerts[i].z = 0.0f;
      pVerts[i].w = 1.0f;
   }

   return true;
}

void
setupBitmapTexCoords(GFXSurface*      io_pSurface,
                     const GFXBitmap* in_pBM,
                     float x0, float y0,
                     float x1, float y1)
{
   Surface* pSurface        = static_cast<Surface*>(io_pSurface);
   DGLTexCoord4F* pTexVerts = pSurface->getCurrentTexCoord0();

   UInt32 padWidth  = getNextPow2(in_pBM->getWidth());
   UInt32 padHeight = getNextPow2(in_pBM->getHeight());

   float hScale = 1.0f / float(padWidth);
   float vScale = 1.0f / float(padHeight);

   pTexVerts[0].s = float(x0) + float(x1 - x0) * sg_lscale;
   pTexVerts[0].t = float(y0) + float(y1 - y0) * sg_tscale;
   pTexVerts[1].s = float(x0) + float(x1 - x0) * sg_rscale;
   pTexVerts[1].t = pTexVerts[0].t;
   pTexVerts[2].s = pTexVerts[1].s;
   pTexVerts[2].t = float(y0) + float(y1 - y0) * sg_bscale;
   pTexVerts[3].s = pTexVerts[0].s;
   pTexVerts[3].t = pTexVerts[2].t;
   
   for (int i = 0; i < 4; i++) {
      pTexVerts[i].s *= hScale;
      pTexVerts[i].t *= vScale;

      pTexVerts[i].r = 0.0f;
      pTexVerts[i].q = 1.0f;
   }
}

} // namespace {}


void
externCheckCache(Surface* pSurface)
{
   g_hazeTriCache.flushCache(pSurface);
}



#define FAKE_W_BUFFER

void 
AddVertexVTC(GFXSurface*          io_pSurface,
             const Point3F*       in_pVert,
             const Point2F*       in_pTex,
             const GFXColorInfoF* in_pColor,
             DWORD                /*in_softwareEdgeKey*/)
{
   // Function must handle the case that in_pTex or in_pColor are NULL.  This
   //  should never happen unless the parameters are unnecessary to the current
   //  poly type.
   // in_softwareEdgeKey can be ignored
   //
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   DGLVertex4F* pVertex = pSurface->getCurrentVertex();
   
   // We always enter the coord into the array...
   //
#ifdef FAKE_W_BUFFER
   if (in_pVert->z != 0.0f) {
      // First, reextract z.  in_pVert->z == nearPlane / z
      //
      float oglZ  = (2.0f * in_pVert->z) - 1.0f;
      float zCalc = (1.0f / in_pVert->z) * pSurface->m_nearClipPlane;

      pVertex->x = in_pVert->x * zCalc;
      pVertex->y = in_pVert->y * zCalc;
      pVertex->z = oglZ        * zCalc;
      pVertex->w = zCalc;
   } else {
      float zCalc = pSurface->m_farClipPlane;

      pVertex->x = in_pVert->x * zCalc;
      pVertex->y = in_pVert->y * zCalc;
      pVertex->z = 0.0f;
      pVertex->w = zCalc;
   }
#else
   if (in_pVert->z != 0.0f) {
      // First, reextract z.  in_pVert->z == nearPlane / z
      //
      float zCalc = (1.0f / in_pVert->z) * pSurface->m_nearClipPlane;
      float oglZ  = (zCalc * (pSurface->m_nearClipPlane + pSurface->m_farClipPlane) - 2.0f * (pSurface->m_nearClipPlane * pSurface->m_farClipPlane)) / (pSurface->m_farClipPlane - pSurface->m_nearClipPlane);

      pVertex->x = in_pVert->x * zCalc;
      pVertex->y = in_pVert->y * zCalc;
      pVertex->z = oglZ        * zCalc;
      pVertex->w = zCalc;
   } else {
      float zCalc = pSurface->m_farClipPlane;

      pVertex->x = in_pVert->x * zCalc;
      pVertex->y = in_pVert->y * zCalc;
      pVertex->z = 0.0f;
      pVertex->w = zCalc;
   }
#endif

   // We only need the color if the mode is GFX_SHADE_VERTEX
   //
   if (pSurface->m_shadeSource == GFX_SHADE_VERTEX) {
      AssertFatal(in_pColor != NULL, "No color info for SHADE_VERTEX poly!");
      
      DGLColor4F* pColor = pSurface->getCurrentColor();
      pColor->r = in_pColor->color.red;
      pColor->g = in_pColor->color.green;
      pColor->b = in_pColor->color.blue;

      if (pSurface->m_alphaSource == GFX_ALPHA_NONE)
         pColor->a = 1.0f;
      else if (pSurface->m_alphaSource == GFX_ALPHA_CONSTANT)
         pColor->a = pSurface->m_constantAlpha;
      else if (pSurface->m_alphaSource == GFX_ALPHA_VERTEX)
         pColor->a = in_pColor->alpha;
      else if (pSurface->m_alphaSource == GFX_ALPHA_FILL) {
         AssertFatal(0, "Inconsistent alpha state: ALPHA_FILL w/ SHADE_VERTEX");
      } else {
         AssertFatal(0, "alphaSource mode is corrupt");
      }
   }

   if (pSurface->m_fillMode != GFX_FILL_CONSTANT) {
      // We need to use the texture coordinates...
      //
      DGLTexCoord4F* pTexCoord = pSurface->getCurrentTexCoord0();
      pTexCoord->s = in_pTex->x;
      pTexCoord->t = in_pTex->y;
      pTexCoord->r = 0.0;
      pTexCoord->q = 1.0;
   }

   if (pSurface->m_hazeSource == GFX_HAZE_VERTEX) {
      AssertFatal(in_pColor != NULL, "No color info for HAZEVERTEX?");

      DGLHazeCoordF* pHazeCoord = pSurface->getCurrentHazeCoord();
      pHazeCoord->h = in_pColor->haze;
   }

   pSurface->incCurrVertexIndex();
}

void 
finishPoly2PassSGI(Surface* io_pSurface)
{
   Surface::TextureCache* pTextureCache = io_pSurface->getTextureCache();
   HandleCache* pHandleCache   = io_pSurface->getHandleCache();
   HandleCacheEntry* pEntry    = pHandleCache->getCurrentEntry();
   AssertFatal(pEntry != NULL, "No handle _here_?!");

   OGL_ERROR_CHECK(__LINE__);
   if (pEntry->pBmp == NULL) {
      // Callback texture...
      AssertFatal(pEntry->cb != NULL, "No callback, and no bitmap?");

      if (pTextureCache->setTexture(pEntry->bitmapCacheInfo, 0) == false) {
         GFXBitmap bmp;
         bmp.height = bmp.stride = bmp.width = pEntry->size;
         bmp.pBits  = g_callbackBuffer;
         bmp.pMipBits[0] = bmp.pBits;
         bmp.detailLevels = 1;
         bmp.bitDepth = 8;
         pEntry->cb(pEntry->handle, &bmp, 0);
         pTextureCache->cacheBitmap(&bmp, pEntry->bitmapCacheInfo, 0, true);
         if (pTextureCache->supportsEdgeClamp() == false) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
         } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
         }
      }
   } else {
      // Normal...
      if (pTextureCache->setTexture(pEntry->pBmp->getCacheInfo(), 0) == false) {
         pTextureCache->cacheBitmap(pEntry->pBmp, pEntry->pBmp->getCacheInfo(), 0,
                                    (pEntry->isTerrain == true));
      }
   }
   OGL_ERROR_CHECK(__LINE__);

   pTextureCache->enableTexUnits(true, true);
   if (pTextureCache->setLightmap(*pEntry->pLightmapCacheInfo, 1) == false)
      pTextureCache->cacheLightmap(pEntry->pLMap, *pEntry->pLightmapCacheInfo, 1);
   OGL_ERROR_CHECK(__LINE__);

   pTextureCache->setBlendMode(GL_ONE, GL_ZERO);
   pTextureCache->setTexMode(GL_REPLACE, GL_MODULATE);
   OGL_ERROR_CHECK(__LINE__);

   glColor3f(1, 1, 1);
   glBegin(GL_TRIANGLE_FAN);
   for (int i = 0; i < io_pSurface->getCurrVertexIndex(); i++) {
      float coords[2];
      
      coords[0] = io_pSurface->m_pTexCoord0Array[i].s * pEntry->coordScale.x;
      coords[1] = io_pSurface->m_pTexCoord0Array[i].t * pEntry->coordScale.y;
      coords[0] += pEntry->coordTrans.x;
      coords[1] += pEntry->coordTrans.y;
      pTextureCache->m_multiTexExtSGI.glMTexCoord2fv(SGIS_TEXTURE_0, coords);

      Point2F lmapCoord(io_pSurface->m_pTexCoord0Array[i].s,
                        io_pSurface->m_pTexCoord0Array[i].t);
      transformLMap(lmapCoord, pEntry);
      pTextureCache->m_multiTexExtSGI.glMTexCoord2fv(SGIS_TEXTURE_1, (float*)&lmapCoord);

      glVertex4fv((GLfloat*)&io_pSurface->m_pVertexArray[i]);
   }
   glEnd();
   OGL_ERROR_CHECK(__LINE__);
}

void 
EmitPoly2Pass(Surface* io_pSurface)
{
   Surface::TextureCache* pTextureCache = io_pSurface->getTextureCache();
   
   if (pTextureCache->supportsSGIMultiTexture() == true) {
      // Use mTex Ext.
      finishPoly2PassSGI(io_pSurface);
      OGL_ERROR_CHECK(__LINE__);
   } else {
      HandleCache* pHandleCache   = io_pSurface->getHandleCache();
      HandleCacheEntry* pEntry    = pHandleCache->getCurrentEntry();
      AssertFatal(pEntry != NULL, "No handle _here_?!");

      if (pEntry->pBmp == NULL) {
         // Callback texture...
         AssertFatal(pEntry->cb != NULL, "No callback, and no bitmap?");

         if (pTextureCache->setTexture(pEntry->bitmapCacheInfo, 0) == false) {
            GFXBitmap bmp;
            bmp.height = bmp.stride = bmp.width = pEntry->size;
            bmp.pBits  = g_callbackBuffer;
            bmp.pMipBits[0] = bmp.pBits;
            bmp.detailLevels = 1;
            bmp.bitDepth = 8;
            pEntry->cb(pEntry->handle, &bmp, 0);
            pTextureCache->cacheBitmap(&bmp, pEntry->bitmapCacheInfo, 0, true);
         }
      } else {
         // Normal...
         if (pTextureCache->setTexture(pEntry->pBmp->getCacheInfo(), 0) == false) {
            pTextureCache->cacheBitmap(pEntry->pBmp, pEntry->pBmp->getCacheInfo(), 0,
                                       pEntry->isTerrain == true);
         }
      }
      OGL_ERROR_CHECK(__LINE__);

      pTextureCache->enableTexUnits(true);
      pTextureCache->setBlendMode(GL_ONE, GL_ZERO);
      pTextureCache->setTexMode(GL_REPLACE);
      glBegin(GL_TRIANGLE_FAN);
      int i;
      for (i = 0; i < io_pSurface->getCurrVertexIndex(); i++) {
         float coords[2];
         coords[0] = io_pSurface->m_pTexCoord0Array[i].s * pEntry->coordScale.x;
         coords[1] = io_pSurface->m_pTexCoord0Array[i].t * pEntry->coordScale.y;
         coords[0] += pEntry->coordTrans.x;
         coords[1] += pEntry->coordTrans.y;
         glTexCoord2fv(coords);
         glVertex4fv((GLfloat*)&io_pSurface->m_pVertexArray[i]);
      }
      glEnd();

      if (pTextureCache->setLightmap(*pEntry->pLightmapCacheInfo, 0) == false)
         pTextureCache->cacheLightmap(pEntry->pLMap, *pEntry->pLightmapCacheInfo, 0);

      pTextureCache->setBlendMode(GL_ZERO, GL_SRC_COLOR);

      glBegin(GL_TRIANGLE_FAN);
      for (i = 0; i < io_pSurface->getCurrVertexIndex(); i++) {
	      // Altimor: Unused
         //float coords[2];

         Point2F lmapCoord(io_pSurface->m_pTexCoord0Array[i].s,
                           io_pSurface->m_pTexCoord0Array[i].t);
         transformLMap(lmapCoord, pEntry);
         glTexCoord2f(lmapCoord.x, lmapCoord.y);
         glVertex4fv((GLfloat*)&io_pSurface->m_pVertexArray[i]);
      }
      glEnd();
      OGL_ERROR_CHECK(__LINE__);
   }
}

void 
EmitPoly(GFXSurface* io_pSurface)
{
   // Send the current set of vertices to the card...
   //
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   Surface::TextureCache* pTxCache = pSurface->getTextureCache();

   if (pSurface->m_fillMode == GFX_FILL_TWOPASS) {
      EmitPoly2Pass(pSurface);
   } else {
      if (pSurface->m_fillMode == GFX_FILL_CONSTANT) {
         pTxCache->enableTexUnits(false);

         setupGLConstantColor(pSurface);
         glBegin(GL_TRIANGLE_FAN);
         for (int i = 0; i < pSurface->getCurrVertexIndex(); i++) {
            if (pSurface->m_colorEnabled == true) {
               glColor4f(pSurface->m_pColorArray[i].r * pSurface->m_fillColor.red,
                         pSurface->m_pColorArray[i].g * pSurface->m_fillColor.green,
                         pSurface->m_pColorArray[i].b * pSurface->m_fillColor.blue,
                         pSurface->m_pColorArray[i].a);
            }

            glVertex4fv((GLfloat*)&pSurface->m_pVertexArray[i]);
         }
         glEnd();
      } else if (pSurface->m_fillMode == GFX_FILL_LIGHTMAP) {
         HandleCache* pHandleCache = pSurface->getHandleCache();
         HandleCacheEntry* pEntry  = pHandleCache->getCurrentEntry();

         // Texture: lightmap
         pTxCache->enableTexUnits(true, false);
         pTxCache->setTexMode(GL_REPLACE);
         pTxCache->setBlendMode(GL_ZERO, GL_SRC_COLOR);

         glBegin(GL_POLYGON);
         for (int i = 0; i < pSurface->getCurrVertexIndex(); i++) {
            Point2F lmapCoord(pSurface->m_pTexCoord0Array[i].s,
                              pSurface->m_pTexCoord0Array[i].t);
            transformLMap(lmapCoord, pEntry);
            glTexCoord2f(lmapCoord.x, lmapCoord.y);
            glVertex4fv((GLfloat*)&pSurface->m_pVertexArray[i]);
         }
         glEnd();
      } else if (pSurface->m_fillMode == GFX_FILL_TEXTUREP1) {
         HandleCache* pHandleCache = pSurface->getHandleCache();
         HandleCacheEntry* pEntry  = pHandleCache->getCurrentEntry();

         // Texture: lightmap
         pTxCache->enableTexUnits(true, false);
         pTxCache->setTexMode(GL_REPLACE);
         pTxCache->setBlendMode(GL_ONE, GL_ZERO);

         glBegin(GL_POLYGON);
         for (int i = 0; i < pSurface->getCurrVertexIndex(); i++) {
            float coords[2];
            coords[0] = pSurface->m_pTexCoord0Array[i].s * pEntry->coordScale.x;
            coords[1] = pSurface->m_pTexCoord0Array[i].t * pEntry->coordScale.y;
            coords[0] += pEntry->coordTrans.x;
            coords[1] += pEntry->coordTrans.y;
            glTexCoord2fv(coords);
            glVertex4fv((GLfloat*)&pSurface->m_pVertexArray[i]);
         }
         glEnd();
      } else {
         // Texture
         pTxCache->enableTexUnits(true, false);
         pTxCache->setTexMode(GL_MODULATE);

         glBegin(GL_POLYGON);
         setupGLConstantColor(pSurface);
         for (int i = 0; i < pSurface->getCurrVertexIndex(); i++) {
            if (pSurface->m_colorEnabled == true)
               glColor4fv((GLfloat*)&pSurface->m_pColorArray[i]);

            glTexCoord2fv((GLfloat*)&pSurface->m_pTexCoord0Array[i]);
            glVertex4fv((GLfloat*)&pSurface->m_pVertexArray[i]);
         }
         glEnd();
      }
   }
   
   if (pSurface->m_hazeSource == GFX_HAZE_VERTEX) {
      if (pSurface->getCurrVertexIndex() != 3) {
         for (int i = 0; i < pSurface->getCurrVertexIndex(); i++) {
            g_hazeTriCache.addFanVertex((const float*)&pSurface->m_pVertexArray[i],
                                        pSurface->m_pHazeStoreArray[i].h);
         }
         g_hazeTriCache.emitFan();
         g_hazeTriCache.checkCache(pSurface);
      } else {
         for (int i = 0; i < pSurface->getCurrVertexIndex(); i++) {
            g_hazeTriCache.addVertex((const float*)&pSurface->m_pVertexArray[i],
                                     pSurface->m_pHazeStoreArray[i].h);
         }
         g_hazeTriCache.checkCache(pSurface);
      }
   } else if (pSurface->m_hazeSource == GFX_HAZE_CONSTANT) {
      if (pSurface->getCurrVertexIndex() > 3) {

         for (int i = 0; i < pSurface->getCurrVertexIndex() - 2; i++) {
            g_hazeTriCache.addVertex((const float*)&pSurface->m_pVertexArray[0],
                                     pSurface->m_constantHaze);
            g_hazeTriCache.addVertex((const float*)&pSurface->m_pVertexArray[i+1],
                                     pSurface->m_constantHaze);
            g_hazeTriCache.addVertex((const float*)&pSurface->m_pVertexArray[i+2],
                                     pSurface->m_constantHaze);
         }

         g_hazeTriCache.checkCache(pSurface);
      } else {
         for (int i = 0; i < pSurface->getCurrVertexIndex(); i++) {
            g_hazeTriCache.addVertex((const float*)&pSurface->m_pVertexArray[i],
                                     pSurface->m_constantHaze);
         }
         g_hazeTriCache.checkCache(pSurface);
      }
   }

   pSurface->clearCurrVertexIndex();
   OGL_ERROR_CHECK(__LINE__);
}


void 
SetTransparency(GFXSurface* io_pSurface,
                Bool        in_transFlag)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   Surface::TextureCache* pTextureCache = pSurface->getTextureCache();
   pTextureCache->setTransparent(in_transFlag);
}


void 
SetFillMode(GFXSurface* io_pSurface,
            GFXFillMode in_fm)
{
   // Possible fill modes:
   //  GFX_FILL_CONSTANT: Color fill
   //  GFX_FILL_TEXTURE:  Texture fill
   //  GFX_FILL_TWOPASS:  Texture with lightmap
   //
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   pSurface->m_fillMode = in_fm;
   pSurface->m_fillColor.set(1, 1, 1);
}


void 
SetTextureMap(GFXSurface*      io_pSurface,
              const GFXBitmap* in_pTextureMap)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   Surface::TextureCache* pTextureCache = pSurface->getTextureCache();

   if (pTextureCache->setTexture(in_pTextureMap->getCacheInfo(), 0) == false)
      pTextureCache->cacheBitmap(in_pTextureMap, in_pTextureMap->getCacheInfo(), 0, true);

   OGL_ERROR_CHECK(__LINE__);
}


void
precacheTextureHandle(Surface*          pSurface,
                      HandleCacheEntry* pEntry)
{
   Surface::TextureCache* pTextureCache = pSurface->getTextureCache();

   if (pEntry->pBmp == NULL) {
      // Callback texture...
      AssertFatal(pEntry->cb != NULL, "No callback, and no bitmap?");

      if (pTextureCache->isCurrent(pEntry->bitmapCacheInfo) == false) {
         GFXBitmap bmp;
         bmp.height = bmp.stride = bmp.width = pEntry->size;
         bmp.pBits  = g_callbackBuffer;
         bmp.pMipBits[0] = bmp.pBits;
         bmp.detailLevels = 1;
         bmp.bitDepth = 8;
         pEntry->cb(pEntry->handle, &bmp, 0);
         pTextureCache->cacheBitmap(&bmp, pEntry->bitmapCacheInfo, 0, true);
      } else {
         pTextureCache->touchEntryIfNecessary(pEntry->bitmapCacheInfo);
      }
   } else {
      // Normal...
      if (pTextureCache->isCurrent(pEntry->pBmp->getCacheInfo()) == false) {
         pTextureCache->cacheBitmap(pEntry->pBmp, pEntry->pBmp->getCacheInfo(), 0,
                                    pEntry->isTerrain == true);
      } else {
         pTextureCache->touchEntryIfNecessary(pEntry->pBmp->getCacheInfo());
      }
   }

   if (pTextureCache->supportsSGIMultiTexture() == true) {
      if (pTextureCache->isCurrentLM(*pEntry->pLightmapCacheInfo) == false) {
         pTextureCache->cacheLightmap(pEntry->pLMap, *pEntry->pLightmapCacheInfo, 1);
      } else {
         pTextureCache->touchEntryIfNecessary(*pEntry->pLightmapCacheInfo);
      }
   } else {
      if (pTextureCache->isCurrentLM(*pEntry->pLightmapCacheInfo) == false) {
         pTextureCache->cacheLightmap(pEntry->pLMap, *pEntry->pLightmapCacheInfo, 0);
      } else {
         pTextureCache->touchEntryIfNecessary(*pEntry->pLightmapCacheInfo);
      }
   }
}

void 
RegisterTexture(GFXSurface*      io_pSurface,
                GFXTextureHandle in_handle,
                int              in_sizeX,
                int              in_sizeY,
                int              in_offsetX,
                int              in_offsetY,
                int              in_lightScale,
                GFXLightMap*     io_pLightMap,
                const GFXBitmap* in_pTexture,
                int              /*in_mipLevel*/)
{
   AssertFatal((in_pTexture->getWidth()  <= 256 && in_pTexture->getWidth() > 0 &&
                in_pTexture->getHeight() <= 256 && in_pTexture->getHeight() > 0),
               "Invalid texture size");

   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   AssertFatal(pSurface->getHandleCache() != NULL, "Error - surface not locked");

   Surface::TextureCache* pTxCache = pSurface->getTextureCache();
   HandleCache* pHandleCache = pSurface->getHandleCache();
   HandleCacheEntry* ent     = pHandleCache->getFreeEntry(pTxCache);
   
   ent->handle = in_handle;
   ent->flags  = 0;
   ent->pBmp   = in_pTexture;
   ent->pLMap  = io_pLightMap;
   ent->size   = 0;
   ent->cb     = 0;
   ent->isTerrain = false;

   ent->coordScale.set(float(in_sizeX) / float(in_pTexture->getWidth()),
                       float(in_sizeY) / float(in_pTexture->getHeight()));
   ent->coordTrans.set(float(in_offsetX) / float(in_pTexture->getWidth()),
                       float(in_offsetY) / float(in_pTexture->getHeight()));

   float scx = float(in_sizeX) / float(1 << in_lightScale);
   float scy = float(in_sizeY) / float(1 << in_lightScale);

   float dim = max(getNextPow2(max(io_pLightMap->size.x, io_pLightMap->size.y)), 8UL);

   float baseoffX = 0.5f / dim;
   float baseoffY = 0.5f / dim;

   Point2F lmapScale;
   Point2F lmapTrans;
   lmapScale.set(scx / dim, scy / dim);
   lmapTrans.set(baseoffX + (lmapScale.x * io_pLightMap->offset.x) / float(in_sizeX),
                 baseoffY + (lmapScale.y * io_pLightMap->offset.y) / float(in_sizeY));

   ent->m_col0.set(lmapScale.x, 0);
   ent->m_col1.set(0,           lmapScale.y);
   ent->m_col2.set(lmapTrans.x, lmapTrans.y);

   pHandleCache->HashInsert(ent);
   precacheTextureHandle(pSurface, ent);
}

void 
RegisterTextureTer(GFXSurface*           io_pSurface,
                   GFXTextureHandle      in_handle,
                   int                   /*in_sizeX*/,
                   int                   /*in_sizeY*/,
                   GFXLightMap*          io_pLightMap,
                   const GFXBitmap*      in_pTexture,
                   int                   /*in_mipLevel*/,
                   GFXBitmap::CacheInfo* io_pCacheInfo,
                   const RectI&          in_rSubSection,
                   const int             in_flags)
{
   AssertFatal(in_rSubSection.isValidRect(), "Error, bad subsection");
   AssertFatal(dynamic_cast<Surface*>(io_pSurface) != NULL, "Error, not an opengl surface!");
   AssertFatal((in_pTexture->getWidth()  <= 256 && in_pTexture->getWidth()  > 0 &&
                in_pTexture->getHeight() <= 256 && in_pTexture->getHeight() > 0),
               "Invalid texture size");

   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   AssertFatal(pSurface->getHandleCache() != NULL, "Error - surface not locked");

   Surface::TextureCache* pTxCache = pSurface->getTextureCache();
   HandleCache* pHandleCache = pSurface->getHandleCache();
   HandleCacheEntry* ent     = pHandleCache->getFreeEntry(pTxCache);
   
   ent->handle    = in_handle;
   ent->flags     = 0;
   ent->pBmp      = in_pTexture;
   ent->pLMap     = io_pLightMap;
   ent->size      = 0;
   ent->cb        = 0;
   ent->isTerrain = true;

   ent->coordScale.set(1.0f, 1.0f);
   ent->coordTrans.set(0.0f, 0.0f);

   Int32 realSizeX = io_pLightMap->size.x ? io_pLightMap->size.x : 256;
   Int32 realSizeY = io_pLightMap->size.y ? io_pLightMap->size.y : 256;
   if (realSizeX == 256) {
      AssertFatal(io_pCacheInfo->bitmapSequenceNum == 0xfffffffd,
                  "WTF!");
   }

   AssertFatal(realSizeX == realSizeY, "Invalid terrain lightmap, must be square");
   AssertFatal((getNextPow2(realSizeX) == UInt32(realSizeX) &&
                getNextPow2(realSizeY) == UInt32(realSizeY)),
               "Invalid terrain lightmap.  Must be pow2.");

   float realDim = float(max(realSizeX, 8L));

   // Altimor: Unused
   //float m0[3][3];
   //float temp[3][3];

   Point2F lmapScale, lmapTrans;
   lmapTrans.x = (float(in_rSubSection.upperL.x) + 0.5f) / realDim;
   lmapTrans.y = (float(in_rSubSection.upperL.y) + 0.5f) / realDim;

   lmapScale.x = float(in_rSubSection.lowerR.x - in_rSubSection.upperL.x) / realDim;
   lmapScale.y = float(in_rSubSection.lowerR.y - in_rSubSection.upperL.y) / realDim;

   enum TerrainRotation {
      Plain      = 0,
      Rotate     = 1,
      FlipX      = 2,
      FlipY      = 4,
      RotateMask = 7
   };

   switch (in_flags & RotateMask) {
     // 0
     case Plain:
      ent->m_col0.set(lmapScale.x, 0);
      ent->m_col1.set(0,           -lmapScale.y);
      ent->m_col2.set(lmapTrans.x, lmapTrans.y + lmapScale.y);
      break;

     // 0
     case FlipX:
      ent->m_col0.set(-lmapScale.x,              0);
      ent->m_col1.set(0,                         -lmapScale.y);
      ent->m_col2.set(lmapTrans.x + lmapScale.x, lmapTrans.y + lmapScale.y);
      break;

     // 0
     case FlipY:
      ent->m_col0.set(lmapScale.x, 0);
      ent->m_col1.set(0,           lmapScale.y);
      ent->m_col2.set(lmapTrans.x, lmapTrans.y);
      break;

     // 0
     case FlipY | FlipX:
      ent->m_col0.set(-lmapScale.x,              0);
      ent->m_col1.set(0,                         lmapScale.y);
      ent->m_col2.set(lmapTrans.x + lmapScale.x, lmapTrans.y);
      break;

     // 0
     case Rotate:
      ent->m_col0.set(0,                         -lmapScale.y);
      ent->m_col1.set(-lmapScale.x,              0);
      ent->m_col2.set(lmapTrans.x + lmapScale.x, lmapTrans.y + lmapScale.y);
      break;

     // 0
     case Rotate | FlipX:
      ent->m_col0.set(0,                         -lmapScale.y);
      ent->m_col1.set(lmapScale.x,               0);
      ent->m_col2.set(lmapTrans.x + lmapScale.x, lmapTrans.y);
      break;

     // 0
     case Rotate | FlipY:
      ent->m_col0.set(0,            lmapScale.y);
      ent->m_col1.set(-lmapScale.x, 0);
      ent->m_col2.set(lmapTrans.x,  lmapTrans.y + lmapScale.y);
      break;

     // 0
     case Rotate | FlipX | FlipY:
      ent->m_col0.set(0,           lmapScale.y);
      ent->m_col1.set(lmapScale.x, 0);
      ent->m_col2.set(lmapTrans.x, lmapTrans.y);
      break;
   }

   delete ent->pLightmapCacheInfo;
   ent->pLightmapCacheInfo = io_pCacheInfo;
   AssertFatal(io_pCacheInfo->bitmapSequenceNum == 0xfffffffd, "Bad terrain cacheinfo");

   pHandleCache->HashInsert(ent);
   precacheTextureHandle(pSurface, ent);
}


void 
RegisterTextureCB(GFXSurface*      io_pSurface,
                  GFXTextureHandle in_handle,
                  GFXCacheCallback in_cb,
                  int              in_csizeX,
                  int              /*in_csizeY*/,
                  int              /*in_lightScale*/,
                  GFXLightMap*     io_pLightMap)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   AssertFatal(pSurface->getHandleCache() != NULL, "Error - surface not locked");
   
   Surface::TextureCache* pTxCache = pSurface->getTextureCache();
   HandleCache* pHandleCache = pSurface->getHandleCache();
   HandleCacheEntry* ent     = pHandleCache->getFreeEntry(pTxCache);
   
   ent->handle = in_handle;
   ent->flags  = 0;
   ent->cb     = in_cb;
   ent->size   = in_csizeX;
   ent->pBmp   = NULL;
   ent->pLMap  = io_pLightMap;
   ent->isTerrain = false;

   ent->coordScale.set(1.0f, 1.0f);
   ent->coordTrans.set(0.0f, 0.0f);

   float spow   = float(max(getNextPow2(io_pLightMap->size.x), 8UL));
   float scale  = (io_pLightMap->size.x - 1) / spow;
   float offset = 0.5f / spow; // half a texel offset;

   Point2F lmapScale, lmapTrans;
   lmapScale.set(scale,  scale);
   lmapTrans.set(offset, offset);

   ent->m_col0.set(lmapScale.x, 0);
   ent->m_col1.set(0,           lmapScale.y);
   ent->m_col2.set(lmapTrans.x, lmapTrans.y);

   pHandleCache->HashInsert(ent);
   precacheTextureHandle(pSurface, ent);
}


void 
RegisterTextureCBTer(GFXSurface*           io_pSurface,
                     GFXTextureHandle      in_handle,
                     GFXCacheCallback      in_cb,
                     int                   in_csizeX,
                     GFXLightMap*          io_pLightMap,
                     GFXBitmap::CacheInfo* io_pCacheInfo,
                     const RectI&          in_rSubSection)
{
   AssertFatal(in_rSubSection.isValidRect(), "Error, bad subsection");
   AssertFatal(dynamic_cast<Surface*>(io_pSurface) != NULL, "Error, not an opengl surface!");
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   AssertFatal(pSurface->getHandleCache() != NULL, "Error - surface not locked");
   
   Surface::TextureCache* pTxCache = pSurface->getTextureCache();
   HandleCache* pHandleCache = pSurface->getHandleCache();
   HandleCacheEntry* ent     = pHandleCache->getFreeEntry(pTxCache);
   
   ent->handle    = in_handle;
   ent->flags     = 0;
   ent->cb        = in_cb;
   ent->size      = in_csizeX;
   ent->pBmp      = NULL;
   ent->pLMap     = io_pLightMap;
   ent->isTerrain = true;

   ent->coordScale.set(1.0f, 1.0f);
   ent->coordTrans.set(0.0f, 0.0f);

   Int32 realSizeX = io_pLightMap->size.x ? io_pLightMap->size.x : 256;
   Int32 realSizeY = io_pLightMap->size.y ? io_pLightMap->size.y : 256;
   if (realSizeX == 256) {
      AssertFatal(io_pCacheInfo->bitmapSequenceNum == 0xfffffffd,
                  "WTF!");
   }

   AssertFatal(realSizeX == realSizeY, "Invalid terrain lightmap, must be square");
   AssertFatal((getNextPow2(realSizeX) == UInt32(realSizeX) &&
                getNextPow2(realSizeY) == UInt32(realSizeY)),
               "Invalid terrain lightmap.  Must be pow2.");

   float realDim = float(max(realSizeX, 8L));

   // Altimor: unused
   //float m0[3][3];
   //float temp[3][3];

   Point2F lmapScale, lmapTrans;
   lmapTrans.x = (float(in_rSubSection.upperL.x) + 0.5f) / realDim;
   lmapTrans.y = (float(in_rSubSection.upperL.y) + 0.5f) / realDim;

   lmapScale.x = float(in_rSubSection.lowerR.x - in_rSubSection.upperL.x) / realDim;
   lmapScale.y = float(in_rSubSection.lowerR.y - in_rSubSection.upperL.y) / realDim;

   ent->m_col0.set(lmapScale.x, 0);
   ent->m_col1.set(0,           -lmapScale.y);
   ent->m_col2.set(lmapTrans.x, lmapTrans.y + lmapScale.y);

   delete ent->pLightmapCacheInfo;
   ent->pLightmapCacheInfo = io_pCacheInfo;
   AssertFatal(io_pCacheInfo->bitmapSequenceNum == 0xfffffffd, "Bad terrain cacheinfo");

   pHandleCache->HashInsert(ent);
   precacheTextureHandle(pSurface, ent);
}

void
RegisterTextureTerCover(GFXSurface*           io_pSurface,
                        GFXTextureHandle      in_handle,
                        GFXLightMap*          io_pLightMap,
                        const GFXBitmap*      in_pTexture,
                        GFXBitmap::CacheInfo* io_pCacheInfo)
{
   AssertFatal(dynamic_cast<Surface*>(io_pSurface) != NULL, "Error, not an opengl surface!");
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   Surface::TextureCache* pTxCache = pSurface->getTextureCache();
   HandleCache* pHandleCache       = pSurface->getHandleCache();
   HandleCacheEntry* ent           = pHandleCache->getFreeEntry(pTxCache);

   ent->handle    = in_handle;
   ent->flags     = 0;
   ent->cb        = NULL;
   ent->pBmp      = in_pTexture;
   ent->pLMap     = io_pLightMap;
   ent->isTerrain = true;

   ent->coordScale.set(1.0f, 1.0f);
   ent->coordTrans.set(0.0f, 0.0f);

   Int32 realSizeX = io_pLightMap->size.x ? io_pLightMap->size.x : 256;
   Int32 realSizeY = io_pLightMap->size.y ? io_pLightMap->size.y : 256;
   if (realSizeX == 256) {
      AssertFatal(io_pCacheInfo->bitmapSequenceNum == 0xfffffffd,
                  "WTF!");
   }

   AssertFatal(realSizeX == realSizeY, "Invalid terrain lightmap, must be square");
   AssertFatal((getNextPow2(realSizeX) == UInt32(realSizeX) &&
                getNextPow2(realSizeY) == UInt32(realSizeY)),
               "Invalid terrain lightmap.  Must be pow2.");

   float realDim = float(max(realSizeX, 8L));

   Point2F lmapScale, lmapTrans;
   lmapTrans.x = 0.5f / realDim;
   lmapTrans.y = 0.5f / realDim;

   lmapScale.x = float(realDim - 1) / realDim;
   lmapScale.y = float(realDim - 1) / realDim;

   ent->m_col0.set(lmapScale.x, 0);
   ent->m_col1.set(0,           -lmapScale.y);
   ent->m_col2.set(lmapTrans.x, lmapTrans.y + lmapScale.y);

   delete ent->pLightmapCacheInfo;
   ent->pLightmapCacheInfo = io_pCacheInfo;
   AssertFatal(io_pCacheInfo->bitmapSequenceNum == 0xfffffffd, "Bad terrain cacheinfo");

   pHandleCache->HashInsert(ent);
   precacheTextureHandle(pSurface, ent);
}


Bool 
SetTextureHandle(GFXSurface*      io_pSurface,
                 GFXTextureHandle in_handle)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(pSurface->getHandleCache() != NULL, "error, no handle cache");
   
   HandleCache* pHandleCache = pSurface->getHandleCache();

   return pHandleCache->setTextureHandle(in_handle);
}


void 
HandleSetLightMap(GFXSurface*  io_pSurface,
                  int          /*in_lightScale*/,
                  GFXLightMap* io_pLightMap)
{
   Surface*               pSurface      = static_cast<Surface*>(io_pSurface);
   HandleCache*           pHandleCache  = pSurface->getHandleCache();
   Surface::TextureCache* pTextureCache = pSurface->getTextureCache();

   AssertFatal(pHandleCache->getCurrentEntry() != NULL, "No current entry?");
   HandleCacheEntry* pEntry = pHandleCache->getCurrentEntry();

   if (pEntry->pLMap != NULL) {
      pTextureCache->flushLightMap(*pEntry->pLightmapCacheInfo);
      gfxLightMapCache.release(pEntry->pLMap);
   }
   
   pEntry->pLMap  = io_pLightMap;
   pEntry->flags &= ~HandleCacheEntry::LightMapValid;
   precacheTextureHandle(pSurface, pEntry);
}

void 
HandleSetLightMapTer(GFXSurface*           io_pSurface,
                     GFXLightMap*          io_pLightMap,
                     GFXBitmap::CacheInfo* io_pCacheInfo)
{
   Surface*               pSurface      = static_cast<Surface*>(io_pSurface);
   HandleCache*           pHandleCache  = pSurface->getHandleCache();
   Surface::TextureCache* pTextureCache = pSurface->getTextureCache();

   AssertFatal(pHandleCache->getCurrentEntry() != NULL, "No current entry?");
   HandleCacheEntry* pEntry = pHandleCache->getCurrentEntry();

   if (pEntry->pLightmapCacheInfo->bitmapSequenceNum == 0xfffffffd) {
      // Terrain lightmap, don't flush unless the lightmap pointers
      //  are different..
      //
      if (pEntry->pLMap != io_pLightMap) {
         // Flush the sucker!
         pTextureCache->flushLightMap(*pEntry->pLightmapCacheInfo);
         gfxLightMapCache.release(pEntry->pLMap);
      }
   } else {
      // Other lightmap, flush away!
      pTextureCache->flushLightMap(*pEntry->pLightmapCacheInfo);
      gfxLightMapCache.release(pEntry->pLMap);
   }

   // Clear out the cacheinfo pointer, if we have a replacement...
   if (io_pCacheInfo != NULL) {
      if (pEntry->pLightmapCacheInfo->bitmapSequenceNum != 0xfffffffd) {
         delete pEntry->pLightmapCacheInfo;
      }
      pEntry->pLightmapCacheInfo = io_pCacheInfo;
   }
   
   pEntry->pLMap  = io_pLightMap;
   pEntry->flags &= ~HandleCacheEntry::LightMapValid;
   precacheTextureHandle(pSurface, pEntry);
}


void 
HandleSetTextureSize(GFXSurface* io_pSurface,
                     int         in_newSize)
{
   // Set the texturesize of the current texture handle
   // Only used for callback textures

   Surface* pSurface         = static_cast<Surface*>(io_pSurface);
   HandleCache* pHandleCache = pSurface->getHandleCache();

   AssertFatal(pHandleCache->getCurrentEntry() != NULL, "No current entry?");
   HandleCacheEntry* pEntry = pHandleCache->getCurrentEntry();

   if (pEntry->cb) {
      pEntry->size = in_newSize;
   }
}


void 
FlushTextureCache(GFXSurface* io_pSurface)
{
   // Remove all textures from the card...
   //
   Surface* pSurface                    = static_cast<Surface*>(io_pSurface);
   Surface::TextureCache* pTextureCache = pSurface->getTextureCache();
   HandleCache* pHandleCache            = pSurface->getHandleCache();

   if (pHandleCache  != NULL) pHandleCache->flush();
   if (pTextureCache != NULL) pTextureCache->flushCache();

   OGL_ERROR_CHECK(__LINE__);
}


void 
FlushTexture(GFXSurface*      in_pSurface,
             const GFXBitmap* in_pTexture,
             const bool       /*in_reload*/)
{
   if (in_pTexture == NULL)
      return;

//   AssertFatal(in_reload == true, "flush not supported");

   // Remove specific texture from the cache.
   //
   Surface* pSurface = static_cast<Surface*>(in_pSurface);
   pSurface->getTextureCache()->refreshBitmap(in_pTexture, in_pTexture->getCacheInfo());

   OGL_ERROR_CHECK(__LINE__);
}


GFXLightMap* 
HandleGetLightMap(GFXSurface* io_pSurface)
{
   Surface* pSurface         = static_cast<Surface*>(io_pSurface);
   HandleCache* pHandleCache = pSurface->getHandleCache();
   HandleCacheEntry* pEntry  = pHandleCache->getCurrentEntry();

   if (pEntry != NULL) {
      return pEntry->pLMap;
   } else {
      return NULL;
   }
}

void
SetClipPlanes(GFXSurface* io_pSurface,
              const float in_nearDist,
              const float in_farDist)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   pSurface->m_nearClipPlane = in_nearDist;
   pSurface->m_farClipPlane  = in_farDist;
}

//------------------------------------------------------------------------------
#define _BEGIN_IMPLEMENTED_
//------------------------------------------------------------------------------

void 
Draw3dBegin(GFXSurface* io_pSurface)
{
   // Prepare for drawing 3d
   GFXMetrics.reset();

   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   Surface::TextureCache* pTxCache = pSurface->getTextureCache();

   pTxCache->enableZBuffering(true, true, GL_LEQUAL);
   pSurface->m_in3dmode = true;

   g_texDownloadThisFrame = 0;
   g_lmDownloadThisFrame  = 0;
   g_lmDownloadBytes      = 0;
   g_oglEntriesTouched    = 0;
   g_oglFrameKey++;

   glEnable(GL_CULL_FACE);
   glFrontFace(GL_CW);
}


void 
Draw3dEnd(GFXSurface* io_pSurface)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   g_hazeTriCache.flushCache(pSurface);

   if (g_oglFrameKeyNum != 0) {
      g_oglAverageFrameKey = float(g_oglFrameKeyAccum) / float(g_oglFrameKeyNum);
   } else {
      g_oglAverageFrameKey = 0.0f;
   }

   Surface::TextureCache* pTxCache = pSurface->getTextureCache();

   pTxCache->enableZBuffering(true, false, GL_ALWAYS);
   pSurface->m_in3dmode = false;
}


void 
ClearScreen(GFXSurface* io_pSurface,
            DWORD       in_color)
{
   // Mostly obsolete, usually a background fill poly is drawn instead...
   //
   AssertFatal(io_pSurface->getPalette(), "No palette attached");
   
   PALETTEENTRY* pColors = io_pSurface->getPalette()->palette[0].color;
   glClearColor(GLfloat(pColors[in_color].peRed)   / 255.0f,
                GLfloat(pColors[in_color].peGreen) / 255.0f,
                GLfloat(pColors[in_color].peBlue)  / 255.0f,
                GLfloat(0.0f));
   glClear(GL_COLOR_BUFFER_BIT);

   OGL_ERROR_CHECK(__LINE__);
}


void 
ClearZBuffer(GFXSurface* /*io_pSurface*/)
{
   // Mostly obsolete, usually a background fill poly is drawn instead...
   //
   glClearDepth(0.0f);
   glClear(GL_DEPTH_BUFFER_BIT);

   OGL_ERROR_CHECK(__LINE__);
}


void 
SetFillColorCF(GFXSurface*   io_pSurface,
               const ColorF* in_pColor)
{
   static_cast<Surface*>(io_pSurface)->m_fillColor = *in_pColor;
}

void 
SetHazeColorCF(GFXSurface*   io_pSurface,
               const ColorF* in_pColor)
{
   static_cast<Surface*>(io_pSurface)->m_hazeColor = *in_pColor;

   GLfloat colorParams[4] = {
      in_pColor->red,
      in_pColor->green,
      in_pColor->blue,
      1.0f
   };
   glFogfv(GL_FOG_COLOR, colorParams);

   OGL_ERROR_CHECK(__LINE__);
}

void 
SetConstantShadeCF(GFXSurface*   io_pSurface,
                   const ColorF* in_pShadeColor)
{
   static_cast<Surface*>(io_pSurface)->m_constantShadeColor = *in_pShadeColor;
}


void 
SetConstantHaze(GFXSurface* io_pSurface,
                float       in_haze)
{
   static_cast<Surface*>(io_pSurface)->m_constantHaze = in_haze;

//   // One part in 1000 should be accurate enough...
//   //
//   GLfloat start = (in_haze) * -1000.0f;
//   GLfloat end   = (1.0f - in_haze) * 1000.0f;
//
//   glFogf(GL_FOG_START, start);
//   glFogf(GL_FOG_END,   end);
}


void 
SetConstantAlpha(GFXSurface* io_pSurface,
                 float       in_alpha)
{
   static_cast<Surface*>(io_pSurface)->m_constantAlpha = in_alpha;
}


void 
DrawPoint(GFXSurface*    io_pSurface,
          const Point2I* in_pt,
          float          in_w,
          DWORD          in_color)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   Surface::TextureCache* pTxCache = pSurface->getTextureCache();
   AssertFatal(pSurface->getPalette(), "No palette attached");

   // Calc z value...
   float oglZ  = (2.0f * in_w) - 1.0f;

   pTxCache->enableTexUnits(false);
   pTxCache->setBlendMode(GL_ONE, GL_ZERO);
   pTxCache->setTransparent(false);
   setupGLConstantIndexedColor(pSurface, in_color);

   glBegin(GL_POINTS);
      glVertex3f(GLfloat(in_pt->x) + 0.5,
                 GLfloat(in_pt->y) + 0.5,
                 oglZ);
   glEnd();

   OGL_ERROR_CHECK(__LINE__);
}


void 
DrawLine2d(GFXSurface*    io_pSurface,
           const Point2I* in_st,
           const Point2I* in_en,
           DWORD          in_color)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   Surface::TextureCache* pTxCache = pSurface->getTextureCache();
   AssertFatal(pSurface->getPalette(), "No palette attached");

   Point2I start = *in_st, end = *in_en;

   if(!rectClip(&start, &end, io_pSurface->getClipRect() ))
         return;

   pTxCache->enableTexUnits(false);
   pTxCache->setBlendMode(GL_ONE, GL_ZERO);
   pTxCache->setTransparent(false);
   setupGLConstantIndexedColor(pSurface, in_color);
   glBegin(GL_LINES);
      glVertex2f(GLfloat(start.x) + 0.5f,
                 GLfloat(start.y) + 0.5f);
      glVertex2f(GLfloat(end.x)   + 0.5f,
                 GLfloat(end.y)   + 0.5f);
   glEnd();
   glBegin(GL_POINTS);
      glVertex2f(GLfloat(start.x) + 0.45,
                 GLfloat(start.y) + 0.45);
      glVertex2f(GLfloat(end.x) + 0.45,
                 GLfloat(end.y) + 0.45);
   glEnd();

   OGL_ERROR_CHECK(__LINE__);
}


void 
DrawRect2d(GFXSurface*  io_pSurface,
           const RectI* in_pRect,
           DWORD        in_color)
{
   DrawLine2d(io_pSurface, &in_pRect->upperL, &Point2I(in_pRect->lowerR.x, in_pRect->upperL.y), in_color);
   DrawLine2d(io_pSurface, &Point2I(in_pRect->lowerR.x, in_pRect->upperL.y), &in_pRect->lowerR, in_color);
   DrawLine2d(io_pSurface, &in_pRect->lowerR, &Point2I(in_pRect->upperL.x, in_pRect->lowerR.y), in_color);
   DrawLine2d(io_pSurface, &Point2I(in_pRect->upperL.x, in_pRect->lowerR.y), &in_pRect->upperL, in_color);
}


void 
DrawRect_f(GFXSurface*  io_pSurface,
           const RectI* in_pRect,
           float        in_w,
           DWORD        in_color)
{
   RectI clippedRect = *in_pRect;
   if(!rectClip(&clippedRect, io_pSurface->getClipRect()))
      return;

   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   Surface::TextureCache* pTxCache = pSurface->getTextureCache();
   AssertFatal(pSurface->getPalette(), "No palette attached");

   DGLVertex4F* pVerts = pSurface->getCurrentVertex();

   pVerts[0].x = GLfloat(clippedRect.upperL.x + 0);
   pVerts[0].y = GLfloat(clippedRect.upperL.y + 0);
   pVerts[1].x = GLfloat(clippedRect.lowerR.x + 1);
   pVerts[1].y = GLfloat(clippedRect.upperL.y + 0);
   pVerts[2].x = GLfloat(clippedRect.lowerR.x + 1);
   pVerts[2].y = GLfloat(clippedRect.lowerR.y + 1);
   pVerts[3].x = GLfloat(clippedRect.upperL.x + 0);
   pVerts[3].y = GLfloat(clippedRect.lowerR.y + 1);

   float oglZ  = (2.0f * in_w) - 1.0f;

   int i;
   for(i = 0; i < 4; i++)
      pVerts[i].z = oglZ;

   for (i = 0; i < 4; i++)
      pSurface->incCurrVertexIndex();

   pTxCache->enableTexUnits(false);

   if (pSurface->m_fillMode      != GFX_ALPHA_FILL &&
       pSurface->m_constantAlpha == 1.0f)
      pTxCache->setBlendMode(GL_ONE, GL_ZERO);
   else
      pTxCache->setBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   pTxCache->setTransparent(false);
   glBegin(GL_TRIANGLE_FAN);
   setupGLConstantIndexedColor(pSurface, in_color);
   for (i = 0; i < pSurface->getCurrVertexIndex(); i++) {
      glVertex3fv((GLfloat*)&pSurface->m_pVertexArray[i]);
   }
   glEnd();
   OGL_ERROR_CHECK(__LINE__);

   pSurface->clearCurrVertexIndex();
}


void 
SetZTest(GFXSurface* io_pSurface,
         int         in_enable)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   Surface::TextureCache* pTxCache = pSurface->getTextureCache();

   pSurface->m_zTest = (GFXZBufferMode)in_enable;

#ifdef FAKE_W_BUFFER
   const GLenum test = GL_GEQUAL;
#else
   const GLenum test = GL_LEQUAL;
#endif

   switch (in_enable) {
     case GFX_NO_ZTEST:
      pTxCache->enableZBuffering(false, false, test);
      break;

     case GFX_ZTEST:
      pTxCache->enableZBuffering(true, false, test);
      break;

     case GFX_ZTEST_AND_WRITE:
      pTxCache->enableZBuffering(true, true, test);
      break;

     case GFX_ZWRITE:
     case GFX_ZALWAYSBEHIND:
      pTxCache->enableZBuffering(true, true, GL_ALWAYS);
      break;

     default:
      AssertFatal(0, "unknown zBuffer mode in OGL driver");
   }

   OGL_ERROR_CHECK(__LINE__);
}


// Fill modes, shade, haze and alpha sources are listed in inc\d_defs.h
//
void 
SetAlphaSource(GFXSurface*    io_pSurface,
               GFXAlphaSource in_as)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   Surface::TextureCache* pTxCache = pSurface->getTextureCache();
   pSurface->m_alphaSource = in_as;
   pSurface->m_constantAlpha = 1.0f;

   // GFX_ALPHA_NONE:     Draw fully opaque
   //          _CONSTANT: Draw at level set by SetConstantAlpha
   //          _VERTEX:   Unused currently
   //          _TEXTURE:  Draw translucent texture
   //          _FILL:     Draw a solid color index from the Palette transColor
   //                      (g_pal.h : 112) table.  (Index is send through
   //                      setFillColorI (alpha value is in peFlags)
   //
   switch (in_as) {
     case GFX_ALPHA_NONE:
      pTxCache->setBlendMode(GL_ONE, GL_ZERO);
      break;

     case GFX_ALPHA_CONSTANT:
     case GFX_ALPHA_VERTEX:
      pTxCache->setBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      pTxCache->setTexMode(GL_MODULATE);
      break;

     case GFX_ALPHA_FILL:
      pTxCache->setBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      break;

     case GFX_ALPHA_TEXTURE:
      pTxCache->setBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      pTxCache->setTexMode(GL_MODULATE);
      break;
     
     case GFX_ALPHA_ADD:
      if (g_prefOGLNoAddFade == true) {
         pTxCache->setBlendMode(GL_ONE, GL_ONE);
      } else {
         pTxCache->setBlendMode(GL_SRC_ALPHA, GL_ONE);
      }
      pTxCache->setTexMode(GL_MODULATE);
      break;

     case GFX_ALPHA_SUB:
      pTxCache->setBlendMode(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
      pTxCache->setTexMode(GL_MODULATE);
      break;

     default:
      AssertFatal(0, "Unknown alphaSource");
   }

   OGL_ERROR_CHECK(__LINE__);
}


void 
SetShadeSource(GFXSurface*    io_pSurface,
               GFXShadeSource in_ss)
{
   // GFX_SHADE_NONE:     Draw at full brightness
   //          _CONSTANT: Draw at level set by SetConstantShadeCF
   //          _VERTEX:   Gouraud shaded by values sent to AddVertexVTC
   //
   //
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   Surface::TextureCache* pTxCache = pSurface->getTextureCache();

   pSurface->m_shadeSource = in_ss;
   pSurface->m_constantShadeColor.set(1, 1, 1);

   switch (in_ss) {
     case GFX_SHADE_NONE:
      pSurface->m_colorEnabled = false;
      pTxCache->setTexMode(GL_REPLACE);
      break;

     case GFX_SHADE_CONSTANT:
      pSurface->m_colorEnabled = false;
      pTxCache->setTexMode(GL_MODULATE);
      break;

     case GFX_SHADE_VERTEX:
      pSurface->m_colorEnabled = true;
      pTxCache->setTexMode(GL_MODULATE);
      break;

     default:
      AssertFatal(0, "unknown shadeSource");
      break;
   }

   OGL_ERROR_CHECK(__LINE__);
}


void 
SetHazeSource(GFXSurface*   io_pSurface,
              GFXHazeSource in_hs)
{
   // GFX_HAZE_NONE:     Draw with no haze
   //         _CONSTANT: Draw at level set by SetConstantHaze
   //         _VERTEX:   Gouraud shaded by values sent to AddVertexVTC
   //
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   Surface::TextureCache* pTxCache = pSurface->getTextureCache();
   pSurface->m_hazeSource = in_hs;

   pTxCache->enableFog(false);

   OGL_ERROR_CHECK(__LINE__);
}


void 
SetTextureWrap(GFXSurface* io_pSurface,
               Bool        in_wrapEnable)
{
   // Probably not necessary in hardware.  No texture coordinates that would
   //  cause edge clamping are passed.
   //
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   
   if (pSurface->getTextureCache()->supportsEdgeClamp() == false) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, in_wrapEnable == false ? GL_CLAMP : GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, in_wrapEnable == false ? GL_CLAMP : GL_REPEAT);
   } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, in_wrapEnable == false ? GL_CLAMP_TO_EDGE : GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, in_wrapEnable == false ? GL_CLAMP_TO_EDGE : GL_REPEAT);
   }

   OGL_ERROR_CHECK(__LINE__);
}


void 
DrawBitmap2d_f(GFXSurface*      io_pSurface,
               const GFXBitmap* in_pBM,
               const Point2I*   in_at,
               GFXFlipFlag      in_flip)
{
   if(!setupBitmapPoints(io_pSurface,
                         in_at->x, in_at->y,
                         in_pBM->getWidth(), in_pBM->getHeight())) {
      return;
   }
   
   if (in_flip != GFX_FLIP_NONE) {
      float coords[4];
      coords[0] = 0.0f;
      coords[1] = 0.0f;
      coords[2] = float(in_pBM->getWidth());
      coords[3] = float(in_pBM->getHeight());
      
      float temp;
      if ((in_flip & GFX_FLIP_X) != 0) {
         temp = coords[2];
         coords[2] = coords[0];
         coords[0] = temp;
      }
      if ((in_flip & GFX_FLIP_Y) != 0) {
         temp = coords[1];
         coords[1] = coords[3];
         coords[3] = temp;
      }
      setupBitmapTexCoords(io_pSurface, in_pBM, coords[0], coords[1], coords[2], coords[3]);
   } else {
      setupBitmapTexCoords(io_pSurface, in_pBM, 0.0f, 0.0f, float(in_pBM->getWidth()), float(in_pBM->getHeight()));
   }
   Surface* pSurface      = static_cast<Surface*>(io_pSurface);
   for (int i = 0; i < 4; i++)
      pSurface->incCurrVertexIndex();
   
   bitmapDraw(io_pSurface, in_pBM);
   OGL_ERROR_CHECK(__LINE__);
}


void 
DrawBitmap2d_rf(GFXSurface*      io_pSurface,
                const GFXBitmap* in_pBM,
                const RectI*     in_subRegion,
                const Point2I*   in_at,
                GFXFlipFlag      in_flip)
{
   if(!setupBitmapPoints(io_pSurface,
                         in_at->x, in_at->y,
                         in_subRegion->len_x() + 1, in_subRegion->len_y() + 1)) {
      return;
   }
   
   if (in_flip != GFX_FLIP_NONE) {
      float coords[4];
      coords[0] = float(in_subRegion->upperL.x);
      coords[1] = float(in_subRegion->upperL.y);
      coords[2] = float(in_subRegion->lowerR.x + 1);
      coords[3] = float(in_subRegion->lowerR.y + 1);
      
      float temp;
      if ((in_flip & GFX_FLIP_X) != 0) {
         temp = coords[2];
         coords[2] = coords[0];
         coords[0] = temp;
      }
      if ((in_flip & GFX_FLIP_Y) != 0) {
         temp = coords[1];
         coords[1] = coords[3];
         coords[3] = temp;
      }
      setupBitmapTexCoords(io_pSurface, in_pBM, coords[0], coords[1], coords[2], coords[3]);
   } else {
      setupBitmapTexCoords(io_pSurface, in_pBM, 
                           float(in_subRegion->upperL.x), float(in_subRegion->upperL.y),
                           float(in_subRegion->lowerR.x) + 1, float(in_subRegion->lowerR.y + 1));
   }
   Surface* pSurface      = static_cast<Surface*>(io_pSurface);
   for (int i = 0; i < 4; i++)
      pSurface->incCurrVertexIndex();

   bitmapDraw(io_pSurface, in_pBM);
   OGL_ERROR_CHECK(__LINE__);
}


void 
DrawBitmap2d_sf(GFXSurface*      io_pSurface,
                const GFXBitmap* in_pBM,
                const Point2I*   in_at,
                const Point2I*   in_stretch,
                GFXFlipFlag      in_flip)
{
   if(!setupBitmapPoints(io_pSurface,
                         in_at->x, in_at->y,
                         in_stretch->x, in_stretch->y)) {
      return;
   }
   
   if (in_flip != GFX_FLIP_NONE) {
      float coords[4];
      coords[0] = 0.0f;
      coords[1] = 0.0f;
      coords[2] = float(in_pBM->getWidth());
      coords[3] = float(in_pBM->getHeight());
      
      float temp;
      if ((in_flip & GFX_FLIP_X) != 0) {
         temp = coords[2];
         coords[2] = coords[0];
         coords[0] = temp;
      }
      if ((in_flip & GFX_FLIP_Y) != 0) {
         temp = coords[1];
         coords[1] = coords[3];
         coords[3] = temp;
      }
      setupBitmapTexCoords(io_pSurface, in_pBM, coords[0], coords[1], coords[2], coords[3]);
   } else {
      setupBitmapTexCoords(io_pSurface, in_pBM, 0.0f, 0.0f, float(in_pBM->getWidth()), float(in_pBM->getHeight()));
   }
   Surface* pSurface      = static_cast<Surface*>(io_pSurface);
   for (int i = 0; i < 4; i++)
      pSurface->incCurrVertexIndex();

   bitmapDraw(io_pSurface, in_pBM);
   OGL_ERROR_CHECK(__LINE__);
}


void
SetZMode(GFXSurface* /*io_pSurface*/,
         bool        /*wBuffer*/)
{
   //
}


//------------------------------------------------------------------------------
// All functions below this point are probably unnecessary for hardware...
//
//--------------------------------------
void 
HandleSetMipLevel(GFXSurface*  /*io_pSurface*/,
                  int          /*in_mipLevel*/)
{
   // Not necessary for hardware in which mips are downloaded and selected
   //  automatically
}

void 
AddVertexV(GFXSurface*    io_pSurface,
           const Point3F* in_pVert,
           DWORD          in_sofwareEdgeKey)
{
   // Obsolete: pass to full function...
   AddVertexVTC(io_pSurface, in_pVert, NULL, NULL, in_sofwareEdgeKey);
}

void 
AddVertexVT(GFXSurface*    io_pSurface,
            const Point3F* in_pVert,
            const Point2F* in_pTex,
            DWORD          in_sofwareEdgeKey)
{
   // Obsolete: pass to full function...
   AddVertexVTC(io_pSurface, in_pVert, in_pTex, NULL, in_sofwareEdgeKey);
}

void 
AddVertexVC(GFXSurface*          io_pSurface,
            const Point3F*       in_pVert,
            const GFXColorInfoF* in_pColor,
            DWORD                in_sofwareEdgeKey)
{
   // Obsolete: pass to full function...
   AddVertexVTC(io_pSurface, in_pVert, NULL, in_pColor, in_sofwareEdgeKey);
}

//------------------------------------------------------------------------------
// void SetFillColorI(GFXSurface* io_pSurface, Int32 in_index)
// void SetHazeColorI(GFXSurface* io_pSurface, Int32 in_index)
// void SetConstantShadeF(GFXSurface* io_pSurface, float in_shade);
//
//  These functions are actually mostly useless in hardware, default is to just
// convert them to their absolute color equivalent, and pass them to the
// corresponding CF function.
//
//------------------------------------------------------------------------------
//
void 
SetFillColorI(GFXSurface* io_pSurface,
              Int32       in_index,
              DWORD       in_paletteIndex)
{
   // Recommended form for hardware...
   //
   AssertFatal(io_pSurface->getPalette() != NULL, "No palette attached to surface");

   GFXPalette::MultiPalette* pMultiPalette =
      io_pSurface->getPalette()->findMultiPalette(in_paletteIndex);

   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   Surface::TextureCache* pTxCache = pSurface->getTextureCache();

   float gamma = pTxCache->m_gamma;

   PALETTEENTRY* pColors = pMultiPalette->color;

   ColorF fillColor;
   fillColor.set(pow(float(pColors[in_index].peRed)   / 255.0f, 1.0f / gamma),
                 pow(float(pColors[in_index].peGreen) / 255.0f, 1.0f / gamma),
                 pow(float(pColors[in_index].peBlue)  / 255.0f, 1.0f / gamma));

   if (pSurface->m_alphaSource == GFX_ALPHA_FILL) {
      SetConstantAlpha(io_pSurface, float(pColors[in_index].peFlags) / 255.0f);
   }

   SetFillColorCF(io_pSurface, &fillColor);
}

void 
SetHazeColorI(GFXSurface* io_pSurface,
              Int32       in_index)
{
   // Recommended form for hardware...
   //
   AssertFatal(io_pSurface->getPalette() != NULL, "No palette attached to surface");

   PALETTEENTRY* pColors = io_pSurface->getPalette()->palette[0].color;
   ColorF hazeColor;
   hazeColor.set(float(pColors[in_index].peRed)   / 255.0f,
                 float(pColors[in_index].peGreen) / 255.0f,
                 float(pColors[in_index].peBlue)  / 255.0f);
   SetHazeColorCF(io_pSurface, &hazeColor);
}

void 
SetConstantShadeF(GFXSurface* io_pSurface,
                  float       in_shade)
{
   // Recommended form for hardware...
   //
   ColorF shadeColor;
   shadeColor.set(in_shade, in_shade, in_shade);
   SetConstantShadeCF(io_pSurface, &shadeColor);
}

void 
SetTexturePerspective(GFXSurface* /*io_pSurface*/,
                      Bool        /*perspTex*/)
{
   // Maybe not necessary for accelerated surface?  All vertices have w
   //  infomation passed to AddVertexVTC
   //
   // Always persp correct for OGL
}

void 
HandleSetTextureMap(GFXSurface*      /*io_pSurface*/,
                    const GFXBitmap* /*in_pTexture*/)
{
   // Set the texturemap of the current texture handle
}

GFXHazeSource
GetHazeSource(GFXSurface* io_pSurface)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   return pSurface->m_hazeSource;
}

float
GetConstantHaze(GFXSurface* io_pSurface)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   return pSurface->m_constantHaze;
}

} // namespace OpenGL

FunctionTable opengl_table =
{
   OpenGL::ClearScreen,    

   OpenGL::DrawPoint,
   OpenGL::DrawLine2d,
   OpenGL::DrawRect2d,
   OpenGL::DrawRect_f,

   OpenGL::DrawBitmap2d_f,
   OpenGL::DrawBitmap2d_rf,
   OpenGL::DrawBitmap2d_sf,
   NULL,

   GFXDrawText_p,    // Unnecessary to modify
   GFXDrawText_r,    // Unnecessary to modify

   OpenGL::Draw3dBegin,
   OpenGL::Draw3dEnd,
   OpenGL::AddVertexV,
   OpenGL::AddVertexVT,
   OpenGL::AddVertexVC,
   OpenGL::AddVertexVTC,
   OpenGL::EmitPoly,
   OpenGL::SetShadeSource,
   OpenGL::SetHazeSource,
   OpenGL::SetAlphaSource,
   OpenGL::SetFillColorCF,
   OpenGL::SetFillColorI,
   OpenGL::SetHazeColorCF,
   OpenGL::SetHazeColorI,
   OpenGL::SetConstantShadeCF,
   OpenGL::SetConstantShadeF,
   OpenGL::SetConstantHaze,
   OpenGL::SetConstantAlpha,
   OpenGL::SetTransparency,
   OpenGL::SetTextureMap,
   OpenGL::SetFillMode,
   OpenGL::SetTexturePerspective,

   OpenGL::RegisterTexture,
   OpenGL::RegisterTextureCB,
   OpenGL::SetTextureHandle,
   GFXAllocateLightMap,          // Unnecessary to modify
   OpenGL::HandleGetLightMap,
   OpenGL::HandleSetLightMap,
   OpenGL::HandleSetTextureMap,
   OpenGL::HandleSetMipLevel,
   OpenGL::HandleSetTextureSize,
   OpenGL::FlushTextureCache,
   OpenGL::SetZTest,
   OpenGL::ClearZBuffer,
   OpenGL::SetTextureWrap,

   OpenGL::FlushTexture,
   OpenGL::SetZMode,

   OpenGL::SetClipPlanes,

   OpenGL::GetHazeSource,
   OpenGL::GetConstantHaze
};

