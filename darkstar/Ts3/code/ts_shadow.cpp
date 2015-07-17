#include <base.h>
#include "ml.h"
#include <m_coll.h>
#include <d_caps.h>

#include <g_surfac.h>
#include <g_bitmap.h>
#include <gdManag.h>

#include "ts_shadow.h"
#include "ts_shapeInst.h"
#include "ts_renderContext.h"
#include "ts_pointArray.h"
#include "ts_light.h"

namespace TS
{
#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//#define debug


static VertexIndexPair vips[] = { {0,0}, {1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}, {7,7}, {8,8}, {9,9},
                                  {10,10}, {11,11}, {12,12}, {13,13}, {14,14}, {15,15}, {16,16}, 
                                  {17,17}, {18,18}, {19,19}, {20,20}, {21,21}, {22,22}, {23,23},
                                  {24,24}, {25,25}
                                };

// tolerance and method parameters
// for debugging only -- defaults should be used
float Shadow::tol1   = 0.05f;
float Shadow::tol2   = 0.1f;
float Shadow::tol3   = 0.1f;
float Shadow::tol5   = 0.05f;
int Shadow::doNullClip = 1;

// static arrays for shadows
Vector<Point2F> Shadow::projectedTextures;
Vector<Point3F> Shadow::projectedPoints;
Vector<int> Shadow::nSides;

Point3F Shadow::sourcePoints[4];
Point2F Shadow::sourceTextures[4] = { Point2F(0.0f,0.0f), Point2F(1.0f,0.0f),
                                      Point2F(1.0f,1.0f), Point2F(0.0f,1.0f) };

#define BufferBmpDim 256
static PointArray ShadowPointArray;
static MaterialList ShadowMaterial;
static GFXBitmap * BufferBmp = NULL;
static GFXSurface * BufferSfc = NULL;
static int PrevBufferInset = 0;

//-----------------------------------------------------------------

// for 2 x 2 blur
const int convWeights2[] =
{
   1, 1,
   1, 1
};

// for 3 x 3 blur
const int convWeights3[] =
{
   1, 2, 1,
   2, 3, 2,
   1, 2, 1,
};

// for 5 x 5 blur
const int convWeights5[] =
{
   1, 2, 3, 2, 1,
   2, 4, 5, 4, 2,
   3, 5, 6, 5, 3,
   2, 4, 5, 4, 2,
   1, 2, 3, 2, 1,
};

// grey tables for blur methods (really alpha tables)
UInt8 * greys2 = NULL;
UInt8 * greys23 = NULL;
UInt8 * greys3 = NULL;
UInt8 * greys5 = NULL;

//-----------------------------------------------------------------

int 
Poly::operator < (Poly const & p)
{ 
   if (fabs(minT-p.minT)>Shadow::tol1)
      return minT <= p.minT;
   else
      return maxT >= p.maxT; // the idea behind this second ineq. is to put the bigger poly
                             // in last if the minT is a virtual tie
}

void PolyList::clearPolys()
{
   for (iterator itr = begin(); itr != end(); itr++)
      delete *itr;
   clear();
}

PolyList::~PolyList()
{
   clearPolys();
}

class texturedPolyBuffer
{
   int numUsing;
   Vector<TexturedPoly*> array;
   public:
   TexturedPoly * getNew();
   void releasePolys(int numPop); // pop numPop off back of list
   texturedPolyBuffer() { numUsing=0; }
   ~texturedPolyBuffer();
};

//-----------------------------------------------------------------
// TexturedPolyBuffer
//-----------------------------------------------------------------

TexturedPoly * texturedPolyBuffer::getNew()
{
   if (numUsing<array.size())
   {
      TexturedPoly * retPoly;
      retPoly = array[numUsing++];
      retPoly->vertexList.clear();
      return retPoly;
   }
   array.increment();
   numUsing++;
   array.last() = new TexturedPoly;
   return array.last();
}

void texturedPolyBuffer::releasePolys(int numPop)
{
   numUsing -= numPop;
   AssertFatal(numUsing>=0,
      "TS::Shadow::texturedPolyStack::releasePolys:  releasing more polys than created");
}

texturedPolyBuffer::~texturedPolyBuffer()
{
   for (int i=0;i<array.size();i++)
      delete array[i];
}


//-----------------------------------------------------------------
// some global variables
//-----------------------------------------------------------------

static texturedPolyBuffer tpStack;

//-----------------------------------------------------------------
// partitionTree
//-----------------------------------------------------------------

class partitionTree
{
public:   
   class nodeList;

   class partitionNode
   {
      public:
      TSPlaneF clipPlane;
      int inNode;  // 0 and above is an index to another node, -1 is a null pointer,
      int outNode; // ditto
      int outsidePlane; // the plane "outside" this node, -1 by default
      int planeIndex; // 0 and above is an index to the plane to project to (& we are a leaf), -1 o.w.
      void addClip(Point3F & a, Point3F & b, TSPlaneF * clip, 
                   int projPlane, nodeList & pNodes);
      void partition(TexturedPoly *inPoly, nodeList & pNodes);
      void emitPoly(TexturedPoly *outPoly);
   };

   class nodeList
   {
      Vector<partitionNode*> array;
      int numUsing;
      public:
      nodeList() { numUsing=0; array.clear(); }
      ~nodeList();
      partitionNode & operator[](int idx) { return *array[idx]; }
      void clear() { numUsing=0; }
      bool empty() { return numUsing==0; }
      int size() { return numUsing; }
      int addPlaneNode(int projPlane, int outPlane);
      int addClipNode(TSPlaneF * clip, int projPlane, int outPlane);
   };

   static nodeList pNodes;
   Point3F vec[3];

   public:

   void clear() { pNodes.clear(); }
   bool empty() { return pNodes.empty(); }

   void addClip(Point3F & a, Point3F & b, TSPlaneF & clip, int projPlane);
   void partition(TexturedPoly *inPoly, int startNode = 0);
   int getPlane( int nodeNum ) { return pNodes[nodeNum].planeIndex; }

} shadowPartition;

Shadow * partitionShadow = NULL;
partitionTree::nodeList partitionTree::pNodes;

//-------------------------------------------------------------------------------
// partitionTree interface functions
//-------------------------------------------------------------------------------

void partitionTree::addClip(Point3F & a, Point3F & b, TSPlaneF & clip, int projPlane)
{
   if (pNodes.empty())
      pNodes.addClipNode(&clip,projPlane,-1);
   else
      pNodes[0].addClip(a,b,&clip,projPlane,pNodes);
}

void partitionTree::partition(TexturedPoly *inPoly, int startNode)
{ 
   pNodes[startNode].partition(inPoly,pNodes);
}

//-------------------------------------------------------------------------------
// partitionTree::nodeList methods
//-------------------------------------------------------------------------------
partitionTree::nodeList::~nodeList()
{
   while (array.size())
   {
      delete array.last();
      array.pop_back();
   }
}   

// add a node that labels this inside region with the projection plane
int partitionTree::nodeList::addPlaneNode(int projPlane, int outPlane)
{
   int newNode = numUsing++;
   if (numUsing>array.size())
   {
      array.increment();
      array[newNode] = new partitionNode;
   }
   array[newNode]->planeIndex = projPlane;
   array[newNode]->outsidePlane = outPlane;
   return newNode;
}

// add a node to the currently outside region which splits this region into
// a now inside region (labeled w/ projPlane) and a still outside region
int partitionTree::nodeList::addClipNode(TSPlaneF * clip, int projPlane, int outPlane)
{
   if (projPlane==outPlane)
      return addPlaneNode(projPlane,outPlane);

   if (clip==0)
      return addPlaneNode(projPlane,outPlane);

   int newNode = numUsing++;
   if (numUsing>array.size())
   {
      array.increment();
      array[newNode]= new partitionNode;
   }
   array[newNode]->clipPlane = *clip;
   array[newNode]->outsidePlane = outPlane;
   array[newNode]->planeIndex = -1;
   array[newNode]->inNode = addPlaneNode(projPlane,outPlane);
   if (outPlane==-1)
      array[newNode]->outNode = -1;
   else
      array[newNode]->outNode = addPlaneNode(outPlane,outPlane);

   return newNode;
}

//-------------------------------------------------------------------------------
// partitionTree:partitionNode methods
//-------------------------------------------------------------------------------

void partitionTree::partitionNode::addClip(Point3F & a, Point3F & b,
                                           TSPlaneF * clip, int projPlane,
                                           nodeList & pNodes)
{
   if (planeIndex!=-1)
   {
      if (planeIndex!=projPlane)
      {
         // if new plane is not "closer" than old plane, we don't do anything
         float t;
         PlaneEntry & oldPlane = partitionShadow->planeStack[planeIndex];
         t = (oldPlane.fPlaneConstant - m_dot(oldPlane.fNormal,a)) * oldPlane.fInvDot;
         if (fabs(t)<Shadow::tol2)
         {
            t = (oldPlane.fPlaneConstant - m_dot(oldPlane.fNormal,b)) * oldPlane.fInvDot;
            if (fabs(t)<Shadow::tol2)
            {
               Point3F c = a;
               if (clip!=0)
                  c -= *clip;
               else
                  c -= clipPlane;
               t  = (oldPlane.fPlaneConstant - m_dot(oldPlane.fNormal,c)) * oldPlane.fInvDot;
               PlaneEntry & newPlane = partitionShadow->planeStack[projPlane];
               t -= (newPlane.fPlaneConstant - m_dot(newPlane.fNormal,c)) * newPlane.fInvDot;
            }
         }

         if (t<=0.0f)
            return; // behind other plane
         // turn this node into a clip node...make the outside be planeIndex
         // unless clip=0, in which case this node stays a plane node with new planeIndex
         outsidePlane = planeIndex;
         if (clip==0)
            planeIndex=projPlane;
         else
         {
            inNode = pNodes.addPlaneNode(projPlane,outsidePlane);
            outNode = pNodes.addPlaneNode(planeIndex,outsidePlane);
            planeIndex = -1;
            clipPlane=*clip;
         }
         return;
      }
      // o.w., we are closing a poly, so constrict this region more
      // except that if clip=0, no constricting (since no clipping)
      if (clip!=0)
      {
         planeIndex = -1;
         clipPlane = *clip;
         inNode = pNodes.addPlaneNode(projPlane,outsidePlane);

         if (outsidePlane==-1)
            outNode = -1;
         else
            outNode = pNodes.addPlaneNode(outsidePlane,outsidePlane);
      }

      return;
   }

   TSPlaneF::Side aSide,bSide;

   // -1 for outside, 0 for on plane, +1 for inside -- this must be true or else code will fail
   aSide = clipPlane.whichSide(a);
   bSide = clipPlane.whichSide(b);

   // if both points on clipPlane, which way should we go -- depends on direction of new clipPlane
   if (aSide==0 && bSide==0)
   {
      if (!clip
            || fabs(fabs(clip->x)-fabs(clipPlane.x))>Shadow::tol3 
            || fabs(fabs(clip->y)-fabs(clipPlane.y))>Shadow::tol3 
            || fabs(fabs(clip->z)-fabs(clipPlane.z))>Shadow::tol3)
      {
         // shouldn't have both been on plane, so tighten up the precision
         float oldP = TSPlaneF::DistancePrecision;
         TSPlaneF::DistancePrecision=.000001f;
         aSide = clipPlane.whichSide(a);
         bSide = clipPlane.whichSide(b);
         TSPlaneF::DistancePrecision=oldP;
//         AssertFatal(!clip || aSide!=0 || bSide!=0,"oops");
         // the following shouldn't need to be here.
         // this is a stopgap measure that should cause no problems
         // at some point we need to figure out why it occasionally
         // gets to this point with clip!=0
         if (clip)
         {
            if (m_dot(*clip,clipPlane)>0.0f)
               aSide = TSPlaneF::Inside; // inside
            else
               aSide = TSPlaneF::Outside; // outside
            if (Shadow::doNullClip==1)
               clip=0; // no need to duplicate the clip-job
         }
      }
      else 
      {
         if (m_dot(*clip,clipPlane)>0.0f)
            aSide = TSPlaneF::Inside; // inside
         else
            aSide = TSPlaneF::Outside; // outside
         if (Shadow::doNullClip==1)
            clip=0; // no need to duplicate the clip-job
      }
   }

   float t;
   Point3F c;
   switch (aSide * 3 + bSide)
   {
      case -4: // A-,B-
      case -1:   // A.,B-
      case -3: // A-,B.
         if (outNode==-1)
            outNode = pNodes.addClipNode(clip,projPlane,outsidePlane);
         else
            pNodes[outNode].addClip(a,b,clip,projPlane,pNodes);
         break;
      case 4: // A+,B+
      case 1: // A.,B+
      case 3: // A+,B.
         pNodes[inNode].addClip(a,b,clip,projPlane,pNodes);
         break;
      case -2: // A-,B+
         clipPlane.intersect(a,b,&t);
         c.x = a.x + t * (b.x-a.x);
         c.y = a.y + t * (b.y-a.y);
         c.z = a.z + t * (b.z-a.z);
         if (outNode==-1)
            outNode = pNodes.addClipNode(clip,projPlane,outsidePlane);
         else
            pNodes[outNode].addClip(a,c,clip,projPlane,pNodes);
         pNodes[inNode].addClip(c,b,clip,projPlane,pNodes);
         break;
      case 2: // A+,B-
         clipPlane.intersect(a,b,&t);
         c.x = a.x + t * (b.x-a.x);
         c.y = a.y + t * (b.y-a.y);
         c.z = a.z + t * (b.z-a.z);
         pNodes[inNode].addClip(a,c,clip,projPlane,pNodes);
         if (outNode==-1)
            outNode = pNodes.addClipNode(clip,projPlane,outsidePlane);
         else
            pNodes[outNode].addClip(c,b,clip,projPlane,pNodes);
         break;
      case 0: // A.,B.
         AssertFatal(!clip,"TS::Shadow::partitionTree::partitionNode::addClip:  shouldn't get here like this");
         pNodes[inNode].addClip(a,b,0,projPlane,pNodes);
         if (outNode==-1)
            outNode = pNodes.addClipNode(0,projPlane,outsidePlane);
         else
            pNodes[outNode].addClip(a,b,0,projPlane,pNodes);
         break;
   }
}

void partitionTree::partitionNode::partition(TexturedPoly *inPoly, nodeList & pNodes)
{
   if (planeIndex!=-1)
   {
      emitPoly(inPoly);
      return;
   }

   // o.w., we have some partitioning to do yet
   TexturedPoly *newFront = tpStack.getNew();
   TexturedPoly *newBack = tpStack.getNew();

   inPoly->split(clipPlane,newFront,newBack);
   if (inNode!=-1 && !newFront->vertexList.empty())
      pNodes[inNode].partition(newFront,pNodes);

   if (outNode!=-1 && !newBack->vertexList.empty())
      pNodes[outNode].partition(newBack,pNodes);

   // release front and back polys
   tpStack.releasePolys(2);
}

void 
partitionTree::partitionNode::emitPoly(TexturedPoly *outPoly)
{
   PlaneEntry & thePlane = partitionShadow->planeStack[planeIndex];

   // don't render if plane not facing camera
   if (!thePlane.faceCamera)
      return;

   int sz=outPoly->vertexList.size();
   for (int i=0;i<sz;i++)
   {
      partitionShadow->projectedPoints.increment();
      partitionShadow->project(partitionShadow->lightInWorld,
                               thePlane.fNormal,thePlane.fPlaneConstant,thePlane.fInvDot,
                               outPoly->vertexList[i].point,
                               partitionShadow->projectedPoints.last());
      partitionShadow->projectedTextures.push_back(outPoly->vertexList[i].texture);
   }
   partitionShadow->nSides.push_back(sz);
}

//-------------------------------------------------------------------------------
// Shadow methods
//-------------------------------------------------------------------------------
Shadow::Shadow()
{
   planeStack.reserve(15);
   
   shadowLift = 0.01f; 
   alphaLevel = 0.5f;
   
   shadowBmp = NULL;
   surface = NULL;
   camera = NULL;
   lights = NULL;
}

//-----------------------------------------------------------------------------
// getPlanes:  get the plane of each poly, and add points and edges to the
//             lists of all pnts and edges (makes it easier to detect repeats)
//-----------------------------------------------------------------------------

void Shadow::getPlanes(const Point3F & cameraCenter, const Point3F & camY)
{
   planeStack.clear();
   maxPlaneDot = 0;

   Point3F offset = lightInWorld;
   offset *= shadowLift;

   int i,j;
   for (i=0;i<projectionList.size();i++)
   {
      Point3F planeNormal;
      Poly & planePoly = *projectionList[i];
      AssertFatal(planePoly.vertexList.size()>=3,"TS::Shadow::getPlanes:  degenerate projection poly");
      Point3F v1 = planePoly.vertexList[0].point - offset;
      Point3F v2 = planePoly.vertexList[1].point - offset;
      Point3F v3 = planePoly.vertexList[2].point - offset;
      m_normal(v1,v2,v3,planeNormal);
      planeNormal.normalize();
      float planeConstant = m_dot(v1,planeNormal);

      // new plane?
      for (j=0;j<planeStack.size();j++)
      {
         Point3F & n = planeStack[j].fNormal;
         float & d = planeStack[j].fPlaneConstant;
         if (IsEqual(planeConstant,d,0.01f) &&
             IsEqual(planeNormal.x,n.x,0.01f) &&
             IsEqual(planeNormal.y,n.y,0.01f) &&
             IsEqual(planeNormal.z,n.z,0.01f))
             break;
      }
      int planeNum=j;
      if (planeNum==planeStack.size())
      {
         // add new plane entry
         planeStack.increment();
         PlaneEntry & newPlane = planeStack.last();
         newPlane.fNormal = planeNormal;
         newPlane.fPlaneConstant = planeConstant;
         float dot = m_dot(lightInWorld,planeNormal);
         if (dot*dot > 0.01f*0.01f)
         {
            newPlane.fInvDot = 1.0f / dot;
            dot = m_dot(camY,planeNormal);
            if (-dot > maxPlaneDot)
               maxPlaneDot = -dot;
         }
         else
            newPlane.fInvDot = 100.0f; // won't be used since positive
         newPlane.faceCamera = m_dot(newPlane.fNormal,cameraCenter) >= newPlane.fPlaneConstant;
      }

      // add the last bit of info to poly
      float minT = m_dot(v1,lightInWorld);
      float maxT = minT;
      for (j=1;j<planePoly.vertexList.size();j++)
      {
         float tmpF = m_dot(planePoly.vertexList[j].point,lightInWorld);
         if (tmpF<minT)
            minT=tmpF;
         else if (tmpF>maxT)
            maxT=tmpF;
      }
      planePoly.minT = minT;
      planePoly.maxT = maxT;
      planePoly.planeIndex = planeNum;
   }
}

void Shadow::buildPartitionTree()
{
   shadowPartition.clear();
   TSPlaneF::DistancePrecision = tol5;
   TSPlaneF::NormalPrecision = .01f;

   Point3F offset = lightInWorld;
   offset *= shadowLift;

   // sort polys according to order (first poly closest to light)
   projectionList.sort();

   // now add points and edges to iPoints and iEdges
   for (int i=0;i<projectionList.size();i++)
   {
      Poly & planePoly = *projectionList[i];
      int planeIndex = planePoly.planeIndex;

      if (planeIndex==-1)
         continue; // don't use, this poly was disqualified for some reason

      // don't add to tree if plane is back-facing to light (shouldn't happen anymore
      // unless almost parallel to light in which case we set fInvDot=100)
      if ( planeStack[planeIndex].fInvDot>=0.0f )
         continue;

      int sz = planePoly.vertexList.size();
      Point3F a = planePoly.vertexList[sz-1].point - offset, b;
      for (int j=0;j<sz;j++)
      {
         b = planePoly.vertexList[j].point - offset;

         Point3F edge,clipNormal;
         edge = b - a;
         m_cross(lightInWorld,edge,&clipNormal);
         TSPlaneF clipPlane;
         clipPlane.set(a,clipNormal);

         shadowPartition.addClip(a,b,clipPlane,planeIndex);

         a=b;
      }
   }
}

void
Shadow::setBitmapSize(GFXDeviceManager * gfxDeviceManager, int _bmpDim, GFXSurface * flushSfc)
{
   if (flushSfc)
      flushSfc->flushTexture(shadowBmp,false);

   bmpDim = _bmpDim;

   // make sure buffer bmp exists -- this bitmap used for rendering
   // into when we have blur ...
   if (!BufferBmp)
      BufferBmp = GFXBitmap::create(BufferBmpDim,BufferBmpDim);

   // ... and we draw into this surface when we have blur
   if (!BufferSfc)
      BufferSfc = gfxDeviceManager->createMemSurface(BufferBmp);
   
   clearResources(gfxDeviceManager,false);

   // this is the bitmap we use for rendering
   shadowBmp = GFXBitmap::create(bmpDim,bmpDim);

   // this surface only used if we don't blur
   surface = gfxDeviceManager->createMemSurface(shadowBmp);

   // we use this camera whether or not we have blur
   if (!camera)
      camera = new TS::OrthographicCamera(RectI(),RectF(),1,1.0E8f);

   if (!lights)
   {
      lights = new TSSceneLighting;
      lights->setAmbientIntensity(ColorF(1,1,1));
   }
}

void
Shadow::clearResources(GFXDeviceManager * gfxDeviceManager, bool clearCameraToo)
{
   if (surface)
      gfxDeviceManager->freeMemSurface(surface);
   surface = NULL;
   delete shadowBmp;
   shadowBmp = NULL;
   if (clearCameraToo)
   {
      delete camera;
      camera = NULL;
      delete lights;
      lights = NULL;
   }
}

// calculate the shape space window we look through when taking our picture
void Shadow::calcSourceWindow(ShapeInstance * shape, RMat3F & shapeToWorld)
{
   // compute rx and rz -- distances from center to the left/right
   // and up/down extents in light space
   Box3F box;
   shape->getNode(0)->fObjectList[0]->getBox(shape,box);
   Point3F radii = box.fMax - box.fMin;
   radii *= 0.5f;
   Point3F x,y,z;
   shapeToWorld.getRow(0,&x);
   shapeToWorld.getRow(1,&y);
   shapeToWorld.getRow(2,&z);
   float rx = radii.x * fabs(m_dot(x,vRight)) +
              radii.y * fabs(m_dot(y,vRight)) +
              radii.z * fabs(m_dot(z,vRight));
   float rz = radii.x * fabs(m_dot(x,vUp)) +
              radii.y * fabs(m_dot(y,vUp)) +
              radii.z * fabs(m_dot(z,vUp));

   // shadow poly has to be square or we have warping problems...
   shadowRadius = rx > rz ? rx : rz;

   // figure out shadow poly (this poly sits at the camera
   // and is projected to the ground)
   Point3F vr = vRight;
   Point3F vu = vUp;
   vr *= shadowRadius;
   vu *= shadowRadius;
   ul  =  vu - vr;
   ur  =  vu + vr;
   ll  = -vu - vr;
   lr  = -vu + vr;
   ul += shadowPolyCenter;
   ur += shadowPolyCenter;
   ll += shadowPolyCenter;
   lr += shadowPolyCenter;
}

void
Shadow::getShadowBitmap(ShapeInstance * shape, GFXPalette * pal, RMat3F & shapeToWorld, int blurMethod)
{
   // assumes calcSourceWindow already called
   
   bool useBlur = blurMethod;
   bool useOversampling = blurMethod > 20;

   // if we use blur we use BufferSfc if we don't we use sfc...
   GFXSurface * sfc = useBlur ? BufferSfc : surface;
   
   // set the palette
   sfc->setPalette(pal);

   // set up viewports
   camera->setWorldViewport(RectF(-shadowRadius,shadowRadius,shadowRadius,-shadowRadius));
   int dim = useOversampling ? bmpDim<<1 : bmpDim;
   int inset = useBlur ? (BufferBmpDim - dim) >> 1 : 0;
   camera->setScreenViewport(RectI(inset,inset,inset+dim,inset+dim));
   camera->setTWC(tc);
   
   // set up the render context
   renderContext.setCamera(camera);
   ShadowPointArray.reset();
   ShadowPointArray.setOrtho(true);
   renderContext.setPointArray(&ShadowPointArray);
   renderContext.setSurface(sfc);
   renderContext.setLights(lights);

   // lock
   renderContext.lock();
   sfc->draw3DBegin();
   
   // have we created the shadow materials yet?
   if (ShadowMaterial.getMaterialsCount()==0)
   {
      ShadowMaterial.setSize(1,1);
      ShadowMaterial[0].fParams.fFlags = Material::MatRGB | Material::ShadingNone;
      ShadowMaterial[0].fParams.fRGB.fRed = 10;
      ShadowMaterial[0].fParams.fRGB.fGreen = 10;
      ShadowMaterial[0].fParams.fRGB.fBlue = 10;
      
      // ok, let's call a spade a spade -- this is dangerous
      // we're casting a float to a resource manager -- yikes
      // however, rm never actually used (and we can't get the rm from here
      // without some trouble, so...)
      float fool;
      ShadowMaterial.load((ResourceManager&)fool,true);
   }
   
   // clear the bitmap
   static Point3F clear[4];
   if (useBlur)
   {
      // drawing into buffer bitmap...clear to previous inset
      clear[0].set(PrevBufferInset               , PrevBufferInset               , 0.0f);
      clear[1].set(BufferBmpDim - PrevBufferInset, PrevBufferInset               , 0.0f);
      clear[2].set(BufferBmpDim - PrevBufferInset, BufferBmpDim - PrevBufferInset, 0.0f);
      clear[3].set(PrevBufferInset               , BufferBmpDim - PrevBufferInset, 0.0f);
      PrevBufferInset = inset;
   }
   else
   {
      // drawing directly into rendering bitmap...clear to edges
      clear[0].set(0      , 0  , 0.0f);
      clear[1].set(dim    , 0  , 0.0f);
      clear[2].set(dim    , dim, 0.0f);
      clear[3].set(0      , dim, 0.0f);
   }
   sfc->setFillMode(GFX_FILL_CONSTANT);
   sfc->setHazeSource(GFX_HAZE_NONE);
   sfc->setShadeSource(GFX_SHADE_NONE);
   sfc->setAlphaSource(GFX_ALPHA_NONE);
   sfc->setFillColor(Int32(0));
   sfc->setZTest(GFX_ZWRITE);
   sfc->addVertex(&clear[0]);
   sfc->addVertex(&clear[1]);
   sfc->addVertex(&clear[2]);
   sfc->addVertex(&clear[3]);
   sfc->emitPoly();
   sfc->setZTest(GFX_ZTEST_AND_WRITE);

   // use a floor?  chops off underground part of shape
   float k1,k2;
   if (useFloor && (!IsZero( k1 = m_dot(floorNormal,lightInWorld), 0.01f)))
   {
      k1 = 1.0f/k1;
      k2 = m_dot(floorNormal,floorPoint);
      
      float t;

      // project ul in direction of light down to floor
      t = (k2 - m_dot(floorNormal,ul)) * k1;
      clear[0] = lightInWorld;
      clear[0] *= t;
      clear[0] += ul;

      // project ur in direction of light down to floor
      t = (k2 - m_dot(floorNormal,ur)) * k1;
      clear[1] = lightInWorld;
      clear[1] *= t;
      clear[1] += ur;

      // project ll in direction of light down to floor
      t = (k2 - m_dot(floorNormal,lr)) * k1;
      clear[2] = lightInWorld;
      clear[2] *= t;
      clear[2] += lr;

      // project lr in direction of light down to floor
      t = (k2 - m_dot(floorNormal,ll)) * k1;
      clear[3] = lightInWorld;
      clear[3] *= t;
      clear[3] += ll;

      // draw floor poly
      TSPointArray * pa = renderContext.getPointArray();
      camera->pushVisibility(ClipMask);

      pa->reset();
      pa->setVisibility(ClipMask);
      pa->useTextures(false);
      pa->useIntensities(false);
      pa->useBackFaceTest(true);

      sfc->setFillMode(GFX_FILL_CONSTANT);
      sfc->setHazeSource(GFX_HAZE_NONE);
      sfc->setShadeSource(GFX_SHADE_NONE);
      sfc->setAlphaSource(GFX_ALPHA_NONE);
      sfc->setTransparency(false);
      sfc->setFillColor(Int32(0));

      sfc->setZMode(false);

      pa->addPoints(4,clear);
      pa->drawPoly(4,vips,0);

      camera->popVisibility();
   }

   // rotate shape as rotated in world
   TMat3F tmp;
   (RMat3F&)tmp = shapeToWorld;
   tmp.p.set();
   camera->pushTransform(tmp);
   
   bool  alAlpha    = shape->getAlwaysAlpha();
   float alAlphaVal = shape->getAlwaysAlphaValue();
   shape->setAlphaAlways(false, 1.0);
   // set material list to shadow material and render
   ShapeInstance::setAlwaysMat(&ShadowMaterial);
   shape->render(renderContext);
   ShapeInstance::setAlwaysMat(NULL);
   shape->setAlphaAlways(alAlpha, alAlphaVal);
   
   camera->popTransform();

   // Unlock
   sfc->draw3DEnd();
	renderContext.unlock();

   if (useBlur)
   {
      blurBitmap(blurMethod);
      shadowBmp->attribute |=  BMA_TRANSLUCENT;
      shadowBmp->attribute &= ~BMA_TRANSPARENT;
      shadowBmp->paletteIndex = -2;
   }
   else
   {
      shadowBmp->attribute &= ~BMA_TRANSLUCENT;
      shadowBmp->attribute |=  BMA_TRANSPARENT;
      shadowBmp->paletteIndex = -1;
   }

   sfc->setZMode(true);
   newBmp = true;
}

void
Shadow::setPosition( const Point3F & pos)
{
   posInWorld = pos;
}

void 
Shadow::setLight( const Point3F & lightDirection, TSShapeInstance * shape)
{
   lightInWorld = lightDirection;

   float radius = shape->getShape().fRadius;

   // build light's "camera" transform
   Point3F x,y=lightInWorld,z;
   if (y.z*y.z < 0.99f)
   {
      m_cross(y,Point3F(0,0,1),&x);
      x.normalize();
      m_cross(x,y,&z);
   }
   else
   {
      m_cross(Point3F(1,0,0),y,&z);
      z.normalize();
      m_cross(y,z,&x);
   }
   tc.setRow(0,x);
   tc.setRow(1,y);
   tc.setRow(2,z);
   tc.p = y;
   tc.p *= -radius-2;
   tc.p += shape->getShape().fCenter + shape->fRootDeltaTransform.p;
RMat3F & r = shape->fRootDeltaTransform;
AssertFatal( IsZero(r.m[0][1],0.01f) && IsZero(r.m[0][2],0.01f) &&
             IsZero(r.m[1][0],0.01f) && IsZero(r.m[1][2],0.01f) &&
             IsZero(r.m[2][0],0.01f) && IsZero(r.m[2][1],0.01f) &&
             IsZero(r.m[0][0]-1,0.01f) && IsZero(r.m[1][1]-1,0.01f) && IsZero(r.m[2][2]-1,0.01f), "boo hoo");
   tc.flags |= TMat3F::Matrix_HasTranslation|TMat3F::Matrix_HasRotation;

   shadowPolyCenter = tc.p;

   // tc is light to world right now
   // make it world to camera  
   TLW = tc;
   tc.inverse();

   // following needed for building the shadow poly
   // (the poly we projet onto the ground)
   vRight = x;
   vUp = z;
}

// project all points in source list onto plane (put into pointArray)
void 
Shadow::project(const Point3F & direction, const Point3F & planeNormal, 
                float planeConstant, float invDot)
{
   if (invDot>=0.0f)
      return;

   projectedPoints.setSize(4);
   project(direction,planeNormal,planeConstant,invDot,sourcePoints[0],projectedPoints[0]);
   project(direction,planeNormal,planeConstant,invDot,sourcePoints[1],projectedPoints[1]);
   project(direction,planeNormal,planeConstant,invDot,sourcePoints[2],projectedPoints[2]);
   project(direction,planeNormal,planeConstant,invDot,sourcePoints[3],projectedPoints[3]);
}

// simple render case
// use VIPS from sourceList, textures from sourceList, and put points straight into pointarray
void 
Shadow::renderPlane( PointArray * pa, PlaneEntry & pe)
{
   project( lightInWorld, pe.fNormal, pe.fPlaneConstant, pe.fInvDot);

   // put the projected points into the point array
   pa->addPoints(4,projectedPoints.address());
   pa->useTextures(sourceTextures);

   // draw the poly
   pa->drawPoly(4,vips,0);
}


void 
Shadow::renderPartition( PointArray * pa)
{
   int i;

   projectedPoints.clear();
   projectedTextures.clear();
   nSides.clear();

   TexturedPoly *workPoly = tpStack.getNew();
   if (getPoly(workPoly))
      shadowPartition.partition(workPoly);
   tpStack.releasePolys(1);
   
   if (projectedPoints.empty())
      return;

   // now add the points collected during partitioning to pointArray
   pa->addPoints(projectedPoints.size(),projectedPoints.address());

   // draw the polys
   int outVerts=0;
   for (i=0;i<nSides.size();i++)
   {
      pa->useTextures( projectedTextures.address() + outVerts );
      pa->drawPoly(nSides[i],vips,outVerts);
      outVerts+=nSides[i];
   }
}

// puts unpackedFace from sourceList into a TexturedPoly
// returns false if faces away from light
bool 
Shadow::getPoly(TexturedPoly * p)
{
   Point3F & v1 = sourcePoints[0];
   Point3F & v2 = sourcePoints[1];
   Point3F & v3 = sourcePoints[2];
   Point3F & v4 = sourcePoints[3];

   Point3F faceNormal;
   m_normal(v1,v2,v3,faceNormal);
   if (m_dot(faceNormal,lightInWorld) >= 0.0f)
      return false; // back face

   p->vertexList.setSize(4);

   p->vertexList[0].point = v1;
   p->vertexList[1].point = v2;
   p->vertexList[2].point = v3;
   p->vertexList[3].point = v4;

   p->vertexList[0].texture = sourceTextures[0];
   p->vertexList[1].texture = sourceTextures[1];
   p->vertexList[2].texture = sourceTextures[2];
   p->vertexList[3].texture = sourceTextures[3];

   return true;
}

void 
Shadow::render( RenderContext & rc)
{
   if (projectionList.empty())
      return;

   // get and reset point array:
   PointArray *pa = rc.getPointArray();
   pa->reset();
   pa->useTextures( true );
   pa->useIntensities(false);
   pa->useBackFaceTest(false);

   // set up surface parameters
   GFXSurface *srf = rc.getSurface();
   srf->setFillMode(GFX_FILL_TEXTURE);
   srf->setHazeSource(GFX_HAZE_NONE);
   srf->setShadeSource(GFX_SHADE_NONE);
   srf->setTexturePerspective(true);
   
   // set up depending on mode
   if (shadowBmp->paletteIndex == (UInt32) -2)
   {
      srf->setTransparency(false);
      srf->setAlphaSource(GFX_ALPHA_TEXTURE);
   }
   else if (srf->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA)
   {
      srf->setTransparency(true);
      srf->setAlphaSource(GFX_ALPHA_CONSTANT);
      srf->setConstantAlpha(alphaLevel);
   }
   else
   {
      srf->setTransparency(true);
      srf->setAlphaSource(GFX_ALPHA_NONE);
   }

   srf->setTextureMap(shadowBmp);
   if (newBmp)
   {
      srf->flushTexture(shadowBmp,true);
      newBmp = false;
   }

   // build poly
   sourcePoints[0] = ul + posInWorld;
   sourcePoints[1] = ur + posInWorld;
   sourcePoints[2] = lr + posInWorld;
   sourcePoints[3] = ll + posInWorld;

   if (planeStack.size()==1)
   {
      PlaneEntry & thePlane = planeStack[0];
      // don't render plane if back-facing to light or camera
      if (thePlane.fInvDot<0.0f && thePlane.faceCamera)
      {
         renderPlane(pa,thePlane);
         pa->reset();
      }
      srf->setAlphaSource(GFX_ALPHA_NONE);
      return;
   }

   partitionShadow = this;
   buildPartitionTree();

   if (shadowPartition.empty())
      return;

   renderPartition(pa);

   srf->setAlphaSource(GFX_ALPHA_NONE);
}

//-----------------------------------------------------------------------------
void Shadow::cachePolys()
{
   int i;
   
   // delete old cache
   cache.nSides.clear();
   cache.projectedPoints.clear();
   cache.projectedTextures.clear();
   
   // this is reset since it depends on camera position
   maxPlaneDot = 1;

   if (projectionList.empty())
      return;

   // get planes
   Point3F zeroV(0,0,0);
   getPlanes(zeroV,zeroV);
   
   // make all planes visible from camera
   for (i=0;i<planeStack.size();i++)
      planeStack[i].faceCamera = true;

   float saveLift = shadowLift;
   shadowLift = 0;

   partitionShadow = this;

   // build poly
   sourcePoints[0] = ul + posInWorld;
   sourcePoints[1] = ur + posInWorld;
   sourcePoints[2] = lr + posInWorld;
   sourcePoints[3] = ll + posInWorld;

   projectedPoints.clear();
   projectedTextures.clear();
   nSides.clear();

   if (planeStack.size()==1)
   {
      PlaneEntry & thePlane = planeStack[0];
      // don't render plane if back-facing to light or camera
      if (thePlane.fInvDot<0.0f && thePlane.faceCamera)
      {
         // fill up projection arrays
         project( lightInWorld, thePlane.fNormal, thePlane.fPlaneConstant, thePlane.fInvDot);
         nSides.setSize(1);
         nSides[0] = 4;
         projectedTextures.setSize(4);
         projectedTextures[0] = sourceTextures[0];
         projectedTextures[1] = sourceTextures[1];
         projectedTextures[2] = sourceTextures[2];
         projectedTextures[3] = sourceTextures[3];
      }
   }
   else
   {
      buildPartitionTree();
      if (!shadowPartition.empty())
      {
         // fill up projection arrays
         TexturedPoly *workPoly = tpStack.getNew();
         if (getPoly(workPoly))
            shadowPartition.partition(workPoly);
         tpStack.releasePolys(1);
      }
   }

   // copy from projection arrays to cache...only happens once,
   // so no need to worry about extra time spent on copy

   cache.nSides = nSides;
   cache.projectedPoints = projectedPoints;
   cache.projectedTextures = projectedTextures;
   
   shadowLift = saveLift;
}

//-----------------------------------------------------------------------------
void Shadow::renderCache( RenderContext & rc)
{
   int i;
   
   if (cache.nSides.size()==0)
      return;

   // get and reset point array:
   PointArray *pa = rc.getPointArray();
   pa->reset();
   pa->useTextures( true );
   pa->useIntensities(false);
//   pa->useBackFaceTest(false);
   pa->useBackFaceTest(true);

   // set up surface parameters
   GFXSurface *srf = rc.getSurface();
   srf->setFillMode(GFX_FILL_TEXTURE);
   srf->setHazeSource(GFX_HAZE_NONE);
   srf->setShadeSource(GFX_SHADE_NONE);
   srf->setTexturePerspective(true);
   
   // set up depending on mode
   if (shadowBmp->paletteIndex == (UInt32) -2)
   {
      srf->setTransparency(false);
      srf->setAlphaSource(GFX_ALPHA_TEXTURE);
   }
   else if (srf->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA)
   {
      srf->setTransparency(true);
      srf->setAlphaSource(GFX_ALPHA_CONSTANT);
      srf->setConstantAlpha(alphaLevel);
   }
   else
   {
      srf->setTransparency(true);
      srf->setAlphaSource(GFX_ALPHA_NONE);
   }

   srf->setTextureMap(shadowBmp);
   if (newBmp)
   {
      srf->flushTexture(shadowBmp,true);
      newBmp = false;
   }
   
   // copy over projected points plus shadow lift -- lame, but need
   // to do because shadowLift changes depending on other shadows (for sorting)
   Point3F lift = lightInWorld;
   lift *= shadowLift;
   projectedPoints = cache.projectedPoints;
   for (i=0;i<projectedPoints.size();i++)
      projectedPoints[i] -= lift;

   // now add these points to point array
   pa->addPoints(projectedPoints.size(),projectedPoints.address());

   // draw the polys
   int outVerts=0;
   for (i=0;i<cache.nSides.size();i++)
   {
      pa->useTextures( cache.projectedTextures.address() + outVerts );
      pa->drawPoly(cache.nSides[i],vips,outVerts);
      outVerts+=cache.nSides[i];
   }

   srf->setAlphaSource(GFX_ALPHA_NONE);
}

//-----------------------------------------------------------------------------

void Shadow::setAlphaLevel(float _alpha)
{
   alphaLevel = _alpha;
}

//-----------------------------------------------------------------------------
// blur routines
//-----------------------------------------------------------------------------

void Shadow::blurBitmap(int blurMethod)
{
   switch (blurMethod)
   {
      case  2 :  blurBitmap2(); return;
      case  3 :  blurBitmap3(); return;
      case  5 :  blurBitmap5(); return;
      case 23 : blurBitmap23(); return;
   }
}

//-----------------------------------------------------------------------------
// 2 x 2 blur
//-----------------------------------------------------------------------------

void Shadow::blurBitmap2()
{
   if (!greys2)
   {
      int i , sum = 0;
      for (i=0; i < 4; i++)
         sum += convWeights2[i];
         
      greys2 = new UInt8[sum+1];
      // only need to set up once
      for (i=0;i<sum+1;i++)
         greys2[i] = 255.0f * ((float)i)/(float)sum;
   }
   
   int inset = (BufferBmpDim - bmpDim) >> 1;

   UInt8 * s1 = BufferBmp->pBits + (inset-1) * BufferBmpDim + inset - 1;
   UInt8 * s2 = BufferBmp->pBits +     inset * BufferBmpDim + inset - 1;

   int skip = BufferBmpDim - bmpDim;

   UInt8 * t = shadowBmp->pBits;
   UInt8 * e = shadowBmp->pBits + bmpDim * bmpDim;   

   for (; t!=e;)
   {
      UInt8 * lineEnd = s2 + bmpDim;
      for (; s2!= lineEnd;)
      {
         int tally = 0;

         if (*s1++)
            tally += convWeights2[0];
         if (*s1)
            tally += convWeights2[1];

         if (*s2++)
            tally += convWeights2[2];
         if (*s2)
            tally += convWeights2[3];

         *t++ = alphaLevel * greys2[tally];
      }
      
      s1 += skip;
      s2 += skip;
   }
}

//-----------------------------------------------------------------------------
// 3 x 3 blur
//-----------------------------------------------------------------------------

void Shadow::blurBitmap3()
{
   if (!greys3)
   {
      int i , sum = 0;
      for (i=0; i < 9; i++)
         sum += convWeights3[i];
         
      greys3 = new UInt8[sum+1];
      // only need to set up once
      for (i=0;i<sum+1;i++)
         greys3[i] = 255.0f * ((float)i)/(float)sum;
   }
   
   int inset = (BufferBmpDim - bmpDim) >> 1;

   UInt8 * s1 = BufferBmp->pBits + (inset-1) * BufferBmpDim + inset - 1;
   UInt8 * s2 = BufferBmp->pBits +     inset * BufferBmpDim + inset - 1;
   UInt8 * s3 = BufferBmp->pBits + (inset+1) * BufferBmpDim + inset - 1;

   int skip = BufferBmpDim - bmpDim;

   UInt8 * t = shadowBmp->pBits;
   UInt8 * e = shadowBmp->pBits + bmpDim * bmpDim;   

   for (; t!=e;)
   {
      UInt8 * lineEnd = s2 + bmpDim;
      for (; s2!= lineEnd;)
      {
         int tally = 0;

         UInt8 * s = s1++;

         if (*s++)
            tally += convWeights3[0];
         if (*s++)
            tally += convWeights3[1];
         if (*s++)
            tally += convWeights3[2];

         s = s2++;

         if (*s++)
            tally += convWeights3[3];
         if (*s++)
            tally += convWeights3[4];
         if (*s++)
            tally += convWeights3[5];

         s = s3++;

         if (*s++)
            tally += convWeights3[6];
         if (*s++)
            tally += convWeights3[7];
         if (*s++)
            tally += convWeights3[8];

         *t++ = alphaLevel * greys3[tally];
      }
      
      s1 += skip;
      s2 += skip;
      s3 += skip;
   }
}

//-----------------------------------------------------------------------------
// 3 x 3 blur with oversampling -- 3 x 3 refers to destination
//-----------------------------------------------------------------------------

void Shadow::blurBitmap23()
{
   if (!greys23)
   {
      int i , sum = 0;
      for (i=0; i < 9; i++)
         sum += convWeights3[i];
      sum *= 4;
         
      greys23 = new UInt8[sum+1];
      // only need to set up once
      for (i=0;i<sum+1;i++)
         greys23[i] = 255.0f * ((float)i)/(float)sum;
   }
   
   int inset = (BufferBmpDim - bmpDim - bmpDim) >> 1;

   UInt8 * s1  = BufferBmp->pBits + (inset-2) * BufferBmpDim + inset - 2;
   UInt8 * s12 = BufferBmp->pBits + (inset-1) * BufferBmpDim + inset - 2;
   UInt8 * s2  = BufferBmp->pBits +     inset * BufferBmpDim + inset - 2;
   UInt8 * s22 = BufferBmp->pBits + (inset+1) * BufferBmpDim + inset - 2;
   UInt8 * s3  = BufferBmp->pBits + (inset+2) * BufferBmpDim + inset - 2;
   UInt8 * s32 = BufferBmp->pBits + (inset+3) * BufferBmpDim + inset - 2;

   int skip = (BufferBmpDim - bmpDim - bmpDim) + BufferBmpDim;

   UInt8 * t = shadowBmp->pBits;
   UInt8 * e = shadowBmp->pBits + bmpDim * bmpDim;   

   for (; t!=e;)
   {
      UInt8 * lineEnd = t + bmpDim;
      for (; t!= lineEnd;)
      {
         int tally = 0;
         UInt8 * s;

         if (*s1++)
            tally += convWeights3[0];
         s=s1++;
         if (*s++)
            tally += convWeights3[0];
         if (*s++)
            tally += convWeights3[1];
         if (*s++)
            tally += convWeights3[1];
         if (*s++)
            tally += convWeights3[2];
         if (*s)
            tally += convWeights3[2];

         if (*s12++)
            tally += convWeights3[0];
         s=s12++;
         if (*s++)
            tally += convWeights3[0];
         if (*s++)
            tally += convWeights3[1];
         if (*s++)
            tally += convWeights3[1];
         if (*s++)
            tally += convWeights3[2];
         if (*s)
            tally += convWeights3[2];

         if (*s2++)
            tally += convWeights3[3];
         s=s2++;
         if (*s++)
            tally += convWeights3[3];
         if (*s++)
            tally += convWeights3[4];
         if (*s++)
            tally += convWeights3[4];
         if (*s++)
            tally += convWeights3[5];
         if (*s)
            tally += convWeights3[5];

         if (*s22++)
            tally += convWeights3[3];
         s=s22++;
         if (*s++)
            tally += convWeights3[3];
         if (*s++)
            tally += convWeights3[4];
         if (*s++)
            tally += convWeights3[4];
         if (*s++)
            tally += convWeights3[5];
         if (*s)
            tally += convWeights3[5];

         if (*s3++)
            tally += convWeights3[6];
         s=s3++;
         if (*s++)
            tally += convWeights3[6];
         if (*s++)
            tally += convWeights3[7];
         if (*s++)
            tally += convWeights3[7];
         if (*s++)
            tally += convWeights3[8];
         if (*s)
            tally += convWeights3[8];

         if (*s32++)
            tally += convWeights3[6];
         s=s32++;
         if (*s++)
            tally += convWeights3[6];
         if (*s++)
            tally += convWeights3[7];
         if (*s++)
            tally += convWeights3[7];
         if (*s++)
            tally += convWeights3[8];
         if (*s)
            tally += convWeights3[8];

         *t++ = alphaLevel * greys23[tally];
      }
      
      s1  += skip;
      s12 += skip;
      s2  += skip;
      s22 += skip;
      s3  += skip;
      s32 += skip;
   }
}

//-----------------------------------------------------------------------------
// 5 x 5 blur
//-----------------------------------------------------------------------------

void Shadow::blurBitmap5()
{
   if (!greys5)
   {
      int i , sum = 0;
      for (i=0; i < 25; i++)
         sum += convWeights5[i];
         
      greys5 = new UInt8[sum+1];
      // only need to set up once
      for (i=0;i<sum+1;i++)
         greys5[i] = ((float)i)/(float)sum;
   }
   
   int inset = (BufferBmpDim - bmpDim) >> 1;

   UInt8 * s1 = BufferBmp->pBits + (inset-2) * BufferBmpDim + inset - 2;
   UInt8 * s2 = BufferBmp->pBits + (inset-1) * BufferBmpDim + inset - 2;
   UInt8 * s3 = BufferBmp->pBits +     inset * BufferBmpDim + inset - 2;
   UInt8 * s4 = BufferBmp->pBits + (inset+1) * BufferBmpDim + inset - 2;
   UInt8 * s5 = BufferBmp->pBits + (inset+2) * BufferBmpDim + inset - 2;

   int skip = BufferBmpDim - bmpDim;

   UInt8 * t = shadowBmp->pBits;
   UInt8 * e = shadowBmp->pBits + bmpDim * bmpDim;   

   for (; t!=e;)
   {
      UInt8 * lineEnd = s3 + bmpDim;
      for (; s3!= lineEnd;)
      {
         int tally = 0;

         UInt8 * s = s1++;

         if (*s++)
            tally += convWeights5[0];
         if (*s++)
            tally += convWeights5[1];
         if (*s++)
            tally += convWeights5[2];
         if (*s++)
            tally += convWeights5[3];
         if (*s++)
            tally += convWeights5[4];

         s = s2++;

         if (*s++)
            tally += convWeights5[5];
         if (*s++)
            tally += convWeights5[6];
         if (*s++)
            tally += convWeights5[7];
         if (*s++)
            tally += convWeights5[8];
         if (*s++)
            tally += convWeights5[9];

         s = s3++;

         if (*s++)
            tally += convWeights5[10];
         if (*s++)
            tally += convWeights5[11];
         if (*s++)
            tally += convWeights5[12];
         if (*s++)
            tally += convWeights5[13];
         if (*s++)
            tally += convWeights5[14];

         s = s4++;

         if (*s++)
            tally += convWeights5[15];
         if (*s++)
            tally += convWeights5[16];
         if (*s++)
            tally += convWeights5[17];
         if (*s++)
            tally += convWeights5[18];
         if (*s++)
            tally += convWeights5[19];

         s = s5++;

         if (*s++)
            tally += convWeights5[20];
         if (*s++)
            tally += convWeights5[21];
         if (*s++)
            tally += convWeights5[22];
         if (*s++)
            tally += convWeights5[23];
         if (*s++)
            tally += convWeights5[24];

         *t++ = alphaLevel * greys5[tally];
      }
      
      s1 += skip;
      s2 += skip;
      s3 += skip;
      s4 += skip;
      s5 += skip;
   }
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

}; // namespace TS
