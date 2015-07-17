
#ifndef _TS_SHADOW_H_
#define _TS_SHADOW_H_

#include "ts_types.h"
#include "ts_poly.h"
#include "ts_vertex.h"
#include "ts_renderContext.h"

class  GFXDeviceManager;
class  GFXBitmap;
class  GFXSurface;
class  GFXPalette;

namespace TS
{

class  ObjectInfo;
class  MaterialList;
class  PointArray;
class  RenderContext;
class  OrthographicCamera;
class  SceneLighting;

struct PlaneEntry;

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class ShapeInstance;

struct TexturedPolyVertex: public TPolyVertex
{
   Point2F texture;
   void set(TexturedPolyVertex & start, TexturedPolyVertex & end, float scale);
};

inline void TexturedPolyVertex::set(TexturedPolyVertex& start,
                                    TexturedPolyVertex& end,float scale)
{
   TPolyVertex::set(start,end,scale);
   texture.x = start.texture.x + (end.texture.x - start.texture.x) * scale;
   texture.y = start.texture.y + (end.texture.y - start.texture.y) * scale;
}

class TexturedPoly : public TPoly<TexturedPolyVertex>
{
public:
   typedef TPoly<TexturedPolyVertex>::VertexList VertexList;
};

class Poly : public TPoly<TPolyVertex>
{
public:
   void set(Point3F &v1,Point3F &v2,Point3F &v3)
   {
      vertexList.setSize(3);
      vertexList[0].point=v1;
      vertexList[1].point=v2;
      vertexList[2].point=v3;
   }
   int operator < (Poly const & p);
   int planeIndex;
   float minT;
   float maxT;
};

class DLLAPI PolyList : public SortableVectorPtr<Poly*>
{
public:
   void clearPolys();

   ~PolyList();
};

struct PlaneEntry
{
   Point3F fNormal;
   float fPlaneConstant;
   float fInvDot;
   bool faceCamera;
};


class DLLAPI Shadow
{
   // for creating shadow bitmap
   GFXBitmap * shadowBmp;
   GFXSurface * surface;
   OrthographicCamera * camera;
   RenderContext renderContext;
   SceneLighting * lights;

   // set when we re-draw the bitmap   
   bool newBmp;
   int bmpDim;

   // set whenever shape moves in the world
   Point3F posInWorld;

   // the following get set whenever light is set
   // light need not be re-set if it doesn't change
   // (unless shape changes)
   Point3F ul,ur,ll,lr;
   Point3F vRight, vUp, shadowPolyCenter;
   TMat3F tc;
   RMat3F TLW;

   // floor info
   bool useFloor;
   Point3F floorNormal;
   Point3F floorPoint;

   // set when we build the bitmap
   float shadowRadius;

   // translucency
   float alphaLevel;

   void blurBitmap(int blurMethod);
   void blurBitmap2();
   void blurBitmap3();
   void blurBitmap5();
   void blurBitmap23();
   
   struct Cache
   {
      Vector<int> nSides;
      Vector<Point2F> projectedTextures;
      Vector<Point3F> projectedPoints;
   } cache;

   void renderPlane( PointArray * pa, PlaneEntry & pe);
   void renderPartition(PointArray *);

   void buildPartitionTree();
   bool getPoly(TexturedPoly * p);
   void project( const Point3F & direction, const Point3F & normal, 
                 float planeConstant, float invDot);

public:
   // tolerance and method parameters -- for debugging...shouldn't change
   static float tol1;
   static float tol2;
   static float tol3;
   static float tol5;
   static int doNullClip;

   // these variables are work variables and should not be manipulated directly
   Vector<PlaneEntry> planeStack;
   static Vector<int> nSides;
   static Vector<Point2F> projectedTextures;
   static Vector<Point3F> projectedPoints;
   static Point3F sourcePoints[4];
   static Point2F sourceTextures[4];

   // for internal use only
   // needs to be public so bsp tree can see it
	Point3F lightInWorld;

   // for internal use only
   inline void project( const Point3F & direction, const Point3F & normal,
                        float planeConstant, float invDot,
                        const Point3F & v, Point3F & pv);

public:
   // The following methods and variables are the intended public interface...
   
   // list of polys to project shadow onto
   PolyList projectionList;

   // how much to lift shadow off surfaces...
	float shadowLift;
   
   // this is the largest dot product of the light and plane normal (* -1)
   // from the previous render frame -- it's used for selecting shadow detail
   float maxPlaneDot;

   // light to world transform...
   RMat3F & getTLW() { return TLW; }
   
   // radius of shadow bitmap -- somewhat smaller than shape's radius
   float getShadowRadius() { return shadowRadius; }
   
   //
   float getAlphaLevel() { return alphaLevel; }
   
   // a plane can be set as the floor -- nothing below floor will cast shadow...
   // plane is in shape space
   void setFloor(Point3F & p, Point3F & n) 
      { floorPoint = p; floorNormal = n; useFloor = true; }
   void clearFloor() { useFloor = false; }

   // only call these when there's a change
   void setBitmapSize(GFXDeviceManager *, int bmpDim, GFXSurface * flushSurface = NULL);
   void setAlphaLevel(float _alpha);
   void setLight(const Point3F & lightDirection, ShapeInstance *);

   // call every frame
   void setPosition(const Point3F &);

   //  call as often as you like...
   void getShadowBitmap(ShapeInstance *, GFXPalette *, RMat3F &, int blurMethod = 3);

   void getPlanes(const Point3F & cameraCenter, const Point3F & camY);
   void render( RenderContext & rc );
   
   // cache projection polys...
   void cachePolys();
   void renderCache( RenderContext & rc);
   
   // used internally but publicly available too
   void calcSourceWindow(ShapeInstance*,RMat3F &);

   Shadow();

   // call before destruction...
   void clearResources(GFXDeviceManager *, bool clearCameraToo = true);
};


inline void Shadow::project( const Point3F & direction, const Point3F & planeNormal, 
                             float planeConstant, float invDot,
                             const Point3F & v, Point3F & pv)
{
	pv = direction;
	float t = (planeConstant - m_dot(v,planeNormal)) * invDot;
	pv *= t;
	pv += v;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

}; // namespace TS


#endif