//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GOGLTCACHE_H_
#define _GOGLTCACHE_H_

#include "types.h"
#include "base.h"
#include "tVector.h"
#include "gOGLSfc.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

namespace OpenGL {

class TriCache
{
  public:
   enum PolyType {
      AlwaysBehindConst,
      AlwaysBehindTexture,

      TwoPass,
      NormalConst,
      NormalTex,
      NormalLightmap,
      NormalHaze,

      AlphaPoly,
      AlphaPolyTex
   };

  private:
   struct AssembledVertex {
      UInt32 vertexIndex;
      UInt32 texCoordIndex;
      UInt32 colorIndex;
      UInt32 hazeIndex;
   };
   
   struct TriFan {
      UInt32 vertexStart;
      UInt32 vertexEnd;

      UInt32 tex0;
      UInt32 tex1;

      GLenum src;
      GLenum dst;

      DGLColor4F color;
      UInt32 alphaMode;
   };

   Vector<DGLVertex4F> m_vertexBuffer;
   Vector<Point2F>     m_texCoordBuffer;
   Vector<DGLColor4F>  m_colorBuffer;
   Vector<float>       m_hazeBuffer;

   Vector<AssembledVertex> m_assembledBuffer;

   Vector<TriFan> m_alwaysBehindConst;
   Vector<TriFan> m_alwaysBehindTexture;

   Vector<TriFan> m_twoPass;
   Vector<TriFan> m_normalConst;
   Vector<TriFan> m_normalTex;
   Vector<TriFan> m_normalLight;
   Vector<TriFan> m_normalHaze;

   Vector<TriFan> m_alphaPolys;

   UInt32   m_beginIndex;
   PolyType m_polyType;

  public:
   TriCache();
   ~TriCache();

   void flushCache(Surface*);

   void beginFan(const PolyType);
   void endFan(Surface*);

   void addVertex(const float* in_pCoord);
   void addTexCoord(const float* in_pTexCoord);
   void addColor(const float* in_pColor);
   void addHaze(const float in_haze);
};

} // namespace OpenGL



#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif // _GOGLTCACHE_H_
