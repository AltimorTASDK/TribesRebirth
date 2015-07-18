//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <stdlib.h>

#include <core.h>
#include <g_bitmap.h>
#include <bitstream.h>

#include "itrgeometry.h"
#include "itrcollision.h"
#include "itrlighting.h"
#include "itrbasiclighting.h"
#include "tplane.h"
#include "itrbit.h"
#include "m_random.h"

//----------------------------------------------------------------------------

#define MIN_INTENSITY (1.0f / 16.0f)

bool itrInverse(TMat3F& tmat);

inline float
getMax(float in_one, float in_two)
{
   return ((in_one >= in_two) ? in_one : in_two);
}


//----------------------------------------------------------------------------

namespace ITRBasicLighting
{
   enum BoxMapping {
      MapYZ,
      MapXZ,
      MapXY,
   };
   struct LightMap {
      enum Constants {
         MaxWidth  = 256,
         MaxHeight = 256,
      };

     private:
      Color* color;
      
     public:
      Point2I size;
      //
      ~LightMap();
      LightMap();
      void clear(const Point2I& size);
      void test();
      
      Color& getColor(int x, int y) {
         AssertFatal(x < size.x && y < size.y, "out of bounds...");
         return color[y*size.x + x];
      }
   };
   struct StateLightMap {
      enum Constants {
         MaxWidth = 256,
         MaxHeight = 256,
      };
     private:
      Color* color;

     public:
      Point2I size;
      //
      StateLightMap();
      ~StateLightMap();
      void clear(const Point2I& size);
      void test();
      Color& getColor(int x, int y) { return color[y * size.x + x]; }
   };
   //
   struct MapData {
      Point2I offset;
      Point2I size;
      TMat3F  transform;
      float   scale;
      
      MapPointList* mapPointList;
      
      //
      ~MapData();
      MapData();
      bool set(ITRLighting&          lighting,
               ITRGeometry&          geometry,
               ITRGeometry::Surface& surface);
      void buildPointList(ITRGeometry&          geometry,
                          ITRGeometry::Surface& surface,
                          bool                  clampPoints = true);
      void clampPointList(ITRGeometry&          geometry,
                          ITRGeometry::Surface& surface);
      void markPointList(ITRGeometry&          geometry,
                         ITRGeometry::Surface& surface);
   };
   //
   static int _USERENTRY compareLights(const void* a,const void* b);

   int exportMap(LightMap&    map,
                 ITRLighting* lighting,
                 bool&        didCompress);
   void exportSurfStaticLMap(ITRLighting* lighting,
                             int          ss);
   int exportStateMap(StateLightMap& map,
                      ITRLighting*   lighting);


   void processAnimations(LightList& lights);
   void exportLights(LightList&   map,
                     ITRLighting* lighting);
   void lightSurface(ITRGeometry&      geometry,
                     LightList&        lights,
                     MaterialPropList& matProps,
                     ITRLighting*      lighting,
                     int               ss );
   void lightSurface( ITRGeometry & geometry,
      LightList & lights, ITRLighting * lighting, int ss1, 
      ITRMissionLighting * missionLighting, int ss2, 
      CallbackInfo * callback );
   
   void emitLightFromSurface(ITRGeometry& io_geometry,
                             MapData&     io_mapData,
                             LightMap&    io_emissionLightmap,
                             Vector<int>* io_pPlaneSurfaceIndex,
                             const int    in_surfaceNum);
   void emitSurfaceLight(ITRGeometry&      geometry,
                         MaterialPropList& matProps);
   void processUniqueLights(LightList& lights);
   bool areEqualStates(const Light::State* pStateOne,
                       const Light::State* pStateTwo);
   //
   bool  lightFilter = false;
   float filterScale = 0.0f;
   int   lightScale  = 4;

   bool  g_useNormal = false;
//   bool  g_testPattern = false;
   bool  g_useLightMaps = false;
   bool  g_missionLighting = false;

   float distancePrecision = 0.01f;
   float normalPrecision   = 0.0001f;
   ColorF m_ambientIntensity = ColorF( 0.f, 0.f, 0.f );
   bool g_applyAmbientOutside = true;

   // I hate to use globals for this, but until the next rewrite of the tool,
   //  it's the simplest way...
   //
   Vector<LightMap> sg_staticLightMaps;
   Vector<MapData>  sg_staticMapData;

   //  Distance beyond which a point is not considered to be on
   // a plane...
   const float distTolerance = 1e-4f;

   int g_quantumNumber = 100;
   
   // Random number generator...
   //
   Random sg_random;

   struct Huffman16Node {
      UInt32 pop;

      Int32   indexZero;
      Int32   indexOne;
   };
   struct Huffman16Leaf {
      UInt32 pop;

      UInt8  code[16];    // assume that a code will never exceed 128 bits...
      UInt16 codeLength;
      UInt16 color;
   };
   
   Vector<Huffman16Node> sg_huffman16Nodes;
   Vector<Huffman16Leaf> sg_huffman16Leaves;

   class Huffman16Wrap {
      Int32        index;
      Huffman16Leaf* pLeaf;
      Huffman16Node* pNode;

     public:
      Huffman16Wrap(const Int32 in_index) : index(in_index) {
         if (in_index < 0) {
            pNode = NULL;
            pLeaf = &sg_huffman16Leaves[-(index+1)];
         } else {
            pNode = &sg_huffman16Nodes[index];
            pLeaf = NULL;
         }
      }

      UInt32 getPop() const {
         if (pNode != NULL)
            return pNode->pop;
         else
            return pLeaf->pop;
      }
      Int32 getIndex() const {
         return index;
      }
   };
   void createStaticHuffman16Codes(ITRLighting*);
   void exportHuffman16Tables(ITRLighting*);
   void generateHuffman16Recurse(const Huffman16Node& in_rNode,
                               UInt8              io_codeBuffer[16],
                               UInt32             in_codeLength);

//   struct Huffman8Node {
//      UInt32 pop;
//
//      Int32   indexZero;
//      Int32   indexOne;
//   };
//   struct Huffman8Leaf {
//      UInt32 pop;
//
//      UInt8  code[16];    // assume that a code will never exceed 128 bits...
//      UInt16 codeLength;
//      UInt8 color;
//   };
//   
//   Vector<Huffman8Node> sg_huffman8Nodes;
//   Vector<Huffman8Leaf> sg_huffman8Leaves;
//
//   class Huffman8Wrap {
//      Int32        index;
//      Huffman8Leaf* pLeaf;
//      Huffman8Node* pNode;
//
//     public:
//      Huffman8Wrap(const Int32 in_index) : index(in_index) {
//         if (in_index < 0) {
//            pNode = NULL;
//            pLeaf = &sg_huffman8Leaves[-(index+1)];
//         } else {
//            pNode = &sg_huffman8Nodes[index];
//            pLeaf = NULL;
//         }
//      }
//
//      UInt32 getPop() const {
//         if (pNode != NULL)
//            return pNode->pop;
//         else
//            return pLeaf->pop;
//      }
//      Int32 getIndex() const {
//         return index;
//      }
//   };
//   void createStaticHuffman8Codes(ITRLighting*);
//   void exportHuffman8Tables(ITRLighting*);
//   void generateHuffman8Recurse(const Huffman8Node& in_rNode,
//                                UInt8               io_codeBuffer[16],
//                                UInt32              in_codeLength);

   bool g_disableCompression = false;
};

//------------------------------------------------------------------------------
//-------------------------------------- Implementation
//------------------------------------------------------------------------------

static Point3F sg_collisionPoint;
bool findCollisionSurface(ITRGeometry*   in_geometry,
                          TMat3F*        in_transform,
                          const Point3F& start,
                          const Point3F& end,
                          int*           out_surfaceIndex);

void
getCosRandDeflectedNormal(const Point3F& in_normal,
                          Point3F& deflectedNormal,
                          float phongExp = 1.0)
{
   using ITRBasicLighting::sg_random;

   float cosRandTheta;
   float cosRandPhi;
   float rand1 = float(sg_random.getInt());
   float rand2 = float(sg_random.getInt());
   float epsilon1 = rand1 / float(0xffffffff);
   float epsilon2 = rand2 / float(0xffffffff);

   cosRandTheta = acos(powf((1.0f - epsilon1),1.0/(1+phongExp)));
   cosRandPhi   = 2 * M_PI * epsilon2;

   // Deflect the surface normal
   //
   EulerF angle;
   RMat3F rotMatrix;
   Point3F unitYVector, randCosVector;
   deflectedNormal.set(0.0f,0.0f,0.0f);

   angle.x = (M_PI/2.0) - cosRandTheta;
   angle.y = 0.0f;
   angle.z = cosRandPhi;
   unitYVector.x = 0.0f;
   unitYVector.y = 1.0f;
   unitYVector.z = 0.0f;
   angle.makeMatrix(&rotMatrix);
   m_mul(unitYVector, rotMatrix, &randCosVector);

   Point3F xPrime, yPrime, zPrime, temp;
   RMat3F tempRot;

   zPrime.x = in_normal.x;
   zPrime.y = in_normal.y;
   zPrime.z = in_normal.z;
   tempRot.set(EulerF(float(M_PI)/1.36f, float(M_PI)/.56f, float(M_PI)/6.5f));
   m_mul(zPrime, tempRot, &temp);
   m_cross(zPrime, temp, &yPrime);
   yPrime.normalize();
   m_cross(zPrime, yPrime, &xPrime);
   xPrime.normalize();

   xPrime *= randCosVector.x;
   yPrime *= randCosVector.y;
   zPrime *= randCosVector.z;
   deflectedNormal += xPrime;
   deflectedNormal += yPrime;
   deflectedNormal += zPrime;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

ITRBasicLighting::MapData::MapData()
{
   mapPointList = NULL;
}  

ITRBasicLighting::MapData::~MapData()
{
   if (mapPointList != NULL)
      delete mapPointList;
   mapPointList = NULL;
}  


//----------------------------------------------------------------------------

inline ITRBasicLighting::Color&
ITRBasicLighting::Color::operator+=(const ITRBasicLighting::Color& cc)
{
   red   += cc.red;
   green += cc.green;
   blue  += cc.blue;
   return *this;
}

inline ITRBasicLighting::Color&
ITRBasicLighting::Color::operator*=(const float& in_mul)
{
   red   *= in_mul;
   green *= in_mul;
   blue  *= in_mul;
   return *this;
}


//----------------------------------------------------------------------------

Persistent::Base::Error
ITRBasicLighting::MaterialProp::read(StreamIO& io_sio,
                                     int       /*version*/,
                                     int       /*user*/)
{
   io_sio.read(&modified);

   int tempEnum;
   io_sio.read(&tempEnum);
   emissionType = EmissionType(tempEnum);
   
   io_sio.read(sizeof(Color), &color);
   io_sio.read(&intensity);
   
   io_sio.read(&modifiedPhong);
   io_sio.read(&phongExp);
   
   return ((io_sio.getStatus() == STRM_OK) ? Ok : ReadError);
}

Persistent::Base::Error
ITRBasicLighting::MaterialProp::write(StreamIO& io_sio,
                                      int       /*version*/,
                                      int       /*user*/)
{
   io_sio.write(modified);

   int tempEnum = emissionType;
   io_sio.write(tempEnum);
   
   io_sio.write(sizeof(Color), &color);
   io_sio.write(intensity);
   
   io_sio.write(modifiedPhong);
   io_sio.write(phongExp);
   
   return ((io_sio.getStatus() == STRM_OK) ? Ok : WriteError);
}


void
ITRBasicLighting::writeMaterialPropList(StreamIO&         io_rStream, 
                                        MaterialPropList& io_rMList)
{
   int listSize = io_rMList.size();
   io_rStream.write(listSize);
   
   for (int i = 0; i < listSize; i++) {
      io_rMList[i].write(io_rStream, 0, 0);
   }
}

void
ITRBasicLighting::readMaterialPropList(StreamIO&         io_rStream, 
                                       MaterialPropList& io_rMList)
{
   AssertFatal(io_rMList.size() == 0, "You don't really want to do that, do you?");

   int listSize;
   io_rStream.read(&listSize);
   io_rMList.setSize(listSize);
   
   for (int i = 0; i < listSize; i++) {
      new (&io_rMList[i]) MaterialProp;

      io_rMList[i].read(io_rStream, 0, 0);
   }
}


//----------------------------------------------------------------------------

ITRBasicLighting::LightMap::LightMap()
{
   color = NULL;
}

ITRBasicLighting::LightMap::~LightMap()
{
   if (color)
      delete [] color;
   color = NULL;
}

void ITRBasicLighting::LightMap::clear(const Point2I& psize)
{
   if (color != NULL)
      delete [] color;
      
   size = psize;
   color = new Color[size.y * size.x];
   memset(color, 0, size.y * size.x * sizeof(Color));
}

void ITRBasicLighting::LightMap::test()
{
   Color light,dark;
   light.red = light.green = light.blue = 1.0f;
   dark.red  = dark.green  = dark.blue  = 0.0f;

   // Fills the whole bitmap with the pattern.
   for (int y = 0; y < MaxHeight; y++)
      for (int x = 0; x < MaxWidth; x++)
         color[y * MaxWidth + x] =
            ((x & 1) ^ (y & 1))? light: dark;
}

ITRBasicLighting::StateLightMap::StateLightMap()
{
   color = NULL;
}

ITRBasicLighting::StateLightMap::~StateLightMap()
{
   if (color)
      delete [] color;
   color = NULL;
}

void ITRBasicLighting::StateLightMap::clear(const Point2I& psize)
{
   if (color != NULL)
      delete [] color;

   size = psize;
   color = new Color[size.y * size.x];
   memset(color, 0, size.y * size.x * sizeof(Color));
}

void ITRBasicLighting::StateLightMap::test()
{
   Color light,dark;
   light.red = light.green = light.blue = 1.0f;
   dark.red  = dark.green  = dark.blue  = 0.0f;

   // Fills the whole bitmap with the pattern.
   for (int y = 0; y < MaxHeight; y++)
      for (int x = 0; x < MaxWidth; x++)
         color[y * MaxWidth + x] =
            ((x & 1) ^ (y & 1))? light: dark;
}


double
ITRBasicLighting::Light::State::Emitter::getDistCoef(const Point3F& in_pos)
{
   float retCoef;
   float distance = m_dist(in_pos,pos);

//   printf("dist coef: (%f, %f) >-< %f\n", d1, d2, distance);

   // light falloff calculation...
   //
   if( lightType != DirectionalLight )
   {
      if (falloff == Distance) {
         // Standard quadratic falloff
         //
         retCoef = 1.0f / (d1 +  (d2 + distance * d3) * distance);
      } else if (falloff == Linear) {
         // Linear falloff
         if( distance < d1 )
            retCoef = 1.0f;
         else 
         {
            if (distance > d2)
               retCoef = 0.0f;
            else if( d1 != d2 )
            {
               AssertFatal( d2 > d1, "Second linear falloff distance before first" );
               retCoef = 1.0f - ((distance - d1) / (d2 - d1));
            }
         }
      }
   }
   
   if (lightType == SpotLight) {
      // Spot light
      Point3F temp;
      float projDist;

      temp  = in_pos;
      temp -= pos;
      projDist    = m_dot(spotVector, temp);
      float angle = acos(projDist/distance);

      if (angle <= spotTheta ) {
         // point is in maximally lit 
      } else if (angle > spotTheta && angle <= spotPhi) {
         // point is between inner and outer angles of the spotlight
         retCoef *= 1.0f - ((angle-spotTheta)/(spotPhi-spotTheta));
      } else {
         // point is totally outside spot
         retCoef = 0.0f;
      }
   } 
   else if( lightType == DirectionalLight )
   {
      retCoef = 1.0f;
   } else {
      AssertFatal(lightType == PointLight, "unknown lighttype...");
   }
   
   return retCoef;
}

//----------------------------------------------------------------------------
// Light all the points in the point list.
// Also does the point light averaging.
//
bool
ITRBasicLighting::Light::State::light(ITRGeometry&                     geometry,
                                      ITRBasicLighting::MapPointList&  pointList,
                                      Point3F&                         normal,
                                      ITRBasicLighting::LightMap*      map,
                                      ITRBasicLighting::CallbackInfo * callback )
{
   CollisionSurfaceList csList;
   ITRCollision los(&geometry);
   los.collisionList = &csList;
   
   bool lit = false;

   for (int i = 0; i < m_emitterList.size(); i++) {
      Emitter& rEmitter = m_emitterList[i];

      for (int p = 0; p < pointList.size(); ) {
         MapPoint& first = pointList[p];
         Color lcolor;
         lcolor.red = lcolor.blue = lcolor.green = 0.0f;

         // Average consecutive points
         for (int end = p + first.average; p < end; p++) {
            float ndot;
            MapPoint& point = pointList[p];

            // color that can be modified by the collision routine
            ColorF castColor;
            castColor.red   = color.red;
            castColor.green = color.green;
            castColor.blue  = color.blue;
            
            if( rEmitter.lightType == DirectionalLight )
            {
               Point3F pnt = rEmitter.spotVector;
               pnt *= -2000.f;
               pnt += point.pos;
               if( !los.collide( point.pos, pnt ) )
               {
                  // call missionlight collision callback 
                  if( callback )
                     if( callback->collide( callback->obj, point.pos, pnt, castColor ) )
                        continue;
                  
                  ndot = -m_dot( normal, rEmitter.spotVector );
                  
                  lcolor.red    += ( castColor.red   * ndot );
                  lcolor.green  += ( castColor.green * ndot );
                  lcolor.blue   += ( castColor.blue  * ndot);
               }
            }
            else
            {
               if (!los.collide(point.pos, rEmitter.pos)) 
               {
                  // call missionlight collision callback 
                  if( callback )
                  {
                     // check if hit something
                     if( callback->collide( callback->obj, point.pos, rEmitter.pos, castColor ) )
                        continue;
                  }
                  
                  if (g_useNormal) {
                     Point3F vec = rEmitter.pos;
                     vec -= point.pos;
                     vec.normalize();
                     ndot = m_dot(normal, vec);
                  } else {
                     ndot = 1.0f;
                  }
                   
                  float distCoef = rEmitter.getDistCoef(point.pos) * ndot;
                  
                  lcolor.red    += ( castColor.red   * distCoef );
                  lcolor.green  += ( castColor.green * distCoef );
                  lcolor.blue   += ( castColor.blue  * distCoef );
//                  printf("lit a point (%f, %f, %f) * %f = (%f, %f, %f)\n",
//                         castColor.red, castColor.green, castColor.blue,
//                         distCoef,
//                         lcolor.red, lcolor.green, lcolor.blue);
               }
            }
         }
         
         if (first.average > 1) {
            float scale = 1.0f / first.average;
            lcolor.red   *= scale;
            lcolor.green *= scale;
            lcolor.blue  *= scale;
         }

         // Only mark map as lit if some minimum amount of
         // light falls on it.
         float intensity = lcolor.red   * 0.299 +
                           lcolor.green * 0.587 +
                           lcolor.blue  * 0.114;
         if (intensity > MIN_INTENSITY)
            lit = true;

         // Just add the color in, it will get clamped later.
         map->getColor(first.mapX, first.mapY) += lcolor;
      }
   }
   
   return lit;
}

bool
ITRBasicLighting::Light::State::lightState(ITRGeometry&                     geometry,
                                           ITRBasicLighting::MapPointList&  pointList,
                                           Point3F&                         normal,
                                           ITRBasicLighting::StateLightMap* map)
{
   CollisionSurfaceList csList;
   ITRCollision los(&geometry);
   los.collisionList = &csList;
   
   bool lit = false;

   for (int i = 0; i < m_emitterList.size(); i++) {
      Emitter& rEmitter = m_emitterList[i];

      for (int p = 0; p < pointList.size(); ) {
         MapPoint& first = pointList[p];
         Color lcolor;
         lcolor.red = lcolor.blue = lcolor.green = 0.0f;

         // Average consecutive points
         for (int end = p + first.average; p < end; p++) {
            MapPoint& point = pointList[p];
               
            if (!los.collide(point.pos, rEmitter.pos)) {
               float ndot;
               if (g_useNormal) {
                  Point3F vec = rEmitter.pos;
                  vec -= point.pos;
                  vec.normalize();
                  ndot = m_dot(normal,vec);
               }
               else
                  ndot = 1.0f;

               float distCoef = rEmitter.getDistCoef(point.pos) * ndot;
               lcolor.red    += distCoef;
            }
         }
         
         if (first.average > 1) {
            float scale = 1.0f / first.average;
            lcolor.red   *= scale;
         }

         // Just add the color in, it will get clamped later.
         map->getColor(first.mapX, first.mapY) += lcolor;
         
         // Only mark map as lit if some minimum amount of
         // light falls on it.
         float intensity = lcolor.red;
         if (intensity > MIN_INTENSITY)
            lit = true;
      }
   }

   return lit;
}


//----------------------------------------------------------------------------
// Return true if the surface might be lit by this state
//
bool ITRBasicLighting::Light::State::Emitter::isLit(ITRGeometry& geometry,
   ITRGeometry::Surface& surface)
{
   TPlaneF& plane = geometry.planeList[surface.planeIndex];

   if( lightType == DirectionalLight )
   {
      float val = m_dot( plane, spotVector );
      return surface.planeFront ? ( val < 0.f ) : ( val > 0.f );
   }
   else
   {
      float distance = m_dot(plane,pos) + plane.d;
      if (!surface.planeFront)
         distance = -distance;
      if (distance < 0.0f || distance > radius)
         return false;
   }
   return true;
}


//----------------------------------------------------------------------------

void ITRBasicLighting::Light::State::appendSurface(int surf,int lightIndex,
   int mapIndex)
{
   surfaceData.increment();
   Light::State::Surface& sp = surfaceData.last();
   sp.surface    = surf;
   sp.lightIndex = lightIndex;
   sp.mapIndex   = mapIndex;
}

const ITRBasicLighting::Light&
ITRBasicLighting::Light::operator=(const ZedLight::PersLight&)
{
   return *this;
}

void
ITRBasicLighting::Light::scale(const float /*in_worldScale*/)
{
   // NULL
}

void
findMinMaxBoxMap(ITRGeometry&          geometry,
                 ITRGeometry::Surface& surface,
                 Point2I&              minMap,
                 Point2I&              maxMap)
{
   ITRGeometry::Vertex* vp = &geometry.vertexList[surface.vertexIndex];
   TPlaneF plane = geometry.planeList[surface.planeIndex];
   if (surface.planeFront == false)
      plane.neg();

   Point2F tMin(+1.0E20f,+1.0E20f);
   Point2F tMax(-1.0E20f,-1.0E20f);

   using namespace ITRBasicLighting;
   BoxMapping boxMapping;
   // Select box mapping
   if (fabs(plane.x) > fabs(plane.y))
      boxMapping = (fabs(plane.x) > fabs(plane.z)) ? MapYZ : MapXY;
   else
      boxMapping = (fabs(plane.y) > fabs(plane.z)) ? MapXZ : MapXY;

   // Map vertices to box face.
   for (int i = 0; i < surface.vertexCount; i++) {
      Point3F& tp = geometry.point3List[vp[i].pointIndex];
      Point2F mapped;
      switch(boxMapping) {
         case MapYZ:
            mapped.set(tp.y,-tp.z);
            break;
         case MapXY:
            mapped.set(tp.x,-tp.y);
            break;
         case MapXZ:
            mapped.set(tp.x,-tp.z);
            break;
      }
      tMin.setMin(mapped);
      tMax.setMax(mapped);
   }

   tMin.x = ceil(tMin.x - 0.499999f);  tMin.y = ceil(tMin.y - 0.499999f);
   tMax.x = floor(tMax.x + 0.499999f); tMax.y = floor(tMax.y + 0.499999f);

   minMap.x = Int32(tMin.x); minMap.y = Int32(tMin.y);
   maxMap.x = Int32(tMax.x); maxMap.y = Int32(tMax.y);
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Get light map size and offset and setup transform needed to
// create the point list (texture to world space mat.)
//
bool
ITRBasicLighting::MapData::set(ITRLighting&          lighting,
                               ITRGeometry&          geometry,
                               ITRGeometry::Surface& surface)
{
   ITRGeometry::Vertex* vp = &geometry.vertexList[surface.vertexIndex];

   scale = float(1 << lighting.lightScaleShift);

   Point2I screenTextureSize, minMap, maxMap;
   Point2I upperL, lowerR;
   Point2I leftop, rightbottom;

   findMinMaxBoxMap(geometry, surface, minMap, maxMap);
   screenTextureSize.x = maxMap.x - minMap.x;
   screenTextureSize.y = maxMap.y - minMap.y;

   // Light Maps are aligned.
   UInt32 iScale     = (1 << lighting.lightScaleShift);
   UInt32 scaleMask  = iScale - 1;
   int    half       = 1 << (lighting.lightScaleShift - 1);

   Point2I tSize;
   tSize.x = (surface.textureSize.x + 1) << geometry.highestMipLevel;
   tSize.y = (surface.textureSize.y + 1) << geometry.highestMipLevel;
   upperL.x = maxMap.x - tSize.x;
   upperL.y = maxMap.y - tSize.y;
   lowerR   = maxMap;

   leftop.x      = (upperL.x - half) & ~scaleMask;
   leftop.y      = (upperL.y - half) & ~scaleMask;
   rightbottom.x = ((lowerR.x-1 + half) & ~scaleMask) + iScale;
   rightbottom.y = ((lowerR.y-1 + half) & ~scaleMask) + iScale;

   offset.x = upperL.x - leftop.x - half;
   offset.y = upperL.y - leftop.y - half;

   size.x = rightbottom.x - leftop.x;
   size.y = rightbottom.y - leftop.y;
   size.x >>= lighting.lightScaleShift;
   size.y >>= lighting.lightScaleShift;

   // ============== Build texture to vector space
   Point2F tv[3];
   tv[0]    = geometry.point2List[vp[1].textureIndex];
   tv[1]    = geometry.point2List[vp[0].textureIndex];
   tv[2]    = geometry.point2List[vp[2].textureIndex];

   // Not sure why the commented values were not
   // surface.textureSize.x+1, since the values are stored
   // as size - 1.
   tv[0].x *= tSize.x; // surface.textureSize.x;
   tv[0].y *= tSize.y; // surface.textureSize.y;
   tv[1].x *= tSize.x; // surface.textureSize.x;
   tv[1].y *= tSize.y; // surface.textureSize.y;
   tv[2].x *= tSize.x; // surface.textureSize.x;
   tv[2].y *= tSize.y; // surface.textureSize.y;
   Point2F tv1 = tv[1]; tv1 -= tv[0];
   Point2F tv2 = tv[2]; tv2 -= tv[0];

   TMat3F tvs;
   tvs.flags = TMat3F::Matrix_HasRotation    | 
               TMat3F::Matrix_HasTranslation |
               TMat3F::Matrix_HasScale;

   tvs.m[0][0] = tv1.x;   tvs.m[0][1] = tv1.y;   tvs.m[0][2] = 0;
   tvs.m[1][0] = tv2.x;   tvs.m[1][1] = tv2.y;   tvs.m[1][2] = 0;
   tvs.m[2][0] = tv[0].x; tvs.m[2][1] = tv[0].y; tvs.m[2][2] = 1.0;
   tvs.p.set(.0f,.0f,.0f);

   if (!itrInverse(tvs)) {
      transform.identity();
      return false;
   }

   // Build vector space to world space
   Point3F vv[3];
   vv[0] = geometry.point3List[vp[1].pointIndex];
   vv[1] = geometry.point3List[vp[0].pointIndex];
   vv[2] = geometry.point3List[vp[2].pointIndex];
   Point3F vv1 = vv[1]; vv1 -= vv[0];
   Point3F vv2 = vv[2]; vv2 -= vv[0];

   TMat3F vws;
   vws.flags = TMat3F::Matrix_HasRotation;
   vws.m[0][0] = vv1.x;   vws.m[0][1] = vv1.y;   vws.m[0][2] = vv1.z;
   vws.m[1][0] = vv2.x;   vws.m[1][1] = vv2.y;   vws.m[1][2] = vv2.z;
   vws.m[2][0] = vv[0].x; vws.m[2][1] = vv[0].y; vws.m[2][2] = vv[0].z;
   vws.p.set(.0f,.0f,.0f);

   // Build texture to world space.
   m_mul(tvs,vws,&transform);
   return true;
}


//----------------------------------------------------------------------------
// Build the list of points that will be sample on the surface
//
void ITRBasicLighting::MapData::buildPointList(ITRGeometry&          geometry,
                                               ITRGeometry::Surface& surface,
                                               bool                  clampPoints)
{
   if (mapPointList == NULL) {
      mapPointList = new MapPointList;
   } else {
      // Already built...
      //
      return;
   }

   // Generate points in world space to be sampled.
   TPlaneF plane = geometry.planeList[surface.planeIndex];
   for (int y = 0; y < size.y; y++) {
      for (int x = 0; x < size.x; x++) {
         // Convert to texture coors.
         Point3F tp;
         tp.x = (x * scale) - offset.x;
         tp.y = (y * scale) - offset.y;
         tp.z = 1.0f;

         int count = 1;
         if (lightFilter) {
            tp.x -= (scale * filterScale * 0.5);
            tp.y -= (scale * filterScale * 0.5);
            count = 4;
         }
         for (int i = 0; i < count; i++) {
            mapPointList->increment();
            MapPoint& point = mapPointList->last();
            point.mapX = x;
            point.mapY = y;
            point.average = (count > 1 && i == 0)? 4: 1;

            Point3F xp = tp;
            xp.x += (i & 1)? scale * filterScale: 0.0f;
            xp.y += (i & 2)? scale * filterScale: 0.0f;
            Point3F wpos;
            m_mul(xp,transform,&wpos);

            // Snap the point on to the plane.
            float distance = plane.distance(wpos);
            point.pos = plane;
            point.pos *= -distance;
            point.pos += wpos;
         }
      }
   }
   
   if (clampPoints) {
      clampPointList(geometry, surface);
   } else {
      markPointList(geometry, surface);
   }
}


//----------------------------------------------------------------------------
// We don't want sample points that extend outside the poly if
// they pass through a solid leaf. This causes light and shadow bleeding.
//
void ITRBasicLighting::MapData::clampPointList(ITRGeometry&          geometry,
                                               ITRGeometry::Surface& surface)
{
   AssertFatal(mapPointList != NULL, "Must call buildpointlist first...");

   TPlaneF plane = geometry.planeList[surface.planeIndex];
   if (!surface.planeFront)
      plane.neg();

   // Build list of bounding planes for all polys
   Point3F polyCenter;
   Vector<TPlaneF> planeList;
   
   ITRGeometry::Vertex* vp = &geometry.vertexList[surface.vertexIndex];

   polyCenter.set(0.0f,0.0f,0.0f);

   Point3F *start = &geometry.point3List[vp[surface.vertexCount - 1].pointIndex];
   int i;
   for (i = 0; i < surface.vertexCount; i++) {
      Point3F* end = &geometry.point3List[vp[i].pointIndex];
      Point3F lnormal,lvec = *end; lvec -= *start;
      float len = lvec.len();
      if (len > distancePrecision) {
         lvec.normalize();
         m_cross(lvec,plane,&lnormal);
         if (!isEqual(lnormal.len(),.0f)) {
            planeList.increment();
            planeList.last().set(*start,lnormal);
         }
      }
      polyCenter += *end;
      start = end;
   }
   polyCenter *= 1.0f / float(surface.vertexCount);

   // Test all the points to see if they are inside a poly.
   CollisionSurfaceList csList;
   ITRCollision los(&geometry);
   los.collisionList = &csList;

   for (i = 0; i < mapPointList->size(); i++) {
      MapPoint& point = (*mapPointList)[i];

      // See if the point is contained
      int pl = 0;
      for (; pl < planeList.size(); pl++)
         if (planeList[pl].whichSide(point.pos) == TPlaneF::Outside)
            break;

      // If not inside, see if it's visibly to a poly center.
      // If it's not visible, it's brought in to the closest
      // point in a poly.
      if (pl != planeList.size()) {
         Point3F planeFrac = plane;
         planeFrac *= 0.05f;
         
         Point3F upCenter  = polyCenter;
         Point3F upPoint   = point.pos;
         upCenter += planeFrac;
         upPoint  += planeFrac;

         if (los.collide(upCenter,upPoint)) {
            point.pos = csList[0].position;
         }
      }
   }
}

//----------------------------------------------------------------------------
// We don't want sample points that extend outside the poly if
// they pass through a solid leaf. This causes light and shadow bleeding.
//
void ITRBasicLighting::MapData::markPointList(ITRGeometry&          geometry,
                                              ITRGeometry::Surface& surface)
{
   AssertFatal(mapPointList != NULL, "Must call buildpointlist first...");

   TPlaneF plane = geometry.planeList[surface.planeIndex];
   if (!surface.planeFront)
      plane.neg();

   // Build list of bounding planes for all polys
   Point3F polyCenter;
   Vector<TPlaneF> planeList;
   
   ITRGeometry::Vertex* vp = &geometry.vertexList[surface.vertexIndex];

   polyCenter.set(0.0f,0.0f,0.0f);

   Point3F *start = &geometry.point3List[vp[surface.vertexCount - 1].pointIndex];
   int i;
   for (i = 0; i < surface.vertexCount; i++) {
      Point3F* end = &geometry.point3List[vp[i].pointIndex];
      Point3F lnormal,lvec = *end; lvec -= *start;
      float len = lvec.len();
      if (len > distancePrecision) {
         lvec.normalize();
         m_cross(lvec,plane,&lnormal);
         if (!isEqual(lnormal.len(),.0f)) {
            planeList.increment();
            planeList.last().set(*start,lnormal);
         }
      }
      polyCenter += *end;
      start = end;
   }
   polyCenter *= 1.0f / float(surface.vertexCount);

   // Test all the points to see if they are inside a poly.
   CollisionSurfaceList csList;
   ITRCollision los(&geometry);
   los.collisionList = &csList;

   for (i = 0; i < mapPointList->size(); i++) {
      MapPoint& point = (*mapPointList)[i];

      // See if the point is contained
      int pl = 0;
      for (; pl < planeList.size(); pl++)
         if (planeList[pl].whichSide(point.pos) == TPlaneF::Outside)
            break;

      // If not inside, see if it's visibly to a poly center.
      // If it's not visible, mark it
      //
      if (pl != planeList.size()) {
         point.visible = false;
      } else {
         point.visible = true;
      }
   }
}

void
ITRBasicLighting::emitLightFromSurface(ITRGeometry& io_geometry,
                                       MapData&     io_mapData,
                                       LightMap&    io_emissionLightmap,
                                       Vector<int>* io_pPlaneSurfaceIndex,
                                       const int    in_surfaceNum)
{
   ITRGeometry::Surface& bsurface = io_geometry.surfaceList[in_surfaceNum];
   TPlaneF& plane = io_geometry.planeList[bsurface.planeIndex];

   MapPointList& pointList = *io_mapData.mapPointList;

   // Determine "close-enough" distance for points...
   //
   float closeEnough = float(1 << lightScale);
   if (lightFilter)
      closeEnough /= 2.0f;

   for (int i = 0; i < pointList.size(); i++) {
      MapPoint& point = pointList[i];

      // If the point is invisible, nothing happens...
      if (point.visible == false)
         continue;
         
      Color emitColor = io_emissionLightmap.getColor(point.mapX, point.mapY);
      if (lightFilter == true)
         emitColor *= 0.25;
      
      float intensity = 0.6 * emitColor.green + 0.3 * emitColor.red + 0.1 * emitColor.blue;
      int numQuanta = int(intensity * g_quantumNumber);
      if (numQuanta == 0)
         numQuanta = 1;
      
      emitColor *= (1.0f / float(numQuanta));

      for (int q = 0; q < numQuanta; q++) {
         // DMMNOTE: If modified phong values are reallowed, this will need to
         // change...
         //
         Point3F deflectedNormal;
         getCosRandDeflectedNormal(plane, deflectedNormal, 1.0);
      
			// Make sure the vector is pointed away from the plane.
			if (m_dot(plane, deflectedNormal) < 0.0f)
				deflectedNormal *= -1.0f;
			deflectedNormal.normalize();
         
			// Find the ray along which to shoot the line...
         Point3F shootPoint =	point.pos;
			Point3F ludicrousPoint = shootPoint;
			Point3F tmp = deflectedNormal;
			tmp *= 1e4f;
			ludicrousPoint += tmp;
      
			// Shoot the ray
			int surfColIndex;
			if (findCollisionSurface(&io_geometry, NULL, shootPoint,
				                      ludicrousPoint, &surfColIndex)) {
            Point3F collisionPoint = sg_collisionPoint;

            // Get the list of surfaces that are coplanar to this one...
            //
            int planeIndex = io_geometry.surfaceList[surfColIndex].planeIndex;
            if (io_geometry.surfaceList[surfColIndex].planeFront == false)
               planeIndex += io_geometry.planeList.size();

            Vector<int>& planeIList = io_pPlaneSurfaceIndex[planeIndex];
            
            // Just a quick check to make sure we're on the right page...
            //
            int test = 0;
            for (; test < planeIList.size(); test++)
               if (planeIList[test] == surfColIndex)
                  break;
            AssertFatal(test != planeIList.size(), "Error, surface not on it's own planelist.");

            for (Vector<int>::iterator itr = planeIList.begin();
                 itr != planeIList.end(); ++itr) {
               int surfIndex = *itr;
               
               MapData&  surfMapData  = sg_staticMapData[surfIndex];
               LightMap& surfLightMap = sg_staticLightMaps[surfIndex];
               
               // Perform an exhaustive search of the world points on this surface
               //  to determine which one gets the light.  This could perhaps
               //  be made more efficient, but is sufficient for now...
               //
               MapPointList& targPointList = *surfMapData.mapPointList;
               int targListSize = targPointList.size();
               for (int j = 0; j < targPointList.size(); j++) {
                  MapPoint& targPoint = targPointList[j];
                  
                  Point3F distPt = collisionPoint;
                  distPt -= targPoint.pos;
                  
                  float dist = getMax(getMax(fabs(distPt.x), fabs(distPt.y)), fabs(distPt.z));
                  if (dist <= closeEnough) {
                     // hit!  Light the point, and leave the search loop
                     targListSize;
                     Color& hitColor = surfLightMap.getColor(targPoint.mapX, targPoint.mapY);
                     hitColor += emitColor;
                     break;
                  }
               }
            }
			}
      }
   }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void ITRBasicLighting::emitSurfaceLight(ITRGeometry&      geometry,
                                        MaterialPropList& matProps)
{
   // If there are no emissive material properties modifications, simply
   //  return...
   //
   int emissiveSurfaces = 0;
   int i;
   for (i = 0; i < geometry.surfaceList.size(); i++) {
      int mp = geometry.surfaceList[i].material;
      if (mp == ITRGeometry::Surface::NullMaterial)
         continue;
         
      if (matProps[mp].modified     == true &&
          (matProps[mp].emissionType == MaterialProp::flatEmission ||
           false)) { // replace false with further emissive conditions...
         emissiveSurfaces++;
      }
   }
   if (emissiveSurfaces == 0)
      return;

   printf("%d emissive surfaces", emissiveSurfaces);

   // We must rebuild the mapData structures with unclamped points...
   //
   for (i = 0; i < sg_staticMapData.size(); i++) {
      delete sg_staticMapData[i].mapPointList;
      sg_staticMapData[i].mapPointList = NULL;
      
      MapData& mapData   = sg_staticMapData[i];

      // Rebuild, passing false to mark rather than clamp...
      ITRGeometry::Surface& bsurface = geometry.surfaceList[i];
      if (bsurface.material == ITRGeometry::Surface::NullMaterial)
         continue;

      mapData.buildPointList(geometry, bsurface, false);
   }

   // Build an index of surfaces by planeIndex...
   //
   Vector<int>* planeSurfaceIndex = new Vector<int>[(geometry.planeList.size() * 2)];
   for (i = 0; i < geometry.planeList.size(); i++) {
      Vector<int>& currFrontList = planeSurfaceIndex[i];
      Vector<int>& currBackList  = planeSurfaceIndex[i + geometry.planeList.size()];
      for (int j = 0; j < geometry.surfaceList.size(); j++) {
         if (geometry.surfaceList[j].material == ITRGeometry::Surface::NullMaterial)
            continue;
      
         if (geometry.surfaceList[j].planeIndex == i) {
            if (geometry.surfaceList[j].planeFront == true) {
               currFrontList.push_back(j);
            } else {
               currBackList.push_back(j);
            }
         }
      }
   }

   for (i = 0; i < geometry.surfaceList.size(); i++) {
      ITRGeometry::Surface& bsurface = geometry.surfaceList[i];
      if (bsurface.material == ITRGeometry::Surface::NullMaterial)
         continue;
         
      MaterialProp& matProp = matProps[bsurface.material];
      if (matProp.modified == true) {
         if (matProp.emissionType == MaterialProp::flatEmission) {
            // Create a dummy LightMap that contains the light to
            //  be emitted...
            //
            MapData& mapData = sg_staticMapData[i];

            LightMap dummyLightmap;
            dummyLightmap.clear(mapData.size);
            
            // Assign the emission color to the dummy lightmap...
            //
            Color pointColor = matProp.color;
            pointColor *= matProp.intensity;
            for (int y = 0; y < mapData.size.y; y++) {
               for (int x = 0; x < mapData.size.x; x++) {
                  Color& mapColor = dummyLightmap.getColor(x, y);
                  mapColor += pointColor;
               }
            }
            
            emitLightFromSurface(geometry, mapData, dummyLightmap, planeSurfaceIndex, i);
            printf(".");
         } else {
            //
         }
      }
   }
}


//----------------------------------------------------------------------------
int g_compressed   = 0;
int g_uncompressed = 0;

void ITRBasicLighting::exportSurfStaticLMap(ITRLighting*      lighting,
                                            int               ss)
{
   ITRLighting::Surface& lsurface = lighting->surfaceList[ss];
   LightMap& staticMap = sg_staticLightMaps[ss];
   
   // Only export the map if this hasn't been marked as being a default color
   //
   if ((lsurface.mapColor & 0x8000000) == 0) {
      bool didCompress;
      lsurface.mapIndex = exportMap(staticMap, lighting, didCompress);

      if (didCompress == true) {
         lsurface.mapColor |= 0x40000000;
         g_compressed++;
      } else {
         lsurface.mapColor &= ~0x40000000;
         g_uncompressed++;
      }
   }
}

// fill's in the shapes's ambient intensities into the lightmap prior to adding lights
void ITRBasicLighting::fillAmbient( LightMap * map )
{
   for( int y = 0; y < map->size.y; y++ )
   {
      for( int x = 0; x < map->size.x; x++ )
      {
         Color & col = map->getColor( x, y );
         col.red += m_ambientIntensity.red;
         col.green += m_ambientIntensity.green;
         col.blue += m_ambientIntensity.blue;
      }
   }
}

void ITRBasicLighting::lightSurface( ITRGeometry & geometry,
   LightList & lights, ITRLighting * lighting, int ss1, 
   ITRMissionLighting * missionLighting, int ss2, CallbackInfo * callback )
{
   ITRLighting::Surface & mlsurface = missionLighting->surfaceList[ss2];
   ITRLighting::Surface & lsurface = lighting->surfaceList[ss1];
   ITRGeometry::Surface & bsurface = geometry.surfaceList[ss1];

   MapData& mapData = sg_staticMapData[ss2];
   mapData.set( *lighting, geometry, bsurface );

   mlsurface.mapSize.x = UInt8(mapData.size.x);
   mlsurface.mapSize.y = UInt8(mapData.size.y);
   mlsurface.mapOffset.x = UInt8(mapData.offset.x);
   mlsurface.mapOffset.y = UInt8(mapData.offset.y);
   mlsurface.lightIndex = missionLighting->lightMapListSize;
   mlsurface.lightCount = 0;

   if( bsurface.material != ITRGeometry::Surface::NullMaterial) 
   {
      // Build list of points to be sampled.
      mapData.buildPointList( geometry,bsurface );

      MapPointList& pointList = *mapData.mapPointList;

      LightMap & staticMap = sg_staticLightMaps[ss2];
      staticMap.clear( mapData.size );

      TPlaneF& plane = geometry.planeList[ bsurface.planeIndex ];
      
      // check if this surface has a lightmap
      if( lsurface.mapIndex < 0 )
      {
         int x, y;
         float r,g,b,i;
         ITRLighting::unpackColor( Int16( lsurface.mapColor ), r, g, b, i );
            
         for( y = 0; y < mapData.size.y; y++ )
               for( x = 0; x < mapData.size.x; x++ )
                  staticMap.getColor( x, y ).set( r,g,b );
      }
      else
      {
         if ((lsurface.mapIndex & 0x40000000) == 0) {
            // Lightmap is uncompressed
            //
            int x, y;
            UInt16 * pData = ( UInt16 * )&lighting->mapData[ lsurface.mapIndex ];
            
         for( y = 0; y < mapData.size.y; y++ )
            {
               for( x = 0; x < mapData.size.x; x++ )
               {
                  // convert the current data item
                  float r,g,b,i;
                     
               ITRLighting::unpackColor( *pData, r, g, b, i );
                  staticMap.getColor( x, y ).set( r, g, b );

                  // get to the next color
                  pData++;
               }
            }
         } else {
            // Lightmap is compressed, we need to decompress it.  Mind you,
            //  it is important what order we access it in.
            //
            int actualIndex  = lsurface.mapIndex & ~(0x40000000);
            UInt8* pCompData = &lighting->mapData[actualIndex];
            BitStream bs(pCompData, mapData.size.x * mapData.size.y * 2);

            for (int y = 0; y < mapData.size.y; y++) {
               for (int x = 0; x < mapData.size.x; x++) {
                  float r,g,b,i;
                  UInt16 color = lighting->decodeNextColor16(bs);
                  ITRLighting::unpackColor(color, r, g, b, i);
                  staticMap.getColor( x, y ).set( r, g, b );
               }
            }
         }
      }
         
      Point3F normal = plane;
      if( !bsurface.planeFront )
         normal.neg();

      // fill in the base ambient for this shape
      if( g_applyAmbientOutside || !geometry.surfaceList[ss1].visibleToOutside )
         fillAmbient( &staticMap );
      
      // Light the point list with all the lights within range.
      for( int l = 0; l < lights.size(); l++ ) 
      {
         Light& light = *lights[l];
         // If light only has a single state, its added
         // to the static lightmap
         if (light.state.size() == 1) 
         {
            Light::State& state = *light.state[0];

            for (int e = 0; e < state.m_emitterList.size(); e++) 
            {
               if( state.m_emitterList[e].isLit( geometry, bsurface ) == true ) 
               {
                  state.light( geometry, pointList, normal, &staticMap, callback );
                  break;
               }
            }
         }
      }
   } 
   else 
   {
      lsurface.mapIndex = -1;
   }
}

//--------------------------------------------------------------------

void ITRBasicLighting::lightSurface(ITRGeometry&      geometry,
                                    LightList&        lights,
                                    MaterialPropList& matProps,
                                    ITRLighting*      lighting,
                                    int               ss )
{
   ITRLighting::Surface& lsurface = lighting->surfaceList[ss];
   ITRGeometry::Surface& bsurface = geometry.surfaceList[ss];

   MapData& mapData = sg_staticMapData[ss];
   mapData.set(*lighting,geometry,bsurface);

   lsurface.mapSize.x = UInt8(mapData.size.x);
   lsurface.mapSize.y = UInt8(mapData.size.y);
   lsurface.mapOffset.x = UInt8(mapData.offset.x);
   lsurface.mapOffset.y = UInt8(mapData.offset.y);
   lsurface.lightIndex = lighting->lightMapListSize;
   lsurface.lightCount = 0;

   if (bsurface.material != ITRGeometry::Surface::NullMaterial) {
      // Build list of points to be sampled.
      mapData.buildPointList(geometry,bsurface);

      MapPointList& pointList = *mapData.mapPointList;

      LightMap& staticMap = sg_staticLightMaps[ss];
      staticMap.clear(mapData.size);

      TPlaneF& plane = geometry.planeList[bsurface.planeIndex];

      // Check to see if we need to affect the static lightmap with any material
      //  properties...
      //
      if (matProps.size() != 0 &&
          matProps[bsurface.material].modified == true) {
         MaterialProp& matProp = matProps[bsurface.material];
         
         if (matProp.emissionType == MaterialProp::flatColor) {
            for (int y = 0; y < mapData.size.y; y++) {
               for (int x = 0; x < mapData.size.x; x++) {
                  Color& color = staticMap.getColor(x, y);
                  color += matProp.color;
               }
            } 
         } else if (matProp.emissionType == MaterialProp::flatEmission) {
            // If the surface is emitting, we want to see the whole thing...
            //
            Color fullBright;
            fullBright.set(1.0f, 1.0f, 1.0f);
            for (int y = 0; y < mapData.size.y; y++) {
               for (int x = 0; x < mapData.size.x; x++) {
                  Color& color = staticMap.getColor(x, y);
                  color += fullBright;
               }
            } 
         }
      }

      Point3F normal = plane;
      if( !bsurface.planeFront )
         normal.neg();

      // fill in the base ambient for this shape
      if( g_applyAmbientOutside || !geometry.surfaceList[ss].visibleToOutside )
         fillAmbient( &staticMap );
      
      // Light the point list with all the lights within range.
      for (int l = 0; l < lights.size(); l++) {
         Light& light = *lights[l];
         // If light only has a single state, its added
         // to the static lightmap
         if (light.state.size() == 1) {
         
            Light::State& state = *light.state[0];

//            // check if this surface should get the ambient light intensity
//            if( geometry.surfaceList[ss].applyAmbient )
//               state.fillAmbient( &staticMap );

            for (int e = 0; e < state.m_emitterList.size(); e++) {
               if (state.m_emitterList[e].isLit(geometry, bsurface) == true) {
                  state.light(geometry,pointList,normal,&staticMap,NULL);
                  break;
               }
            }
         }
         else {
            // If any state lights the surface, they all do.
            int i = 0;
            for (; i < light.state.size(); i++) {
               bool breakNow = false;
               for (int j = 0; j < light.state[i]->m_emitterList.size(); j++) {
                  if (light.state[i]->m_emitterList[j].isLit(geometry, bsurface) == true) {
                     breakNow = true;
                     break;
                  }
               }
               
               if (breakNow == true)
                  break;
            }
            if (i != light.state.size()) {
               // Light map for each state
               //
               for (int i = 0; i < light.state.size(); i++) {
                  Light::State& state = *light.state[i];
                  
                  if (state.uniqueKey == -1) {
                     // state is unique, or the first of a series...
                     //
                     static StateLightMap map;
                     map.clear(mapData.size);

                     bool possible = false;
                     for (int e = 0; e < state.m_emitterList.size(); e++) {
                        if (state.m_emitterList[e].isLit(geometry, bsurface) == true) {
                           possible = true;
                           break;
                        }
                     }
                     
                     if (possible == true && state.lightState(geometry, pointList, normal, &map)) {
                        state.appendSurface(ss, lighting->lightMapListSize,
                                            exportStateMap(map, lighting));
                     } else {
                        // No light map for this guy.
                        state.appendSurface(ss,lighting->lightMapListSize, -1);
                     }
                  } else {
                     // state matches a previously lit surface...
                     //
                     Light::State& matchState = *light.state[state.uniqueKey];
                     
                     // copy surface data...
                     //
                     state.surfaceData = matchState.surfaceData;
                     state;
                     state;
                  }
               }

               // All the states for this light got assigned the
               // same lightMapList index;
               //
               lighting->lightMapListSize++;
               lsurface.lightCount++;
            }
         }
      }
   } else {
      lsurface.mapIndex = -1;
   }
}

bool
ITRBasicLighting::Light::State::Emitter::operator==(const Emitter& in_rCompare) const
{
   if (lightType != in_rCompare.lightType)
      return false;
   if (pos != in_rCompare.pos)
      return false;
   if (falloff != in_rCompare.falloff)
      return false;

   if (d1 != in_rCompare.d1)
      return false;
   if (d2 != in_rCompare.d2)
      return false;
   if (falloff != Linear)
      if (d3 != in_rCompare.d3)
         return false;

   if (lightType == Light::State::SpotLight) {
      if (spotVector != in_rCompare.spotVector)
         return false;
      if (spotTheta != in_rCompare.spotTheta)
         return false;
      if (spotPhi != in_rCompare.spotPhi)
         return false;
   }

   return true;
}

bool
ITRBasicLighting::areEqualStates(const Light::State* pStateOne,
                                 const Light::State* pStateTwo)
{
   if (pStateOne->m_emitterList.size() != pStateTwo->m_emitterList.size())
      return false;

   Vector<int> stateTwoEmitters;
   int i;
   for (i = 0; i < pStateTwo->m_emitterList.size(); i++)
      stateTwoEmitters.push_back(i);

   for (i = 0; i < pStateOne->m_emitterList.size(); i++) {
      const Light::State::Emitter& rEmitter = pStateOne->m_emitterList[i];
      for (int j = 0; j < stateTwoEmitters.size(); j++) {
         const Light::State::Emitter& rCompare = pStateTwo->m_emitterList[stateTwoEmitters[j]];

         if (rEmitter == rCompare) {
            stateTwoEmitters.erase(j);
            break;
         }
      }
   }

   return (stateTwoEmitters.size() == 0);
}


void
ITRBasicLighting::processUniqueLights(LightList& lights)
{
   for (int i = 0; i < lights.size(); i++) {
      Light* pLight = lights[i];

      for (int j = 0; j < pLight->state.size(); j++) {
         Light::State* pState = pLight->state[j];
         
         int key = -1;
         for (int k = 0; k < pLight->uniqueStates.size(); k++) {
            Light::State* pCompareState = pLight->state[pLight->uniqueStates[k]];
            
            if (areEqualStates(pState, pCompareState) == true) {
               key = pLight->uniqueStates[k];
               break;
            }
         }
         pState->uniqueKey = key;
         
         if (key == -1) {
            // state is a new unique state...
            //
            pLight->uniqueStates.push_back(j);
         }
      }
   }
}

//----------------------------------------------------------------------------

void ITRBasicLighting::light( ITRGeometry & geometry, LightList & lights,
   ITRLighting * lighting, ITRMissionLighting * missionLighting, CallbackInfo * callback )
{
   g_missionLighting = true;
   
   sg_huffman16Nodes.clear();
   sg_huffman16Leaves.clear();
   g_disableCompression = false;

   g_compressed = 0;
   g_uncompressed = 0;
   
   TPlaneF::DistancePrecision = distancePrecision;
   TPlaneF::NormalPrecision = normalPrecision;
   if( filterScale != 0.f )
      lightFilter = true;
   missionLighting->geometryBuildId = geometry.buildId;
   missionLighting->lightScaleShift = lightScale;

   // walk through the shape and count the number of outside vis. surfaces
   int numVis = 0;
   int s;
   for( s = 0; s < lighting->surfaceList.size(); s++ )
      if( geometry.surfaceList[s].visibleToOutside )
         numVis++;

   // create the surface list for this guy
   missionLighting->lightMapListSize = 0;
   missionLighting->surfaceList.setSize( numVis );
   memset( missionLighting->surfaceList.address(), 0,  sizeof( ITRLighting::SurfaceList::value_type ) * numVis );
      
   // Seed the random number generator...
   sg_random.setSeed(0);

   // Set up the static lightmaps and mapdatas...
   sg_staticLightMaps.setSize( missionLighting->surfaceList.size() );
   sg_staticMapData.setSize( missionLighting->surfaceList.size() );
   int i;
   for( i = 0; i < sg_staticLightMaps.size(); i++ ) 
   {
      new ( &sg_staticLightMaps[i] )LightMap;
      new ( &sg_staticMapData[i] )MapData;
   }

   printf("   - First pass lighting...");
   int index = 0;
   for( s = 0; s < lighting->surfaceList.size(); s++ )
   {
      // is this one we care about?
      if( geometry.surfaceList[s].visibleToOutside )
      {
         lightSurface( geometry, lights, lighting, s, missionLighting, 
            index++, callback );
      }
   }
   
   printf("done\n");
   
   // Ok, create the huffman16 tables for compressing the static lightmaps...
   printf("Exporting Static lightMaps...\n");
   printf("\tCreating Huffman16 Code...");
   createStaticHuffman16Codes( missionLighting );
   printf("done.\n");
   
   printf("\tEncoding lightmaps...");
   for( s = 0; s < missionLighting->surfaceList.size(); s++ )
      exportSurfStaticLMap( missionLighting, s );

   printf("%d comp., %d uncomp...done.\n", g_compressed, g_uncompressed);
   printf("\tExporting Huffman16 Codes...");
   exportHuffman16Tables( missionLighting );
   printf("done.\n");

   // fill the surface mapping
   index = 0;
   for( s = 0; s < lighting->surfaceList.size(); s++ )
   {            
      // is this one we care about?
      if( geometry.surfaceList[s].visibleToOutside )
      {
         ITRMissionLighting::IndexEntry entry;
         entry.srcIndex = lighting->surfaceList[s].mapIndex & ~0x40000000;
         entry.destIndex = missionLighting->surfaceList[index].mapIndex & ~0x40000000;
         missionLighting->addEntry(entry);
         index++;            
      }
   }
   
   // Destroy the static lightmaps...
   for( i = 0; i < sg_staticLightMaps.size(); i++ ) 
   {
      sg_staticLightMaps[i].~LightMap();
      sg_staticMapData[i].~MapData();
   }
   sg_staticLightMaps.clear();
   sg_staticMapData.clear();

   exportLights( lights, missionLighting );
}

//----------------------------------------------------------------------------
// Light the geometry file with the given lights and export
// to the given lighting struct.
//
void
ITRBasicLighting::light(ITRGeometry&      geometry,
                        LightList&        lights,
                        MaterialPropList& matProps,
                        ITRLighting*      lighting )
{
   sg_huffman16Nodes.clear();
   sg_huffman16Leaves.clear();
   g_disableCompression = false;

   g_compressed = 0;
   g_uncompressed = 0;
   
   TPlaneF::DistancePrecision = distancePrecision;
   TPlaneF::NormalPrecision = normalPrecision;
   if (filterScale != 0.0f)
      lightFilter = true;
   lighting->geometryBuildId = geometry.buildId;
   lighting->lightScaleShift = lightScale;

   lighting->lightMapListSize = 0;
   lighting->surfaceList.setSize(geometry.surfaceList.size());
   memset(lighting->surfaceList.address(), 0, sizeof(ITRLighting::SurfaceList::value_type) * geometry.surfaceList.size());
   
   // Seed the random number generator...
   //
   sg_random.setSeed(0);//time(NULL));

   // First, process all the lights, and find which states are unique.
   //
   printf("   - Extracting Unique light states...");
   processUniqueLights(lights);
   printf("done\n");

   // Set up the static lightmaps and mapdatas...
   //
   
   sg_staticLightMaps.setSize(lighting->surfaceList.size());
   sg_staticMapData.setSize(lighting->surfaceList.size());
   int i, s;
   for (i = 0; i < sg_staticLightMaps.size(); i++) {
      new (&sg_staticLightMaps[i]) LightMap;
      new (&sg_staticMapData[i])   MapData;
   }

   printf("   - First pass lighting...");
   for (s = 0; s < lighting->surfaceList.size(); s++) {
      lightSurface(geometry, lights, matProps, lighting, s);
      printf(".");
      fflush(stdout);
   }
   printf("done\n");
   
   if (matProps.size() != 0) {
      printf("   - Surface light emission...");
      emitSurfaceLight(geometry, matProps);
      printf("done\n");
   }
   
   // Ok, create the huffman16 tables for compressing the static lightmaps...
   //
   printf("Exporting Static lightMaps...\n");
   printf("\tCreating Huffman16 Code...");
   createStaticHuffman16Codes(lighting);
   printf("done.\n");
   
   printf("\tEncoding lightmaps...");
   for (s = 0; s < lighting->surfaceList.size(); s++) 
      exportSurfStaticLMap(lighting, s);
   printf("%d comp., %d uncomp...done.\n",
          g_compressed, g_uncompressed);

   printf("\tExporting Huffman16 Codes...");
   exportHuffman16Tables(lighting);
   printf("done.\n");
   
   // Destroy the static lightmaps...
   //
   for (i = 0; i < sg_staticLightMaps.size(); i++) {
      sg_staticLightMaps[i].~LightMap();
      sg_staticMapData[i].~MapData();
   }
   sg_staticLightMaps.clear();
   sg_staticMapData.clear();

   exportLights(lights, lighting);
}


//----------------------------------------------------------------------------
// Compare function used to sort ITRLighting::Lights in ::exportLights
//
static int _USERENTRY
ITRBasicLighting::compareLights(const void* a,const void* b)
{
   const ITRLighting::Light* aa =
      reinterpret_cast<const ITRLighting::Light*>(a);
   const ITRLighting::Light* bb =
      reinterpret_cast<const ITRLighting::Light*>(b);
   return aa->id - bb->id;
}



void
ITRBasicLighting::processAnimations(LightList& lights)
{
   for (LightList::iterator itr = lights.begin(); itr != lights.end(); itr++) {
      // Only lights with more than one state are animated...
      //
      if ((*itr)->state.size() > 1) {
         Light& light = **itr;
         light.animationDuration = 0.0f;
         for (int i = 0; i < light.state.size(); i++) {
            Light::State& state = *light.state[i];
            
            float temp          = state.stateDuration;
            state.stateDuration = light.animationDuration;
            light.animationDuration += temp;
         }
      }
   }
}


inline UInt16 buildFixColor(const float in_colorElem)
{
   return UInt16((in_colorElem * 255.0f) * 256.0f);
}

//----------------------------------------------------------------------------
// Export all the state animated lights to the lighting struct.
//
void ITRBasicLighting::exportLights(LightList&   lights,
                                    ITRLighting* lighting)
{
   processAnimations(lights);

   for (LightList::iterator itr = lights.begin(); itr != lights.end(); itr++) {
      // Only exports lights with at least two states, or special attributes
      if ((*itr)->state.size() > 1) {
         Light& light = **itr;
         lighting->lightList.increment();
         ITRLighting::Light& llight = lighting->lightList.last();

         if (light.name.c_str() != NULL) {
            // if this is a named light, export it to the Map in the lighting
            //  structure...
            //
            llight.nameIndex = lighting->nameBuffer.size();
            lighting->nameBuffer.setSize(llight.nameIndex + strlen(light.name.c_str()) + 1);
            strcpy(&lighting->nameBuffer[llight.nameIndex], light.name.c_str());
         } else {
            llight.nameIndex = -1;
         }

         llight.id                = light.id;
         llight.animationDuration = light.animationDuration;
         llight.animationFlags    = light.flags;
         llight.stateCount        = light.state.size();
         llight.stateIndex        = lighting->stateList.size();
         lighting->stateList.setSize(lighting->stateList.size() + llight.stateCount);

         for (int c = 0; c < light.state.size(); c++) {
            ITRLighting::State& lstate = 
               lighting->stateList[llight.stateIndex + c];
            Light::State& state = *light.state[c];

            float mono = (state.color.red   * .299f) +
                         (state.color.green * .587f) +
                         (state.color.blue  * .114f);
            lstate.red   = buildFixColor(state.color.red);
            lstate.green = buildFixColor(state.color.green);
            lstate.blue  = buildFixColor(state.color.blue);
            lstate.mono  = buildFixColor(mono);

            // in processAnimations, this was turned into the time that the
            //  state becomes active...
            //
            lstate.animationTime = state.stateDuration;

            lstate.dataCount   = state.surfaceData.size();
            lstate.dataIndex   = lighting->stateDataList.size();
            lighting->stateDataList.setSize(lighting->stateDataList.size() +
                                            lstate.dataCount);

            for (int s = 0; s < state.surfaceData.size(); s++) {
               ITRLighting::StateData& ldata =
                  lighting->stateDataList[lstate.dataIndex + s];
               Light::State::Surface& csurf  = state.surfaceData[s];

               ldata.surface    = csurf.surface;
               ldata.lightIndex = csurf.lightIndex;
               ldata.mapIndex   = csurf.mapIndex;
            }
         }
      }
   }
   
   // Sort the lightList by ID for later searching.
   qsort(lighting->lightList.address(),
         lighting->lightList.size(),
         sizeof(ITRLighting::Light),
         compareLights);
}



void
ITRBasicLighting::createStaticHuffman16Codes(ITRLighting* lighting)
{
   AssertFatal(sg_huffman16Nodes.size() == 0, "Error, huffman16 tree exists?");

   // First, we have to assess the popularity of all possible colors from
   //  the static lightmaps.  We always include black and white to make sure
   //  we have at least two colors to create a tree with.
   //
   UInt32 popTable[(1 << 16)];
   memset(popTable, 0, sizeof(UInt32) * (1 << 16));
   popTable[0]      = 1;
   popTable[0xffff] = 1;

   int i;
   for (i = 0; i < lighting->surfaceList.size(); i++) {
      ITRLighting::Surface& lsurface = lighting->surfaceList[i];
      // only enter into popularity table if map is not a solid color...
      //
      if ((lsurface.mapColor & 0x80000000) != 0)
         continue;

      LightMap& staticMap = sg_staticLightMaps[i];

      for (int y = 0; y < staticMap.size.y; y++) {
         for (int x = 0; x < staticMap.size.x; x++) {
            Color& colorf = staticMap.getColor(x, y);
            UInt16 color  = ITRLighting::buildColor(colorf.red,
                                                   colorf.green,
                                                   colorf.blue);
            popTable[color]++;
         }
      }
   }

   // Now create the leaves of the tree...
   //
   for (i = 0; i < (1 << 16); i++) {
      Huffman16Leaf leaf;
      leaf.pop = popTable[i];
      if (leaf.pop == 0)
         continue;

      leaf.color = i;
      sg_huffman16Leaves.push_back(leaf);
   }
   if (sg_huffman16Leaves.size() == 1) {
      // The interior is all one color, i.e., an unmission lit
      //  rock-type shape.  Disable compression, and return...
      //
      g_disableCompression = true;
      return;
   }

   // Ok, the leaves are created, on to building the tree.
   //
   sg_huffman16Nodes.increment();

   UInt32 currWraps = UInt32(sg_huffman16Leaves.size());
   AssertFatal(currWraps > 0, "Error, no leaves in huffman16 tree?");

   Huffman16Wrap* nodesLeft[1 << 16];
   Int32 j;
   for (j = 0; j < Int32(currWraps); j++) {
      nodesLeft[j] = new Huffman16Wrap(-j - 1);
   }
   for (j = currWraps; j < (1 << 16); j++)
      nodesLeft[j] = NULL;

   // Create the tree, loop until there is only one node left...
   //
   while (currWraps != 1) {
      Int32  firstIndex  = -1;
      Int32  secondIndex = -1;
      UInt32 firstValue  = 0xffffffff;
      UInt32 secondValue = 0xffffffff;

      for (j = 0; j < Int32(currWraps); j++) {
         UInt32 pop = nodesLeft[j]->getPop();

         if (pop < firstValue) {
            // New lowest
            //
            secondIndex = firstIndex;
            secondValue = firstValue;

            firstIndex = j;
            firstValue = Int32(pop);
         } else if (pop < secondValue) {
            // New second lowest...
            secondIndex = j;
            secondValue = Int32(pop);
         }
      }
      AssertFatal(firstIndex != -1 && secondIndex != -1,
                  "This should never happen when there is more than one wrap");
      AssertFatal(firstIndex != secondIndex, "This should just never happen");

      // Ok, we need to create a node that encompasses both the first and
      //  second wraps...
      //
      Huffman16Wrap* pFirst  = nodesLeft[firstIndex];
      Huffman16Wrap* pSecond = nodesLeft[secondIndex];

      Int32 index = sg_huffman16Nodes.size();
      sg_huffman16Nodes.increment();
      Huffman16Node& rNode = sg_huffman16Nodes[index];

      rNode.indexOne  = pFirst->getIndex();
      rNode.indexZero = pSecond->getIndex();
      rNode.pop       = pFirst->getPop() + pSecond->getPop();

      Huffman16Wrap* pNew = new Huffman16Wrap(index);
      int insertIndex = firstIndex < secondIndex ? firstIndex : secondIndex;
      int otherIndex  = firstIndex > secondIndex ? firstIndex : secondIndex;
      
      nodesLeft[insertIndex] = pNew;
      nodesLeft[otherIndex]  = nodesLeft[currWraps - 1];
      nodesLeft[--currWraps] = NULL;

      delete pFirst;
      delete pSecond;
   }

   // Ok, at this point, nodesLeft[0] is the root node of the huffman16 tree, and
   //  should be the last entry in the sg_huffman16Nodes array.  We need to swap it
   //  so that the root node is in the sg_huffman16Nodes[0] entry.  Easy.
   //
   AssertFatal(nodesLeft[0]->getIndex() == sg_huffman16Nodes.size() - 1,
               "Root node is at the wrong place in the node list");
   delete nodesLeft[0];
   nodesLeft[0] = NULL;
   sg_huffman16Nodes[0] = sg_huffman16Nodes.last();
   sg_huffman16Nodes.decrement();

   // Ok, at this point, we have a huffman16 tree, rooted at [0].  Now we need to generate
   //  the codes for the leaves...
   //
   UInt8 codeBuffer[16];
   generateHuffman16Recurse(sg_huffman16Nodes[0],
                          codeBuffer,
                          0);
   int maxLength = -1;
   for (i = 0; i < sg_huffman16Leaves.size(); i++) {
      if (sg_huffman16Leaves[i].codeLength > maxLength)
         maxLength = sg_huffman16Leaves[i].codeLength;
   }
   printf("longest code: %d bits...", maxLength);
}

void
ITRBasicLighting::generateHuffman16Recurse(const Huffman16Node& in_rNode,
                                         UInt8              io_codeBuffer[16],
                                         UInt32             in_codeLength)
{
   AssertFatal(in_codeLength < 128, "Code length too long!");

   // Do zero branch, clear last bit...
   //
   io_codeBuffer[in_codeLength >> 3] &= ~(1 << (in_codeLength & 0x7));
   if (in_rNode.indexZero < 0) {
      // Leaf node!  Set the code, and the codeLength...
      //
      Huffman16Leaf& rLeaf = sg_huffman16Leaves[-(in_rNode.indexZero + 1)];
      memcpy(rLeaf.code, io_codeBuffer, 16);
      rLeaf.codeLength = in_codeLength + 1;
   } else {
      generateHuffman16Recurse(sg_huffman16Nodes[in_rNode.indexZero],
                             io_codeBuffer, in_codeLength + 1);
   }

   // Do one branch, set last bit...
   //
   io_codeBuffer[in_codeLength >> 3] |= 1 << (in_codeLength & 0x7);
   if (in_rNode.indexOne < 0) {
      // Leaf node!  Set the code, and the codeLength...
      //
      Huffman16Leaf& rLeaf = sg_huffman16Leaves[-(in_rNode.indexOne + 1)];
      memcpy(rLeaf.code, io_codeBuffer, 16);
      rLeaf.codeLength = in_codeLength + 1;
   } else {
      generateHuffman16Recurse(sg_huffman16Nodes[in_rNode.indexOne],
                             io_codeBuffer, in_codeLength + 1);
   }
}

void
ITRBasicLighting::exportHuffman16Tables(ITRLighting* lighting)
{
   if (g_disableCompression == true) {
      lighting->m_huffmanCompressed = false;
      return;
   }

   lighting->m_huffmanCompressed = true;

   lighting->huffman16Nodes.setSize(sg_huffman16Nodes.size());
   lighting->huffman16Leaves.setSize(sg_huffman16Leaves.size());

   int i;
   for (i = 0; i < sg_huffman16Nodes.size(); i++) {
      lighting->huffman16Nodes[i].indexOne  = sg_huffman16Nodes[i].indexOne;
      lighting->huffman16Nodes[i].indexZero = sg_huffman16Nodes[i].indexZero;
   }
   for (i = 0; i < sg_huffman16Leaves.size(); i++) {
      lighting->huffman16Leaves[i].color = sg_huffman16Leaves[i].color;
   }
}

//----------------------------------------------------------------------------
// Copy the light map into the Lighting data struct.
//
UInt16
testDecompress(UInt8* pBuffer,
               const UInt32 in_bitSize)
{
   BitStream bs(pBuffer, in_bitSize);
   
   int currNode = 0;
   UInt32 bitCount = 0;
   while (true) {
      bool bit = bs.readFlag();
      bitCount++;
      if (bit == true) {
         currNode = ITRBasicLighting::sg_huffman16Nodes[currNode].indexOne;
      } else {
         currNode = ITRBasicLighting::sg_huffman16Nodes[currNode].indexZero;
      }

      if (currNode < 0) {
         AssertFatal(bitCount == in_bitSize, avar("Mismatch code size (%d, %d)", bitCount, in_bitSize));
         return ITRBasicLighting::sg_huffman16Leaves[-(currNode + 1)].color;
      }
   }
}


int
ITRBasicLighting::exportMap(ITRBasicLighting::LightMap& map,
                            ITRLighting*                lighting,
                            bool&                       didCompress)
{
   if (g_disableCompression == false) {
      // First, we need to create a compressed copy of the lightmap.  We assume
      //  that the huffman16 compressed version will be no larger than twice the
      //  size of the lightmap.
      //
      UInt32 bufferSize = map.size.x * map.size.y * sizeof(UInt16) * 2;
      UInt8* pCompBuffer = new UInt8[bufferSize];
      BitStream bStream(pCompBuffer, bufferSize);

      for (int y = 0; y < map.size.y; y++) {
         for (int x = 0; x < map.size.x; x++) {
            Color& rColor = map.getColor(x, y);
            UInt16 iColor = ITRLighting::buildColor(rColor.red,
                                                    rColor.green,
                                                    rColor.blue);

            int match = -1;
            for (int i = 0; i < sg_huffman16Leaves.size(); i++) {
               if (sg_huffman16Leaves[i].color == iColor) {
                  match = i;
                  break;
               }
            }
            AssertFatal(match != -1, "Color not in huffman16 pop tables");
            Huffman16Leaf& rLeaf = sg_huffman16Leaves[match];
            AssertFatal(rLeaf.color == iColor, "Wrong node color");

            AssertFatal(testDecompress(rLeaf.code, rLeaf.codeLength) == rLeaf.color,
                        "Code doesn't decode to leaf?");

            bStream.writeBits(rLeaf.codeLength, rLeaf.code);
         }
      }

      UInt32 byteSize = (bStream.getCurPos() + 7) >> 3;

      if (byteSize < map.size.x * map.size.y * sizeof(UInt16)) {
         // Ok, the huffman16 encoding wins.
         //
         didCompress = true;

         int mapIndex = lighting->mapData.size();
         lighting->mapData.setSize(lighting->mapData.size() + byteSize);

         UInt8* pData = &lighting->mapData[mapIndex];
         memcpy(pData, pCompBuffer, byteSize);

         delete [] pCompBuffer;
         return mapIndex;
      } else {
         // We're better off to output the data in an uncompressed format...
         //
         didCompress = false;
         delete [] pCompBuffer;
         pCompBuffer = NULL;

         int mapIndex   = lighting->mapData.size();
         int uncMapSize = map.size.x * map.size.y * sizeof(UInt16);
         lighting->mapData.setSize(lighting->mapData.size() + uncMapSize);
         UInt16* pOutput = (UInt16*)&lighting->mapData[mapIndex];

         for (int y = 0; y < map.size.y; y++) {
            for (int x = 0; x < map.size.x; x++) {
               UInt16* pDst = &pOutput[y * map.size.x + x];

               Color& rColor = map.getColor(x, y);
               UInt16 iColor = ITRLighting::buildColor(rColor.red,
                                                       rColor.green,
                                                       rColor.blue);
               *pDst = iColor;
            }
         }
         
         return mapIndex;
      }
   } else {
      // Compression is off, do this the normal way...
      //
      didCompress = false;

      int mapIndex   = lighting->mapData.size();
      int uncMapSize = map.size.x * map.size.y * sizeof(UInt16);
      lighting->mapData.setSize(lighting->mapData.size() + uncMapSize);
      UInt16* pOutput = (UInt16*)&lighting->mapData[mapIndex];

      for (int y = 0; y < map.size.y; y++) {
         for (int x = 0; x < map.size.x; x++) {
            UInt16* pDst = &pOutput[y * map.size.x + x];

            Color& rColor = map.getColor(x, y);
            UInt16 iColor = ITRLighting::buildColor(rColor.red,
                                                    rColor.green,
                                                    rColor.blue);
            *pDst = iColor;
         }
      }
      
      return mapIndex;
   }
}

int
ITRBasicLighting::exportStateMap(StateLightMap& io_map,
                                 ITRLighting*   io_lighting)
{
   // The light map is appended to the end of Lighting's 
   // light map buffer.
   int mapIndex = io_lighting->mapData.size();

   int mapSize = ((io_map.size.x * io_map.size.y * sizeof(UInt8)) +
                  ITRLighting::LightMap::HeaderSize);

   io_lighting->mapData.setSize(io_lighting->mapData.size() + mapSize);

   ITRLighting::LightMap* lmap =
      reinterpret_cast<ITRLighting::LightMap*>(&io_lighting->mapData[mapIndex]);

   // Convert and copy colors.  Note we cheesed, and stored the intensity
   //  in the red component of the lightmap...
   //
   UInt8* md = (UInt8*)&lmap->data;
   for (int y = 0; y < io_map.size.y; y++) {
      for (int x = 0; x < io_map.size.x; x++) {
         UInt8* dst = &md[y * io_map.size.x + x];
         float fInten = io_map.getColor(x, y).red;
         if (fInten > 1.0f)
            fInten = 1.0f;
         UInt8 iInten = UInt8((fInten * 255.0f) + 0.5);
         *dst = iInten;
      }
   }
   
   return mapIndex;
}




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-------------------------------------- Functions copied from itrRadiosityLighting.cpp
//

// Quick (writing time, not execution) function to determine whether a point is
// inside the given surface
//
bool
pointContainedInSurf(ITRGeometry*                in_geometry,
                     const ITRGeometry::Surface& in_surf,
                     const Point3F&              in_point)
{
   TPlaneF plane = in_geometry->planeList[in_surf.planeIndex];
   if (!in_surf.planeFront)
      plane.neg();

   // Build list of bounding planes for surface
   Vector<TPlaneF> planeList;

   ITRGeometry::Vertex* vp = &in_geometry->vertexList[in_surf.vertexIndex];

   Point3F *start = &in_geometry->point3List[vp[in_surf.vertexCount - 1].pointIndex];
   for (int i = 0; i < in_surf.vertexCount; i++) {
      Point3F* end = &in_geometry->point3List[vp[i].pointIndex];
      Point3F lnormal,lvec = *end; lvec -= *start;
      float len = lvec.len();
      if (len > ITRBasicLighting::distancePrecision) {
         lvec.normalize();
         m_cross(lvec,plane,&lnormal);
         if (!isEqual(lnormal.len(),.0f)) {
            planeList.increment();
            planeList.last().set(*start,lnormal);
         }
      }
      start = end;
   }
   

   for (int pl = 0; pl < planeList.size(); pl++)
      if (planeList[pl].whichSide(in_point) == TPlaneF::Outside)
         return false;

   return true;
}


//----------------------------------------------------------------------------
// Collide a line with the geometry.
// Sets ITRCollision::collisionPoint to the collision point
// closest to the start point.
//
// Copied and modified from ITRCollision - DMM
//
bool findCollisionSurfaceLine(int nodeIndex,const Point3F& start, const Point3F& end,
                              int *out_surfaceIndex);

// to replace ITRCollision internal variables...
ITRGeometry *geometry = NULL;
TMat3F      *transform = NULL;
LineSeg3F   line;

bool findCollisionSurface(ITRGeometry *in_geometry, TMat3F *in_transform,
                           const Point3F& start,const Point3F& end,
                           int *out_surfaceIndex)
{
   // Set up internal vars
   //
   geometry  = in_geometry;
   transform = in_transform;

   bool returnVal = findCollisionSurfaceLine(0,start,end,out_surfaceIndex);

   geometry  = NULL;
   transform = NULL;

   return returnVal;
}

bool findCollisionSurfaceLine(int nodeIndex,const Point3F& start,
   const Point3F& end, int *out_surfaceIndex)
{
   using ITRBasicLighting::distTolerance;

   if (nodeIndex >= 0) {
      ITRGeometry::BSPNode& node = geometry->nodeList[nodeIndex];
      TPlaneF& plane = geometry->planeList[node.planeIndex];
      TPlaneF::Side startSide = plane.whichSide(start);
      TPlaneF::Side endSide = plane.whichSide(end);
      switch(startSide * 3 + endSide) {
         case -4:    // S-, E-
         case -3:    // S-, E.
         case -1:    // S., E-
            return findCollisionSurfaceLine(node.back,start,end,out_surfaceIndex);
         case 4:     // S+, E+
         case 3:     // S+, E.
         case 1:     // S., E+
            return findCollisionSurfaceLine(node.front,start,end,out_surfaceIndex);
         case 0: {   // S., E.
            // Line lies on the plane.
            if (node.back >= 0)
               if (findCollisionSurfaceLine(node.back,start,end,out_surfaceIndex))
                  return true;
            if (node.front >= 0)
               if (findCollisionSurfaceLine(node.front,start,end,out_surfaceIndex))
                  return true;
            return false;
         }
         case -2: {  // S-, E+
            Point3F ip;
            plane.intersect(LineSeg3F(start,end),&ip);

            if (findCollisionSurfaceLine(node.back,start,ip,out_surfaceIndex))
               return true;
            return findCollisionSurfaceLine(node.front,ip,end,out_surfaceIndex);
         }
         case 2: {   // S+, E-
            Point3F ip;
            plane.intersect(LineSeg3F(start,end),&ip);

            if (findCollisionSurfaceLine(node.front,start,ip,out_surfaceIndex))
               return true;
            return findCollisionSurfaceLine(node.back,ip,end,out_surfaceIndex);
         }
      }
   }

   ITRGeometry::BSPLeafWrap leafWrap(geometry, -(nodeIndex+1));

   if (leafWrap.isSolid()) {
      sg_collisionPoint = start;
      Point3F collisionVec = end;
      collisionVec -= start;

      UInt8 *pbegin = &geometry->bitList[leafWrap.getSurfaceIndex()];
      UInt8 *pend   = pbegin + leafWrap.getSurfaceIndex();

      for (ITRCompressedBitVector::iterator itr(pbegin, pend); ++itr; ) {
         ITRGeometry::Surface &surface = geometry->surfaceList[*itr];
         TPlaneF &planeRef = geometry->planeList[surface.planeIndex];
         Point3F  normal    = planeRef;
         
         if (!surface.planeFront) {
            normal.neg();
         }

         float dot = m_dot(collisionVec, normal);
         if (dot >= 0.0f) {
            //  surface is either facing away from shooting ray, or is
            // parallel to it.
            //
            continue;
         }
         
         float distance = planeRef.distance(sg_collisionPoint);
         if (distance < -distTolerance || distance > distTolerance) {
            // point is not on the surface...
            continue;
         }

         if (pointContainedInSurf(geometry, surface, sg_collisionPoint)) {
            *out_surfaceIndex = *itr;
            return true;
         }
      }

      return false;
   }
   return false;
}

