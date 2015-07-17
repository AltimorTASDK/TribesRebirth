//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "gOGLTCache.h"

namespace OpenGL {

TriCache::TriCache()
 : m_vertexBuffer(4096 / sizeof(DGLVertex4F)),
   m_texCoordBuffer(4096 / sizeof(Point2F)),
   m_colorBuffer(4096 / sizeof(DGLColor4F)),
   m_hazeBuffer(4096 / sizeof(float)),

   m_assembledBuffer(4096 / sizeof(AssembledVertex)),

   m_alwaysBehindConst(4096 / sizeof(TriFan)),
   m_alwaysBehindTexture(4096 / sizeof(TriFan)),

   m_twoPass(4096 / sizeof(TriFan)),
   m_normalConst(4096 / sizeof(TriFan)),
   m_normalTex(4096 / sizeof(TriFan)),
   m_normalLight(4096 / sizeof(TriFan)),

   m_alphaPolys(4096 / sizeof(TriFan)),

   m_beginIndex(0xFFFFFFFF),
   m_polyType(NormalConst)
{
   //
}

TriCache::~TriCache()
{
   AssertFatal(m_assembledBuffer.size() == 0, "Error, cache not flushed out on delete");
}

void
TriCache::beginFan(const PolyType in_polyType)
{
   m_beginIndex = m_assembledBuffer.size();
   m_polyType   = in_polyType;
}

//   Vector<TriFan> m_alwaysBehindConst;
//   Vector<TriFan> m_alwaysBehindTexture;
//
//   Vector<TriFan> m_twoPass;
//   Vector<TriFan> m_normalConst;
//   Vector<TriFan> m_normalTex;
//   Vector<TriFan> m_normalLight;
//   Vector<TriFan> m_normalHaze;
//
//   Vector<TriFan> m_alphaPolys;

void
TriCache::endFan(Surface*)
{
   AssertFatal(m_beginIndex != 0xFFFFFFFF, "Error, poly not begun");

   TriFan* pTriFan;
   switch(m_polyType) {
     case AlwaysBehindConst:
      m_alwaysBehindConst.increment();
      pTriFan = &(m_alwaysBehindConst.last());
      break;

     case AlwaysBehindTexture:
      m_alwaysBehindTexture.increment();
      pTriFan = &(m_alwaysBehindTexture.last());
      break;

     case TwoPass:
      m_twoPass.increment();
      pTriFan = &(m_twoPass.last());
      break;

     case NormalConst:
      m_normalHaze.increment();
      pTriFan = &(m_normalHaze.last());
      break;

     case NormalTex:
      m_normalTex.increment();
      pTriFan = &(m_normalTex.last());
      break;

     case NormalLightmap:
      m_normalLight.increment();
      pTriFan = &(m_normalLight.last());
      break;

     case NormalHaze:
      m_normalHaze.increment();
      pTriFan = &(m_normalHaze.last());
      break;

     case AlphaPoly:
      m_alphaPolys.increment();
      pTriFan = &(m_alphaPolys.last());
      break;

     case AlphaPolyTex:
      m_alphaPolys.increment();
      pTriFan = &(m_alphaPolys.last());
      break;

     default:
      AssertFatal(false, "Bad poly type");
   }

   pTriFan->vertexStart = m_beginIndex;
   pTriFan->vertexEnd   = m_assembledBuffer.size() - 1;

   m_beginIndex = 0xFFFFFFFF;
}

void
TriCache::addVertex(const float* in_pCoord)
{
   AssertFatal(m_beginIndex != 0xFFFFFFFF, "Error, poly not begun");

   m_assembledBuffer.increment();
   AssembledVertex& rAVert = m_assembledBuffer.last();

   m_vertexBuffer.increment();
   DGLVertex4F& rVert = m_vertexBuffer.last();

   memcpy(&rVert, in_pCoord, sizeof(float) * 4);

   rAVert.vertexIndex = m_vertexBuffer.size() - 1;
}

void
TriCache::addTexCoord(const float* in_pTexCoord)
{
   AssertFatal(m_beginIndex != 0xFFFFFFFF, "Error, poly not begun");
   AssertFatal(m_beginIndex <= (UInt32)m_assembledBuffer.size(),
               "Error, must addVertex first");

   AssembledVertex& rAVert = m_assembledBuffer.last();

   m_texCoordBuffer.increment();
   Point2F& rCoord = m_texCoordBuffer.last();

   memcpy(&rCoord, in_pTexCoord, sizeof(float) * 2);

   rAVert.texCoordIndex = m_texCoordBuffer.size() - 1;
}

void
TriCache::addColor(const float* in_pColor)
{
   AssertFatal(m_beginIndex != 0xFFFFFFFF, "Error, poly not begun");
   AssertFatal(m_beginIndex <= (UInt32)m_assembledBuffer.size(),
               "Error, must addVertex first");

   AssembledVertex& rAVert = m_assembledBuffer.last();

   m_colorBuffer.increment();
   DGLColor4F& rColor = m_colorBuffer.last();

   memcpy(&rColor, in_pColor, sizeof(float) * 4);

   rAVert.colorIndex = m_colorBuffer.size() - 1;
}

void
TriCache::addHaze(const float in_haze)
{
   AssertFatal(m_beginIndex != 0xFFFFFFFF, "Error, poly not begun");
   AssertFatal(m_beginIndex <= (UInt32)m_assembledBuffer.size(),
               "Error, must addVertex first");

   AssembledVertex& rAVert = m_assembledBuffer.last();

   m_hazeBuffer.increment();
   float& rHaze = m_hazeBuffer.last();

   rHaze = in_haze;

   rAVert.hazeIndex = m_hazeBuffer.size() - 1;
}

void
TriCache::flushCache(Surface*)
{
   AssertFatal(m_beginIndex == 0xFFFFFFFF, "Error, poly started during cache flush");

   // For now, just clear out the buffers...
   //
   m_vertexBuffer.setSize(0);
   m_texCoordBuffer.setSize(0);
   m_colorBuffer.setSize(0);
   m_hazeBuffer.setSize(0);
   m_assembledBuffer.setSize(0);
   m_alwaysBehindConst.setSize(0);
   m_alwaysBehindTexture.setSize(0);
   m_twoPass.setSize(0);
   m_normalConst.setSize(0);
   m_normalTex.setSize(0);
   m_normalLight.setSize(0);
   m_normalHaze.setSize(0);
   m_alphaPolys.setSize(0);

   m_polyType   = AlwaysBehindConst;
}

} // namespace OpenGL