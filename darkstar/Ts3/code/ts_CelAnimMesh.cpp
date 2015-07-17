//---------------------------------------------------------------------------

// $Workfile:   ts_mesh.h  $
// $Revision:   2.8  $
// $Version$
// $Date:   15 Sep 1995 10:27:12  $
// $Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
// 

//---------------------------------------------------------------------------

#include <g_surfac.h>

#include "ts_CelAnimMesh.h"
#include "ts_BoundingBox.h"
#include "ts_shapeinst.h"
#include "m_coll.h"
#include "m_collision.h"

#pragma option -Jg
#include <tio.h>


namespace TS
{
   //----------------------------------------------------

void
CelAnimMesh::validateFace(MaterialList const& mats,
                          Face const&         face,
                          int                 matFrameIndex ) const
{
#ifdef DEBUG
   AssertFatal( face.fVIP[0].fVertexIndex < fnVertsPerFrame,
      "TS::CelAnimMesh::ValidateFace: invalid fV[0] index");
   AssertFatal( face.fVIP[1].fVertexIndex < fnVertsPerFrame,
      "TS::CelAnimMesh::ValidateFace: invalid fV[1] index");
   AssertFatal( face.fVIP[2].fVertexIndex < fnVertsPerFrame,
      "TS::CelAnimMesh::ValidateFace: invalid fV[2] index");

   if( (mats[face.fMaterial].fParams.fFlags & Material::MatFlags) ==
      Material::MatTexture )
   {
      AssertFatal( face.fVIP[0].fTextureIndex+(matFrameIndex*fnTextureVertsPerFrame) < fTextureVerts.size(),
         "TS::CelAnimMesh::ValidateFace: invalid fT[0] index");
      AssertFatal( face.fVIP[1].fTextureIndex+(matFrameIndex*fnTextureVertsPerFrame) < fTextureVerts.size(),
         "TS::CelAnimMesh::ValidateFace: invalid fT[1] index");
      AssertFatal( face.fVIP[2].fTextureIndex+(matFrameIndex*fnTextureVertsPerFrame) < fTextureVerts.size(),
         "TS::CelAnimMesh::ValidateFace: invalid fT[2] index");
   }
#endif
}

   //---------------------------------------------------------------------------

int
CelAnimMesh::faceVisibility(PointArray * pa, 
                            Face const&    face,
                            int            first ) const
{
   TransformedVertex *v0 = &pa->getTransformedVertex(first + face.fVIP[0].fVertexIndex);
   TransformedVertex *v1 = &pa->getTransformedVertex(first + face.fVIP[1].fVertexIndex);
   TransformedVertex *v2 = &pa->getTransformedVertex(first + face.fVIP[2].fVertexIndex);
   int orTest = ClipMask & (v0->fStatus | v1->fStatus | v2->fStatus );

   if( !orTest )
   {
      // no vertices are clipped, so just do cross product in screen space:
      if( ((v2->fPoint.x - v1->fPoint.x) * 
           (v0->fPoint.y - v1->fPoint.y) - 
           (v2->fPoint.y - v1->fPoint.y) * 
           (v0->fPoint.x - v1->fPoint.x)) 
         >= 0.0f )
         return ClipAllVis;
      else
         return ClipNoneVis;
   }

   // see if entire face can be trivially rejected:
   int andTest = ClipMask & ( v0->fStatus & v1->fStatus & v2->fStatus );
   if( andTest )
      return ClipNoneVis;
   else
      return orTest;
}

//---------------------------------------------------------------------------

void CelAnimMesh::render(RenderContext&       rc, 
                         const ShapeInstance* shapeInst, 
                         MaterialList const&  mats, 
                         int                  frame,
                         int                  matFrameIndex,
                         int*                 matRemap ) const
{
   if( !fFaces.size() )
      return;

   AssertFatal( fFrames.size() > 0, "Shape must have at least one frame." );
   AssertFatal( frame >= 0 && frame < fFrames.size(),
                "TS::CelAnimMesh: frame index out of range" );
                
   // get the frame struct:
   const Frame *frm = &fFrames[frame];
   int FirstVert = frm->fFirstVert;
   const Point3F *pScale = &frm->fScale;
   const Point3F *pOrigin = &frm->fOrigin;
   
   // unpack the min and max points for this frame:
   Vertex minVert, maxVert;
   minVert.set( fVerts[FirstVert], *pScale, *pOrigin );
   maxVert.set( fVerts[FirstVert + 1], *pScale, *pOrigin );
   
   // create bounding object and do vis test:
   BoundingBox bb;
   bb.setBox( minVert.fPoint,maxVert.fPoint );
   int vis = bb.pushVisibility( rc ); // no longer need sphere for visibility check

   if( vis != ClipNoneVis )
   {
      PointArray *pa = rc.getPointArray();
      GFXSurface *srf = rc.getSurface();
      
      // reset point array:
      pa->reset();

      // optimize lights and tell point array to use them:
      bb.setSphere(); // need sphere for lighting
      bb.prepareLights( rc );

      // transform and project all points:
      int first = pa->addPoints( fnVertsPerFrame, 
         &fVerts[FirstVert], *pScale, *pOrigin, &vis );

      if( vis != ClipNoneVis )
      {
         AssertFatal(matFrameIndex*fnTextureVertsPerFrame<fTextureVerts.size(),
            "TS::CelAnimMesh::render:  texture verts index out of range");
         pa->useTextures( &fTextureVerts[matFrameIndex*fnTextureVertsPerFrame] );

         // set up some statics used by renderers
         TSMaterial::prepareRender(fVerts.address(),first,*pScale,*pOrigin,srf,pa,shapeInst);

         // render visible faces (lighting vertices as we go):
         for( Vector<Face>::const_iterator pFace = fFaces.begin(); 
            pFace != fFaces.end(); pFace++ )
         {
            #ifdef DEBUG
            validateFace( mats, *pFace, matFrameIndex );
            #endif
            int faceVis = faceVisibility( pa, *pFace, first );
            
            if( faceVis != ClipNoneVis )
            {
               // draw the face:

               pa->setVisibility( faceVis );

               if( faceVis != ClipAllVis )
                  pa->useBackFaceTest( TRUE );

               if ( pFace->fMaterial != -1 )
               {
                  int mat_id = matRemap?  matRemap[ pFace->fMaterial ] : 
                                          pFace->fMaterial;
                  const Material& fmat = ShapeInstance::getAlwaysMat() ? 
                     (*ShapeInstance::getAlwaysMat())[0] : mats[ mat_id ];

                  fmat.render((const char*)pFace);
               }

               if( faceVis != ClipAllVis )
                  pa->useBackFaceTest( FALSE );
            }
         }
      }
   }
   bb.popVisibility( rc );
}

//-------------------------------------------------------------------

bool CelAnimMesh::collideLineBox( int frameIndex, 
                                  const Point3F & a, const Point3F & b, 
                                  float & hitTime ) const
{
   AssertFatal( fFrames.size() > 0, "Shape must have at least one frame." );
   AssertFatal( frameIndex >= 0 && frameIndex < fFrames.size(),
                "TS::CelAnimMesh: frame index out of range" );
                
   // get the frame struct:
   const Frame *frm = &fFrames[frameIndex];
   int fv = frm->fFirstVert;
   const Point3F *pScale = &frm->fScale;
   const Point3F *pOrigin = &frm->fOrigin;

   // first two verts aren't really verts -- they hold min and max points
   Point3F min,max;
   fVerts[fv].getPoint(min,*pScale,*pOrigin);
   fVerts[fv+1].getPoint(max,*pScale,*pOrigin);

   return m_lineAABox(a,b,min,max,hitTime);
}

bool CelAnimMesh::collideLine( int frameIndex, const Point3F & a, const Point3F & b,
                               CollisionSurface & cs, float minTime ) const
{
   float hitTime = cs.time;
   int hitFace = -1; // if -1 on exit, then we didn't hit a face

   AssertFatal( fFrames.size() > 0, "Shape must have at least one frame." );
   AssertFatal( frameIndex >= 0 && frameIndex < fFrames.size(),
                "TS::CelAnimMesh: frame index out of range" );
                
   // get the frame struct:
   const Frame *frm = &fFrames[frameIndex];
   int fv = frm->fFirstVert;
   const Point3F *pScale = &frm->fScale;
   const Point3F *pOrigin = &frm->fOrigin;

   Point3F vect;
   vect.x = b.x - a.x;
   vect.y = b.y - a.y;
   vect.z = b.z - a.z;

   // set up range of possible hitTimes (w/ hitTime holding the current smallest hit)
   minTime *= 0.9f; // * 0.9f to account for rounding error

   int i;
   Point3F v1,v2,v3;
   for (i=0;i<fFaces.size();i++)
   {
      const Face & theFace = fFaces[i];
      fVerts[theFace.fVIP[0].fVertexIndex+fv].getPoint(v1,*pScale,*pOrigin);
      fVerts[theFace.fVIP[1].fVertexIndex+fv].getPoint(v2,*pScale,*pOrigin);
      fVerts[theFace.fVIP[2].fVertexIndex+fv].getPoint(v3,*pScale,*pOrigin);

      // build the normal
      Point3F normal;
      m_normal(v1,v2,v3,normal);

      // get distance from origin or plane
      float d = m_dot(normal,v3); // distance of plane from origin
      float denom = m_dot(normal,vect);

//    if (denom==0.0f) // if we want to check all faces
      if (denom>=0.0f) // check front facing faces only
         continue;

      float absT = d - m_dot(normal,a);
      if (absT>=minTime*denom || absT<hitTime*denom) // denom negative
         continue;
      float t=absT/denom;

      // intersection = a + hitTime*(b-a)
      Point3F intersection;
      intersection.x = a.x + t*vect.x;
      intersection.y = a.y + t*vect.y;
      intersection.z = a.z + t*vect.z;

      if (m_pointInTriangle(intersection,normal,v1,v2,v3))
      {
         hitTime=t;
         hitFace=i;
         cs.normal=normal;
      }
   }

   if (hitFace>=0)
   {
      cs.time=hitTime;
      cs.surface=hitFace;
      cs.material=fFaces[hitFace].fMaterial;
      // cs.distance?
      // normal already set
      return true;
   }
   return false;
}

//-------------------------------------------------------------------

bool CelAnimMesh::collidePlaneBox( int frameIndex, 
                                   const Point3F & normal, float k) const
{
   AssertFatal( fFrames.size() > 0, "Shape must have at least one frame." );
   AssertFatal( frameIndex >= 0 && frameIndex < fFrames.size(),
                "TS::CelAnimMesh: frame index out of range" );
                
   // get the frame struct:
   const Frame *frm = &fFrames[frameIndex];
   int fv = frm->fFirstVert;
   const Point3F *pScale = &frm->fScale;
   const Point3F *pOrigin = &frm->fOrigin;

   // first two verts aren't really verts -- they hold min and max points
   Point3F min,max;
   fVerts[fv].getPoint(min,*pScale,*pOrigin);
   fVerts[fv+1].getPoint(max,*pScale,*pOrigin);

   Point3F center = min + max; center *= 0.5f;
   Point3F radii  = max - min; radii  *= 0.5f;

   float centerDist = m_dot(normal,center) - k;

   // if center on other side of plane, we hit...
   if (centerDist<0.0f)
      return true;

   // return false if sphere doesn't intersect plane
   if (centerDist*centerDist>m_dot(radii,radii))
      return false;

   // now check box against plane -- find radius of box when projected onto normal
   float boxRadius = fabs(normal.x * radii.x) +
                     fabs(normal.y * radii.y) +
                     fabs(normal.z * radii.z);
   return centerDist<boxRadius;
}

bool CelAnimMesh::collidePlane( int frameIndex, const Point3F & normal, float k,
                                CollisionSurface & cs ) const
{
   AssertFatal( fFrames.size() > 0, "Shape must have at least one frame." );
   AssertFatal( frameIndex >= 0 && frameIndex < fFrames.size(),
                "TS::CelAnimMesh: frame index out of range" );
                
   // get the frame struct:
   const Frame *frm = &fFrames[frameIndex];
   int fv = frm->fFirstVert;
   const Point3F *pScale = &frm->fScale;
   const Point3F *pOrigin = &frm->fOrigin;

   int i;
   int minIndex = -1;
   cs.distance = 0.0f;
   Point3F vert;
   for (i=2; i< fnVertsPerFrame; i++)
   {
      fVerts[fv+i].getPoint(vert,*pScale,*pOrigin);
      float dist = m_dot(normal,vert) - k;
      if (dist<cs.distance)
      {
         cs.position=vert;
         cs.distance=dist;
         minIndex = i;
      }
   }
   if (minIndex>=0)
   {
      cs.normal = fVerts[fv+minIndex].decodeNormal();
      cs.distance *= -1.0f;
      cs.material=0;
      return true;
   }
   else
      return false;
}

//-------------------------------------------------------------------

bool CelAnimMesh::collideTubeBox( int frameIndex, 
                                  const Point3F & a, const Point3F & b, float rad, 
                                  float & hitTime ) const
{
   AssertFatal( fFrames.size() > 0, "Shape must have at least one frame." );
   AssertFatal( frameIndex >= 0 && frameIndex < fFrames.size(),
                "TS::CelAnimMesh: frame index out of range" );
                
   // get the frame struct:
   const Frame *frm = &fFrames[frameIndex];
   int fv = frm->fFirstVert;
   const Point3F *pScale = &frm->fScale;
   const Point3F *pOrigin = &frm->fOrigin;
   
   // first two verts aren't really verts -- they hold min and max points
   Point3F min,max;
   fVerts[fv].getPoint(min,*pScale,*pOrigin);
   fVerts[fv+1].getPoint(max,*pScale,*pOrigin);

   return m_tubeAABox(a,b,rad,min,max,hitTime);
}

// R1 and R2 hold the projection of edge endpoints onto cross-section of tube centered on origin
// t1 and t2 hold projections of edge onto axis of tube (tube goes from 0 to tubeLen)
// we assume t1<t2
// on return hit point is offset from center line but not projected along the line
bool edgeInTube(Point3F & R1,Point3F & R2,float t1,float t2, float radius, float invRadius2, float invTubeLen,
                float & hitTime,Point3F & hitPoint)
{
   // range of edge that could still be in tube and close enough: (0,1) is whole edge
   float rangeA=0.0f;
   float rangeB=1.0f;
   // shrink down range based on time:
   if (t1<0.0f)
   {
      if (t2<0.0f)
         return false;
      rangeA = -t1/(t2-t1);
   }
   if (invTubeLen * t2 > hitTime)
   {
      if (invTubeLen * t1 > hitTime)
         return false;
      rangeB = (hitTime - t1 * invTubeLen) / (invTubeLen * (t2-t1));
   }

   // now shrink down range based on distance of projection from origin
   Point3F R12 = R2;
   R12 -= R1;
   float edgeLen2= m_dot(R12,R12);

   if (edgeLen2==0.0f)
   {
      // R1=R2.  Are they in the circle?
      if (R1.x*R1.x + R1.y*R1.y + R1.z*R1.z >= radius*radius)
         return false;
      // o.w., don't restrict range based on projection
   }
   else
   {
      // signed dist from R1 of point on edge line (not edge) closest to origin
      // actually R12 times above dist. (optimization)
      float edgePointDist = - m_dot(R1,R12);

      // dist squared of projected edge line (not edge) from origin
      float edgeDistSquared = edgeLen2 * m_dot(R1,R1) - edgePointDist*edgePointDist;
      if (edgeDistSquared>=edgeLen2 * radius * radius)
         return false;

      // put off adjusting edgePointDist and edgeDistSquared till now for speed...
      float invEdgeLen = 1.0f/m_sqrt(edgeLen2);
      edgePointDist *= invEdgeLen;
      edgeDistSquared *= invEdgeLen;
      edgeDistSquared *= invEdgeLen;

      // projected edge intersects circle iff (0,edgeLen) intersects
      // edgePointDist +/- sqrt(rad^2-edgeDist^2)
      // use an approximation to sqrt:
      float x = 1.0f-invRadius2*edgeDistSquared;
      float bound = -.699 * radius * (x+.07978539f)*(x-2.277210289f);

      float loRange = invEdgeLen * (edgePointDist-bound);
      float hiRange = invEdgeLen * (edgePointDist+bound);
      if (hiRange<rangeA || loRange>rangeB)
         return false;
      if (loRange > rangeA)
         rangeA=loRange;
   }
   hitTime = (t1 + rangeA * (t2-t1)) * invTubeLen;
   hitPoint.x = R1.x + (R2.x-R1.x) * rangeA;
   hitPoint.y = R1.y + (R2.y-R1.y) * rangeA;
   hitPoint.z = R1.z + (R2.z-R1.z) * rangeA;

   return true;
}

static Vector<Point3F>     workVerts;
static Vector<Point3F>     workRs;
static Vector<float>    workTs;

bool CelAnimMesh::collideTube( int frameIndex, const Point3F & a, const Point3F &b, float radius,
                                       CollisionSurface & cs, float minTime) const
{
   minTime;

   // hitTime holds the current smallest...
   float hitTime = cs.time;
   int hitFace = -1;
   Point3F hitPoint;

   AssertFatal( fFrames.size() > 0, "Shape must have at least one frame." );
   AssertFatal( frameIndex >= 0 && frameIndex < fFrames.size(),
                "TS::CelAnimMesh: frame index out of range" );
                
   // get the frame struct:
   const Frame *frm = &fFrames[frameIndex];
   int fv = frm->fFirstVert;
   const Point3F *pScale = &frm->fScale;
   const Point3F *pOrigin = &frm->fOrigin;
   
   Point3F tubeVect;
   tubeVect.x = b.x - a.x;
   tubeVect.y = b.y - a.y;
   tubeVect.z = b.z - a.z;
   float tubeLen = tubeVect.len();
   float invTubeLen = 1.0f/tubeLen;

   // tubeVect will hold unit length vector pointing down tube
   tubeVect *= invTubeLen;
   float vectDotA = m_dot(tubeVect,a);

   // inverse radius squared for edgeInTube routine
   float invRadius2 = 1.0f / (radius*radius);

   int i;
   workVerts.setSize(fnVertsPerFrame);
   workRs.setSize(fnVertsPerFrame);
   workTs.setSize(fnVertsPerFrame);
   bool gotNormal;
   for (i=0;i<fFaces.size();i++)
   {
      const Face & theFace = fFaces[i];
      int idx1 = theFace.fVIP[0].fVertexIndex;
      int idx2 = theFace.fVIP[1].fVertexIndex;
      int idx3 = theFace.fVIP[2].fVertexIndex;

      Point3F &v1=workVerts[idx1];
      Point3F &v2=workVerts[idx2];
      Point3F &v3=workVerts[idx3];

      Point3F &R1=workRs[idx1];
      Point3F &R2=workRs[idx2];
      Point3F &R3=workRs[idx3];
      
      float &t1=workTs[idx1];
      float &t2=workTs[idx2];
      float &t3=workTs[idx3];

      if (!(v1Recycled&faceReuseFlags[i]))
      {
         fVerts[idx1+fv].getPoint(v1,*pScale,*pOrigin);
         // distance of vertex down the tube
         t1 = m_dot(v1,tubeVect) - vectDotA;
         // projection of vertex onto tube cross-section (centered on origin)
         R1.x = v1.x - a.x - t1 * tubeVect.x;
         R1.y = v1.y - a.y - t1 * tubeVect.y;
         R1.z = v1.z - a.z - t1 * tubeVect.z;
      }

      if (!(v2Recycled&faceReuseFlags[i]))
      {
         fVerts[idx2+fv].getPoint(v2,*pScale,*pOrigin);
         // distance of vertex down the tube
         t2 = m_dot(v2,tubeVect) - vectDotA;
         // projection of vertex onto tube cross-section (centered on origin)
         R2.x = v2.x - a.x - t2 * tubeVect.x;
         R2.y = v2.y - a.y - t2 * tubeVect.y;
         R2.z = v2.z - a.z - t2 * tubeVect.z;
      }

      if (!(v3Recycled&faceReuseFlags[i]))
      {
         fVerts[idx3+fv].getPoint(v3,*pScale,*pOrigin);
         // distance of vertex down the tube
         t3 = m_dot(v3,tubeVect) - vectDotA;
         // projection of vertex onto tube cross-section (centered on origin)
         R3.x = v3.x - a.x - t3 * tubeVect.x;
         R3.y = v3.y - a.y - t3 * tubeVect.y;
         R3.z = v3.z - a.z - t3 * tubeVect.z;
      }

      bool gotHit=false;
      if (!(e1Recycled&faceReuseFlags[i]))
      {
         if (t1<=t2)
            gotHit = edgeInTube(R1,R2,t1,t2,radius,invRadius2,invTubeLen,hitTime,hitPoint);
         else
            gotHit = edgeInTube(R2,R1,t2,t1,radius,invRadius2,invTubeLen,hitTime,hitPoint);
      }

      if (!(e2Recycled&faceReuseFlags[i]))
      {
         if (t2<=t3)                                          
            gotHit |= edgeInTube(R2,R3,t2,t3,radius,invRadius2,invTubeLen,hitTime,hitPoint);
         else                                                 
            gotHit |= edgeInTube(R3,R2,t3,t2,radius,invRadius2,invTubeLen,hitTime,hitPoint);
      }

      if (!(e3Recycled&faceReuseFlags[i]))
      {
         if (t3<=t1)
            gotHit |= edgeInTube(R3,R1,t3,t1,radius,invRadius2,invTubeLen,hitTime,hitPoint);
         else
            gotHit |= edgeInTube(R1,R3,t1,t3,radius,invRadius2,invTubeLen,hitTime,hitPoint);
      }

      if (gotHit)
      {
         hitPoint.x += tubeLen * hitTime * tubeVect.x + a.x;
         hitPoint.y += tubeLen * hitTime * tubeVect.y + a.y;
         hitPoint.z += tubeLen * hitTime * tubeVect.z + a.z;
         hitFace=i;
         gotNormal=false;
      }

      // now check if tube goes through center of face w/o hitting any edges
      if (m_pointInTriangle(Point3F(0.0f,0.0f,0.0f),tubeVect,R1,R2,R3))
      {
         // build the normal
         Point3F normal;
         m_normal(v1,v2,v3,normal);
         // now we need to find hitTime
         float d = m_dot(normal,v3); // distance of plane from origin
         float denom = m_dot(normal,tubeVect) * tubeLen;
         if (denom>=0.0f) // back face, we can ignore
            continue;
         float absT = d - m_dot(normal,a);
         if (absT<=hitTime*denom) // denom is neg.
            continue;

         // ok, a real collision, set ci variables...
         hitTime=absT/denom;
         hitFace=i;
         cs.normal=normal;
         gotNormal=true;
         hitPoint.x = a.x + hitTime * tubeLen * tubeVect.x;
         hitPoint.y = a.y + hitTime * tubeLen * tubeVect.y;
         hitPoint.z = a.z + hitTime * tubeLen * tubeVect.z;
      }
   }

   if (hitFace>=0)
   {
      const Face & theFace = fFaces[hitFace];
      if (!gotNormal)
      {
         Point3F &v1=workVerts[theFace.fVIP[0].fVertexIndex];
         Point3F &v2=workVerts[theFace.fVIP[1].fVertexIndex];
         Point3F &v3=workVerts[theFace.fVIP[2].fVertexIndex];

         // build the normal
         Point3F v13,v23;
         v13.x = v1.x-v3.x;
         v13.y = v1.y-v3.y;
         v13.z = v1.z-v3.z;
         v23.x = v2.x-v3.x;
         v23.y = v2.y-v3.y;
         v23.z = v2.z-v3.z;
         m_cross(v23,v13,&cs.normal);
      }
      cs.material=theFace.fMaterial;
      cs.surface=hitFace;
      cs.time=hitTime;
      cs.position=hitPoint;
      // cs.distance ??
      return true;
   }
   return false;
}

//-------------------------------------------------------------------

bool CelAnimMesh::collideSphereBox( int frameIndex, 
                                    const Point3F & center, float radius) const
{
   AssertFatal( fFrames.size() > 0, "Shape must have at least one frame." );
   AssertFatal( frameIndex >= 0 && frameIndex < fFrames.size(),
                "TS::CelAnimMesh: frame index out of range" );
                
   // get the frame struct:
   const Frame *frm = &fFrames[frameIndex];
   int fv = frm->fFirstVert;
   const Point3F *pScale = &frm->fScale;
   const Point3F *pOrigin = &frm->fOrigin;

   Point3F min,max;
   fVerts[fv].getPoint(min,*pScale,*pOrigin);
   fVerts[fv+1].getPoint(max,*pScale,*pOrigin);

   float trash; // this will hold min distance from sphere center to box -- we don't use it
   return m_sphereAABox(center,radius*radius,min,max,trash);
}

bool CelAnimMesh::collideSphere( int frameIndex, 
                                 const Point3F & center, float radius, 
                                 CollisionSurfaceList & csl) const
{
   AssertFatal( fFrames.size() > 0, "Shape must have at least one frame." );
   AssertFatal( frameIndex >= 0 && frameIndex < fFrames.size(),
                "TS::CelAnimMesh: frame index out of range" );
                
   // get the frame struct:
   const Frame *frm = &fFrames[frameIndex];
   int fv = frm->fFirstVert;
   const Point3F *pScale = &frm->fScale;
   const Point3F *pOrigin = &frm->fOrigin;

   bool gotHit=false;

   int i;
   Point3F v[3];
   for (i=0;i<fFaces.size();i++)
   {
      const Face & theFace = fFaces[i];
      fVerts[theFace.fVIP[0].fVertexIndex+fv].getPoint(v[0],*pScale,*pOrigin);
      fVerts[theFace.fVIP[1].fVertexIndex+fv].getPoint(v[1],*pScale,*pOrigin);
      fVerts[theFace.fVIP[2].fVertexIndex+fv].getPoint(v[2],*pScale,*pOrigin);

      // build the normal and distance from origin of face plane
      Point3F normal;
      m_normal(v[0],v[1],v[2],normal);
      float norm=normal.len();
      if (norm==0.0f) // can happen if a face gets scrunched during animation
         continue;
      normal *= 1.0f/norm;

      // distance from origin of plane and parallel plane containing sphere center
      float dSphere = m_dot(normal,center);
      float dPlane = m_dot(normal,v[0]);
      float dist = dSphere-dPlane;
      if (dist<0) // ignore back facing planes
         continue;

      if (dist>=radius) // whole plane outside of sphere
         continue;

      // intersection of face plane and sphere is a circle on face plane
      // with center p (below) and radius circleRad (below)
      Point3F p;
      p.x = center.x - dist * normal.x;
      p.y = center.y - dist * normal.y;
      p.z = center.z - dist * normal.z;
      float circleRad2 = radius*radius - dist*dist; // square of circle radius

      // check to see if p is inside face (in case whole circle inside face)
      if (m_pointInTriangle(p,normal,v[0],v[1],v[2]))
      {
         // add entry to cil
         csl.increment();
         CollisionSurface & info = csl.last();
         info.surface=i;
         info.material = theFace.fMaterial;
         info.position = p;
         info.normal = normal;
         info.distance = dist;
         gotHit=true;
         continue;
      }

      // go through each edge, check to see if it intersects inside of circle
      // if so, add collision info
      for (int j=0;j<3;j++)
      {
         const Point3F & pivotVert = v[j];
         Point3F edge;
         edge.x = v[(j+1) % 3].x - pivotVert.x;
         edge.y = v[(j+1) % 3].y - pivotVert.y;
         edge.z = v[(j+1) % 3].z - pivotVert.z;

         Point3F radialLine;
         radialLine.x = p.x - pivotVert.x;
         radialLine.y = p.y - pivotVert.y;
         radialLine.z = p.z - pivotVert.z;

         float edgeLen=edge.len();
         if (edgeLen==0.0f)
            continue;
         edge *= 1.0f/edgeLen;

         // get length of projection of radial line onto edge line
         float projRadialToEdgeDist = m_dot(radialLine,edge);

         // find point at which perp. to edge from circle center hits edge line
         Point3F intPoint;
         intPoint.x = pivotVert.x + projRadialToEdgeDist * edge.x;
         intPoint.y = pivotVert.y + projRadialToEdgeDist * edge.y;
         intPoint.z = pivotVert.z + projRadialToEdgeDist * edge.z;
         float distToIntersect2 = (p.x-intPoint.x)*(p.x-intPoint.x) +
                            (p.y-intPoint.y)*(p.y-intPoint.y) +
                            (p.z-intPoint.z)*(p.z-intPoint.z);

         // is whole line outside circle
         if (distToIntersect2>circleRad2)
            continue;

         // length of half-cord formed by edge line
//       float halfcord = sqrt(circleRad2 - distToIntersect2);
         float halfcord2 = circleRad2 - distToIntersect2;

         // check to see if line lands inside of circle
//       if ( (projRadialToEdgeDist < 0.0f-halfcord) || 
//          (projRadialToEdgeDist > halfcord+edgeLen) )
//          continue;
         if (projRadialToEdgeDist < 0.0f)
         {
            if (projRadialToEdgeDist*projRadialToEdgeDist>halfcord2)
               continue;
         }
         else if (projRadialToEdgeDist>edgeLen)
         {
            float tmpF = projRadialToEdgeDist-edgeLen;
            if (tmpF*tmpF>halfcord2)
               continue;
         }

         // we know we hit now...
         // where exactly did we hit, shift intersect point onto edge to form hitPoint
         if (projRadialToEdgeDist < 0.0f)
            projRadialToEdgeDist = 0.0f;
         else if (projRadialToEdgeDist > edgeLen)
            projRadialToEdgeDist = edgeLen;
         Point3F hitPoint;
         hitPoint.x = pivotVert.x + projRadialToEdgeDist * edge.x;
         hitPoint.y = pivotVert.y + projRadialToEdgeDist * edge.y;
         hitPoint.z = pivotVert.z + projRadialToEdgeDist * edge.z;

         // add entry to cil
         csl.increment();
         CollisionSurface & info = csl.last();
         info.surface=i;
         info.material = theFace.fMaterial;
         info.position = hitPoint;
         info.normal = normal;
         info.distance = dist;
         gotHit=true;
         break;
      }
   }
   return gotHit;
}

//-------------------------------------------------------------------

bool 
CelAnimMesh::collideShapeBox(int frameIndex,
                      TMat3F & objToOtherShape,
                      objectList &otherOL,
                      objectList &thisOL) const
{
   AssertFatal( fFrames.size() > 0, "Shape must have at least one frame." );
   AssertFatal( frameIndex >= 0 && frameIndex < fFrames.size(),
                "TS::CelAnimMesh: frame index out of range" );
                
   // get the frame struct:
   const Frame *frm = &fFrames[frameIndex];
   int fv = frm->fFirstVert;
   const Point3F *pScale = &frm->fScale;
   const Point3F *pOrigin = &frm->fOrigin;

   // first two verts aren't really verts -- they hold min and max points
   Point3F radii,min;
   fVerts[fv+1].getPoint(radii,*pScale,*pOrigin);
   fVerts[fv].getPoint(min,*pScale,*pOrigin);
   radii -= min;
   radii *= 0.5f;

   // shift transform so box center at origin
   Point3F tmpPoint = min;
   tmpPoint += radii;
   objToOtherShape.preTranslate(tmpPoint);

   bool gotOne = false;
   thisOL.increment();
   ObjectInfo * poi = &thisOL.last();
   for (int i=0;i<otherOL.size();i++)
   {
      ObjectInfo & otherOI = otherOL[i];
      TMat3F boxTobox;
      m_mul(objToOtherShape,otherOI.invObjTrans,&boxTobox);
      if ( otherOI.pObj->collideBoxObj( boxTobox, radii, poi->aOverlap,poi->bOverlap ))
      {
         // fill in pointer to other object
         poi->pObj2=otherOI.pObj;
         // create another entry on the object list (to be deleted if no more collisions)
         thisOL.increment();
         poi= &thisOL.last();
         gotOne = true;
      }
   }
   thisOL.decrement();
   return gotOne;
}

bool 
CelAnimMesh::collideBoxBox(int frameIndex,
                     TMat3F &trans,
                     const Point3F & bRadii,
                     Point3F & aOverlap,
                     Point3F & bOverlap ) const
{
   AssertFatal( fFrames.size() > 0, "Shape must have at least one frame." );
   AssertFatal( frameIndex >= 0 && frameIndex < fFrames.size(),
                "TS::CelAnimMesh: frame index out of range" );
                
   // get the frame struct:
   const Frame *frm = &fFrames[frameIndex];
   int fv = frm->fFirstVert;
   const Point3F *pScale = &frm->fScale;
   const Point3F *pOrigin = &frm->fOrigin;

   // first two verts aren't really verts -- they hold min and max points
   Point3F aRadii,min;
   fVerts[fv+1].getPoint(aRadii,*pScale,*pOrigin);
   fVerts[fv].getPoint(min,*pScale,*pOrigin);
   aRadii -= min;
   aRadii *= 0.5f;
   
   // shift so transform takes center of one box to center of the other
   // rather than pivot point to pivot point
   trans.p -= min;
   trans.p -= aRadii;
   trans.flags |= TMat3F::Matrix_HasTranslation;

   return m_OBoxOBox(aRadii,bRadii,trans,aOverlap,bOverlap);
}

// warning: a work in progress...not tested yet
bool 
CelAnimMesh::collideBox(int frameIndex,
                        const TMat3F & trans,       // from box to mesh space (box center to origin)
                        const TMat3F & invTrans,    // from mesh to box space
                        const Point3F &radii,
                        CollisionSurface & cs) const
{
   cs;

   int hitface = -1;
   float hitDepth;
   Point3F hitNormal;
   float overlap;

   AssertFatal( fFrames.size() > 0, "Shape must have at least one frame." );
   AssertFatal( frameIndex >= 0 && frameIndex < fFrames.size(),
                "TS::CelAnimMesh: frame index out of range" );
                
   // get the frame struct:
   const Frame *frm = &fFrames[frameIndex];
   int fv = frm->fFirstVert;
   const Point3F *pScale = &frm->fScale;
   const Point3F *pOrigin = &frm->fOrigin;

   int i;
   Point3F v[3],tv[3]; // tv is work space for m_polyOBox...
   for (i=0;i<fFaces.size();i++)
   {
      const Face & theFace = fFaces[i];
      fVerts[theFace.fVIP[0].fVertexIndex+fv].getPoint(v[0],*pScale,*pOrigin);
      fVerts[theFace.fVIP[1].fVertexIndex+fv].getPoint(v[1],*pScale,*pOrigin);
      fVerts[theFace.fVIP[2].fVertexIndex+fv].getPoint(v[2],*pScale,*pOrigin);

      // build the normal
      Point3F normal;
      m_normal(v[0],v[1],v[2],normal);

      if (!m_polyOBox(radii,trans,invTrans,normal,v,tv,3,overlap))
         continue;

/*
      // build the normal
      Point3F normal;
      m_normal(v1,v2,v3,normal);

      float planeDist = m_dot(normal,v1);
      float negBoxDist = m_dot(normal,trans.p); // negative of dist of box from origin
      if (planeDist+negBoxDist<0.0f) // back-face from box center
         continue;

      // does the face's plane intersect the box
      float overlap;
      if (!m_planeOBox(bRadii,trans,normal,planeDist,overlap))
         continue;

      Point3F tv1,tv2,tv3,tMin,tMax;
      m_mul(v1,invTrans,&tv1);
      m_mul(v2,invTrans,&tv2);
      m_mul(v3,invTrans,&tv3);
      tMin=tv1;
      tMin.setMin(tv2);
      tMin.setMin(tv3);
      tMax=tv1;
      tMax.setMax(tv2);
      tMax.setMax(tv3);

      if (tMin.x>bRadii.x)
         continue;
      if (tMax.x<-bRadii.x)
         continue;
      if (tMin.y>bRadii.y)
         continue;
      if (tMax.y<-bRadii.y)
         continue;
      if (tMin.z>bRadii.z)
         continue;
      if (tMax.z<-bRadii.z)
         continue;
*/

      {
         // collision
         hitface = i;
         hitNormal = normal;
         hitDepth = overlap/normal.len();
      }
   }
   hitNormal,hitDepth;
   return hitface!=-1;
}

//-------------------------------------------------------------------

void CelAnimMesh::getBox( int frameIndex, Box3F & box) const
{
   if( !fVerts.size() )
      return;

   // get the frame struct:
   AssertFatal( frameIndex >= 0 && frameIndex < fFrames.size(),
                "TS::CelAnimMesh: frame index out of range" );

   // get the frame struct:
   const Frame *frm = &fFrames[frameIndex];
   int fv = frm->fFirstVert;
   const Point3F *pScale = &frm->fScale;
   const Point3F *pOrigin = &frm->fOrigin;

   // unpack the min and max points for this frame:
   Vertex minVert, maxVert;
   minVert.set( fVerts[fv], *pScale, *pOrigin );
   maxVert.set( fVerts[fv + 1], *pScale, *pOrigin );

   box.fMin = minVert.fPoint;
   box.fMax = maxVert.fPoint;
}

void CelAnimMesh::getPolys(int frameIndex, int matFrameIndex,
                           const TMat3F * meshToShape,
                           unpackedFaceList & fl) const
{
   AssertFatal( fFrames.size() > 0, "Shape must have at least one frame." );
   AssertFatal( frameIndex >= 0 && frameIndex < fFrames.size(),
                "TS::CelAnimMesh: frame index out of range" );

   matFrameIndex;
                
   // get the frame struct:
   const Frame *frm = &fFrames[frameIndex];
   int fv = frm->fFirstVert;
   const Point3F *pScale = &frm->fScale;
   const Point3F *pOrigin = &frm->fOrigin;
   
//   int ftv=matFrameIndex*fnTextureVertsPerFrame;

   int i;
   Vector<Point3F> & toPoints = fl.fPoints;
   int startPoint = toPoints.size();
   for (i=0;i<fnVertsPerFrame;i++)
   {
      toPoints.increment();

      Point3F pnt;
      fVerts[i+fv].getPoint(pnt,*pScale,*pOrigin);

      if (meshToShape)
         m_mul(pnt,*meshToShape,&toPoints.last());
      else
         toPoints.last() = pnt;
   }

//   Vector<Point2F> & toTextures = fl.fTextures;
//   int startTexture = toTextures.size();
//   for (i=0;i<fnTextureVertsPerFrame;i++)
//   {
//      toTextures.increment();
//      toTextures.last()=fTextureVerts[i+ftv];
//   }

   for (i=0;i<fFaces.size();i++)
   {
      const Face & fromFace = fFaces[i];
      fl.fFaces.increment();
      TS::Face & toFace = fl.fFaces.last();

      toFace.fVIP[0].fVertexIndex=fromFace.fVIP[0].fVertexIndex+startPoint;
//    toFace.fVIP[0].fTextureIndex=fromFace.fVIP[0].fTextureIndex+startTexture;

      toFace.fVIP[1].fVertexIndex=fromFace.fVIP[1].fVertexIndex+startPoint;
//    toFace.fVIP[1].fTextureIndex=fromFace.fVIP[1].fTextureIndex+startTexture;

      toFace.fVIP[2].fVertexIndex=fromFace.fVIP[2].fVertexIndex+startPoint;
//       toFace.fVIP[2].fTextureIndex=fromFace.fVIP[2].fTextureIndex+startTexture;

//    toFace.material=fromFace.fMaterial;
   }
}

int CelAnimMesh::importFrame( const Shape::Mesh & fromMesh, int fromFrameIndex)
{
   const CelAnimMesh * pfromCelAnimMesh = static_cast<const CelAnimMesh *>(&fromMesh);
   AssertFatal(pfromCelAnimMesh,
      "TS::CelAnimMesh::importFrame:  attempt to import a non CelAnimMesh frame into a CelAnimMesh");
   const Frame &frm = pfromCelAnimMesh->fFrames[fromFrameIndex];

   // add new frame
   Frame newFrame;
   newFrame.fFirstVert=fVerts.size();
   newFrame.fScale = frm.fScale;
   newFrame.fOrigin = frm.fOrigin;
   fFrames.push_back(newFrame);

   // copy new verts
   int firstVert = frm.fFirstVert;
   for (int i=0;i<fnVertsPerFrame;i++)
      fVerts.push_back(pfromCelAnimMesh->fVerts[firstVert+i]);
   return fFrames.size()-1;
}

void CelAnimMesh::setCommonScale( Shape::Mesh & otherMesh )
{
   CelAnimMesh *potherMesh = dynamic_cast<CelAnimMesh *>(&otherMesh);
   AssertFatal(potherMesh,
      "TS::CelAnimMesh::setCommonScale:  meshes not same type");

#if 0 
   // array of unpacked verts -- points only
   Point3F *unpackedVerts = new Point3F[fVerts.size()];
   int v;
   for (v=0;v<fVerts.size();v++)
      fVerts[v].getPoint(unpackedVerts[v],fScale,fOrigin);

   Point3F *otherUnpackedVerts = new Point3F[potherMesh->fVerts.size()];
   for (v=0;v<potherMesh->fVerts.size();v++)
      potherMesh->fVerts[v].getPoint(otherUnpackedVerts[v],potherMesh->fScale,potherMesh->fOrigin);

   // get minVert and maxVert for setting new fScale, fOrigin, and fRadius
   Point3F minVert = unpackedVerts[0];
   Point3F maxVert = unpackedVerts[0];
   for (v=1;v<fVerts.size();v++)
   {
      minVert.setMin( unpackedVerts[v] );
      maxVert.setMax( unpackedVerts[v] );
   }
   for (v=0;v<potherMesh->fVerts.size();v++)
   {
      minVert.setMin( otherUnpackedVerts[v] );
      maxVert.setMax( otherUnpackedVerts[v] );
   }

   // figure new fOrigin, fScale, and fRadius
   Point3F newOrigin = minVert;
   maxVert -= minVert;
   Point3F newScale( maxVert.x/255.0f, maxVert.y/255.0f, maxVert.z/255.0f);
   float newRadius = maxVert.len();

   // re-pack this shapes verts     
   int i;
   Point3F temp;
   for (i=0;i<fVerts.size();i++)
      fVerts[i].setPoint(unpackedVerts[i],newScale,newOrigin);
   fOrigin=newOrigin;
   fScale=newScale;
   fRadius=newRadius;

   // re-pack other shapes verts
   for (i=0;i<potherMesh->fVerts.size();i++)
      potherMesh->fVerts[i].setPoint(otherUnpackedVerts[i],newScale,newOrigin);
   potherMesh->fOrigin=fOrigin;
   potherMesh->fScale=newScale;
   potherMesh->fRadius=newRadius;

   delete [] unpackedVerts;
   delete [] otherUnpackedVerts;
#endif      
}

//---------------------------------------------------------------------------

IMPLEMENT_PERSISTENT(CelAnimMesh);

int CelAnimMesh::version()
{
   return 3;
}

#pragma argsused

Persistent::Base::Error CelAnimMesh::read( StreamIO & sio, int version, int user )
{
   user;

   Persistent::Base::Error err = Persistent::Base::Ok;

   Int32 nVerts, nTextureVerts, nFaces, nFrames;

   sio.read(&nVerts);
   sio.read(&fnVertsPerFrame);
   sio.read(&nTextureVerts);
   sio.read(&nFaces);
   sio.read(&nFrames);
   if (version>=2)
      sio.read(&fnTextureVertsPerFrame);
   else
      fnTextureVertsPerFrame=nTextureVerts;

   Point3F v2Scale, v2Origin;
   
   if ( version < 3 )
   {
      v2Scale.read( sio );
      v2Origin.read( sio );
   }
   
   sio.read( &fRadius );

   Vector<v2Frame> v2Frames;

   #ifdef TOOL_VERSION
   readVector( sio, nVerts,         &fVerts );
   readVector( sio, nTextureVerts,  &fTextureVerts );
   readVector( sio, nFaces,         &fFaces );
   
   if ( version < 3 )
      readVector( sio, nFrames,        &v2Frames );
   else
      readVector( sio, nFrames,        &fFrames );
   #else
   lockVector( sio, nVerts,         &fVerts );
   lockVector( sio, nTextureVerts,  &fTextureVerts );
   lockVector( sio, nFaces,         &fFaces );
   
   if ( version < 3 )
      lockVector( sio, nFrames,        &v2Frames );
   else
      lockVector( sio, nFrames,        &fFrames );
   #endif

   if ( version < 3 )
   {
      if ( !nFrames )
      {
         Frame f;
         f.fFirstVert = 0;
         f.fOrigin = v2Origin;
         f.fScale = v2Scale;
         fFrames.push_back( f );
         nFrames = 1;
      }
      else
      {     
         fFrames.setSize( nFrames );
         for ( int i = 0; i < nFrames; i++ )
         {
            fFrames[i].fFirstVert = v2Frames[i].fFirstVert;
            fFrames[i].fOrigin = v2Origin;
            fFrames[i].fScale = v2Scale;
         }
      }
   }

   // set up re-use flags
   faceReuseFlags.setSize(fFaces.size());
   for (int i=0;i<fFaces.size();i++)
   {
      Int8 f=0;
      for (int j=0;j<i;j++)
      {
         if (fFaces[i].fVIP[0].fVertexIndex==fFaces[j].fVIP[0].fVertexIndex ||
             fFaces[i].fVIP[0].fVertexIndex==fFaces[j].fVIP[1].fVertexIndex ||
             fFaces[i].fVIP[0].fVertexIndex==fFaces[j].fVIP[2].fVertexIndex)
         {
            f |= v1Recycled;
            if (fFaces[i].fVIP[1].fVertexIndex==fFaces[j].fVIP[0].fVertexIndex ||
                fFaces[i].fVIP[1].fVertexIndex==fFaces[j].fVIP[1].fVertexIndex ||
                fFaces[i].fVIP[1].fVertexIndex==fFaces[j].fVIP[2].fVertexIndex)
              f |= e1Recycled;
         }
         if (fFaces[i].fVIP[1].fVertexIndex==fFaces[j].fVIP[0].fVertexIndex ||
             fFaces[i].fVIP[1].fVertexIndex==fFaces[j].fVIP[1].fVertexIndex ||
             fFaces[i].fVIP[1].fVertexIndex==fFaces[j].fVIP[2].fVertexIndex)
         {
            f |= v2Recycled;
            if (fFaces[i].fVIP[2].fVertexIndex==fFaces[j].fVIP[0].fVertexIndex ||
                fFaces[i].fVIP[2].fVertexIndex==fFaces[j].fVIP[1].fVertexIndex ||
                fFaces[i].fVIP[2].fVertexIndex==fFaces[j].fVIP[2].fVertexIndex)
              f |= e2Recycled;
         }
         if (fFaces[i].fVIP[2].fVertexIndex==fFaces[j].fVIP[0].fVertexIndex ||
             fFaces[i].fVIP[2].fVertexIndex==fFaces[j].fVIP[1].fVertexIndex ||
             fFaces[i].fVIP[2].fVertexIndex==fFaces[j].fVIP[2].fVertexIndex)
         {
           f |= v3Recycled;
           if (fFaces[i].fVIP[0].fVertexIndex==fFaces[j].fVIP[0].fVertexIndex ||
               fFaces[i].fVIP[0].fVertexIndex==fFaces[j].fVIP[1].fVertexIndex ||
               fFaces[i].fVIP[0].fVertexIndex==fFaces[j].fVIP[2].fVertexIndex)
             f |= e3Recycled;
         }
      }
      faceReuseFlags[i]=f;
   }

   return err;
}

#pragma argsused

Persistent::Base::Error CelAnimMesh::write( StreamIO & sio, int version, int user )
{
   version, user;

   sio.write( (Int32)fVerts.size() );
   sio.write( fnVertsPerFrame );
   sio.write( (Int32)fTextureVerts.size() );
   sio.write( (Int32)fFaces.size() );
   sio.write( (Int32)fFrames.size() );
   sio.write( fnTextureVertsPerFrame );

//      fScale.write( sio );
//      fOrigin.write( sio );
   sio.write( fRadius );

   writeVector( sio, fVerts );
   writeVector( sio, fTextureVerts );
   writeVector( sio, fFaces );
   writeVector( sio, fFrames );
   return Ok;
}

//---------------------------------------------------------------------------

CelAnimMesh::CelAnimMesh()
{
}

CelAnimMesh::~CelAnimMesh()
{
}

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
}; // namespace TS
