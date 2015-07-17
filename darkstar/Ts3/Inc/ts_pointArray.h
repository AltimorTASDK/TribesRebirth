//---------------------------------------------------------------------------

// $Workfile$
// $Revision$
// $Version$
// $Date$

//---------------------------------------------------------------------------

#ifndef _TS_POINTARRAY_H_
#define _TS_POINTARRAY_H_

#include <m_point.h>
#include <ts_camera.h>
#include <ts_RenderContext.h>
#include <ts_vertex.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

namespace TS {

class DLLAPI PointArray
{
   enum EdgeKey {
      ResetKeySize = 9,
      IndexKeySize = 11,
   };

   Vector<TransformedVertex>  fVerts;

   Vector<int> fIndex;
   int fEdgeResetKey;

   int (__cdecl *clipIntersectFunc)(TransformedVertex *p1, TransformedVertex *p2, TransformedVertex *dest, float farDist, int flags, int planeIndex, RectF *,bool);

   bool isOrtho;
   Point2F const *   fpTextureArray;   // array of source texture coords
   Bool              fUseBackFaceTest;
   int               fVisibility;
   RenderContext *   fpRC;
   float             farDist;
   RectF             screenViewport;
   GFXSurface *      surface;
   int numClippingPlanes;
   int usesFlags;

   void   copy( VertexIndexPair const &ip1, int offset );
   void   close();
   UInt32 buildEdgeKey(int a, int b);

   void drawTriangleClip( const VertexIndexPair *, int offset);

  public:
   enum useFlags {
      usesTextures    = 1 << 0,
      usesIntensities = 1 << 1,
      usesHazes       = 1 << 2,
   };

   PointArray( );
   PointArray( RenderContext *context);
   PointArray( GFXSurface *sfc, RectF const &viewport);
   ~PointArray();

   void reset();
   void useTextures( Point2F const * textures );
   void useTextures( Bool state );
   void useHazes( Bool state );
   void useIntensities( Bool state );
   void useBackFaceTest( Bool state );
   void setVisibility( int vis );
   void setFarDist(float farDist);

   void setRenderContext( RenderContext* prc);
   TransformedVertex & getTransformedVertex( int index );

   void drawLine (int v1, int v2, int color);

   bool testPolyVis( int count, VertexIndexPair const * indices,
                             int offset );

   void drawPoly( int count, VertexIndexPair const * indices,
                          int offset );
   void drawTriangle( const VertexIndexPair *, int offset);
   void drawProjectedPoly (int count, VertexIndexPair const *indices,
                                   int offset );

   int  addPoints( int count, PackedVertex const *verts, 
   Point3F const &scale, Point3F const origin, int *vis );

   int  addPoint( Point3F const &point);
   int  addPoints( int count, const Point3F * point);
   int  addProjectedPoint( Point3F const &point);

   int  addUnusedPoint();

   void lightPoint( int index, PackedVertex const &vert, 
                            Point3F const &scale, Point3F const origin );
   void setOrtho(bool in_ortho) { isOrtho = in_ortho; }
   bool getOrtho() { return isOrtho; }
};

inline int
PointArray::addUnusedPoint()
{
   int newIndex = fVerts.size();

   fVerts.increment();
   return newIndex;
}


inline void PointArray::setRenderContext( RenderContext* prc)
{
   fpRC = prc;
   RectI const &r = fpRC->getCamera()->getScreenViewport();
   screenViewport.upperL.x = float(r.upperL.x);
   screenViewport.upperL.y = float(r.upperL.y);
   screenViewport.lowerR.x = float(r.lowerR.x);
   screenViewport.lowerR.y = float(r.lowerR.y);
   surface = fpRC->getSurface();
}

inline void PointArray::useTextures( Point2F const * textures )
{
   fpTextureArray = textures;
}

inline void PointArray::useTextures( Bool state )
{
   if(state)
      usesFlags |= usesTextures;
   else
      usesFlags &= ~usesTextures;
}

inline void PointArray::useIntensities( Bool state )
{
   if(state)
      usesFlags |= usesIntensities;
   else
      usesFlags &= ~usesIntensities;
}

inline void PointArray::useBackFaceTest( Bool state )
{
   fUseBackFaceTest = state;
}

inline void PointArray::useHazes( Bool state )
{
   if(state)
      usesFlags |= usesHazes;
   else
      usesFlags &= ~usesHazes;
}


inline void PointArray::setVisibility( int vis )
{
   fVisibility = vis;
}

inline TransformedVertex & PointArray::getTransformedVertex( int index )
{
   return fVerts[index];
}

//---------------------------------------------------------------------------
// add point to transformed/projected vertex array

inline int PointArray::addPoint( Point3F const &point )
{
   AssertFatal( fpRC != 0,
      "TS::PointArray::addPoint: No render context installed" );

   // make space for the new point:
   fVerts.increment();
   // transform & project the point:
   fpRC->getCamera()->transformProject( point, &fVerts[fVerts.size() -1]);
   return fVerts.size() - 1;
}


//--------------------------------------------------------------------------
// light vertex in array

inline void PointArray::lightPoint( int index, PackedVertex const &vert, 
   Point3F const &scale, Point3F const origin )
{
   // don't do this more than once:
   if( fVerts[index].fStatus & TransformedVertex::Lit )
      return;

   // Unpack vertex:
   Vertex uvert;
   uvert.set( vert, scale, origin );

   fVerts[index].calcLighting( *fpRC, uvert );
}

};  // namespace TS


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef TS::PointArray TSPointArray;

#endif // _TS_POINTARRAY_H_
