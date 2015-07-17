//------------------------------------------------------------------------------

// $Workfile$
// $Revision$
// $Version$

//------------------------------------------------------------------------------

#include <g_surfac.h>

#include "ts_PointArray.h"
#include "ts_RenderContext.h"

bool gPointArrayHack;
Point2F *gPointArrayHackCoords= NULL;

namespace TS {

extern "C" int __cdecl clip_intersect (TransformedVertex *p1, TransformedVertex *p2, TransformedVertex *dest, float farDist, int flags, int planeIndex, RectF *, bool ortho);

//---------------------------------------------------------------------------

//int PointArray::PolyClipState::fClipCodes[] = 
//   { ClipNear, ClipFar, ClipLeft, ClipRight, ClipTop, ClipBottom };

//---------------------------------------------------------------------------

PointArray::PointArray()
{
   usesFlags = 0;
   fUseBackFaceTest = FALSE;
   fpTextureArray = 0;
   fpRC = 0;
   surface = 0;
   // 0 is reserved key for GFX
   fEdgeResetKey = 1;
   farDist = 10000.0f;
   clipIntersectFunc = clip_intersect;
   isOrtho = false;
}

PointArray::PointArray( RenderContext *context )
{
   usesFlags = 0;
   fpTextureArray = 0;
   setRenderContext(context);
   clipIntersectFunc = clip_intersect;
   isOrtho = false;
}

PointArray::PointArray( GFXSurface *sfc, RectF const &viewport )
{
   usesFlags = 0;
   fpTextureArray = 0;
   fpRC = 0;
   clipIntersectFunc = clip_intersect;
   isOrtho = false;

   surface = sfc;
   screenViewport.upperL.x = float(viewport.upperL.x);
   screenViewport.upperL.y = float(viewport.upperL.y);
   screenViewport.lowerR.x = float(viewport.lowerR.x);
   screenViewport.lowerR.y = float(viewport.lowerR.y);
}

//---------------------------------------------------------------------------

PointArray::~PointArray()
{
}

//---------------------------------------------------------------------------

void PointArray::reset()
{
   fVerts.clear();
   // 0 is reserved key for GFX
   fEdgeResetKey = (fEdgeResetKey + 1) & ((1 << ResetKeySize) - 1);
   if (!fEdgeResetKey)
      fEdgeResetKey++;
}

void PointArray::setFarDist(float dist)
{
   farDist = dist;
}
//---------------------------------------------------------------------------
// add points to transformed/projected vertex array

int PointArray::addPoints(int                 count,
                                  PackedVertex const* verts, 
                                  Point3F const&      scale,
                                  Point3F const       origin,
                                  int*                vis )
{
   AssertFatal( fpRC != 0,
      "TS::PointArray::addPoints: No render context installed" );
   // make space for the new points:
   int start = fVerts.size();
   fVerts.setSize( fVerts.size() + count );

   // transform & project the points:
   *vis = fpRC->getCamera()->transformProject( count, verts, scale, origin, 
      &fVerts[start] );
   return start;
   }

int PointArray::addPoints( int count, const Point3F  * point )
{
   AssertFatal( fpRC != 0,
      "TS::PointArray::addPoints: No render context installed" );

	// make space for the new points
	int start = fVerts.size();
	fVerts.setSize( fVerts.size() + count );

   // transform & project the points:
   fpRC->getCamera()->transformProject( count, point, &fVerts[start] );
   return start;
}

//------------------------------------------------------------------------------
//
UInt32 PointArray::buildEdgeKey(int a, int b)
{
   // Always encode the edge with the lowest index first.
   int k;
   if((a | b) > 2047) // check if it overflowed key size
      return 0;
   if (a > b)
      k = (b << IndexKeySize) | a;
   else
      k = (a << IndexKeySize) | b;
   return (((fEdgeResetKey << (IndexKeySize * 2))) | k) << 1;
}

//------------------------------------------------------------------------------
// find the intersection with a clipping plane:
void __cdecl clipIntersect(TransformedVertex* p1,
                           TransformedVertex* p2,
                           TransformedVertex* dest,
                           float              farDist,
                           int                flags,
                           int                planeIndex,
                           RectF*)
{
   // calculate interpolation fraction:

   RealF t; // t is the fraction.

   Point4F *pT1 = &p1->fTransformedPoint;
   Point4F *pT2 = &p2->fTransformedPoint;

   switch(planeIndex)
   {
      case 0:  // Near
         t = pT1->z / (pT1->z - pT2->z);
         break;
      case 1:  // Far
         t = (pT1->w - pT1->z) / (pT1->w - pT1->z - pT2->w + pT2->z);
         break;
      case 2:  // far sphere
         t = (farDist - p1->fDist) / (p2->fDist - p1->fDist);
         break;
      case 3:  // Left
         t = (pT1->x + pT1->w) / (pT1->x - pT2->x + pT1->w - pT2->w);
         break;
      case 4:  // Right
         t = (pT1->x - pT1->w) / (pT1->x - pT2->x + pT2->w - pT1->w);
         break;
      case 5:  // Top
         t = (pT1->y - pT1->w) / (pT1->y - pT2->y + pT2->w - pT1->w);
         break;
      case 6:  // Bottom
         t = (pT1->y + pT1->w) / (pT1->y - pT2->y + pT1->w - pT2->w);
         break;
      default: // it's a custom plane... we have to do the dot stuff...
         // nothing for now.
         break;
   }
   //---------------------------------------------------------------------------
   // Clip interpolate the endpoints:

   dest->fTransformedPoint.interpolate(*pT1, *pT2, t);

   switch(planeIndex)
   {
      case 3:
         dest->fTransformedPoint.x = -dest->fTransformedPoint.w;
         break;
      case 4:
         dest->fTransformedPoint.x = dest->fTransformedPoint.w;
         break;
      case 5:
         dest->fTransformedPoint.y = dest->fTransformedPoint.w;
         break;
      case 6:
         dest->fTransformedPoint.y = -dest->fTransformedPoint.w;
         break;
   }
   
   //---------------------------------------------------------------------------
   // determine clip code of interpolated point:
   
   dest->fStatus = ClipAllVis;
   
   if(planeIndex < 3)
   {
      if( dest->fTransformedPoint.x < -dest->fTransformedPoint.w )
         dest->fStatus |= ClipLeft;
      else if( dest->fTransformedPoint.x > dest->fTransformedPoint.w )
         dest->fStatus |= ClipRight;
   }
   if(planeIndex < 5)
   {
      if( dest->fTransformedPoint.y < -dest->fTransformedPoint.w )
         dest->fStatus |= ClipBottom;
      else if( dest->fTransformedPoint.y > dest->fTransformedPoint.w )
         dest->fStatus |= ClipTop;
   }
   // loop through extra planes...

   //---------------------------------------------------------------------------
   // do perspective divide, only if point is inside the view volume:
   AssertFatal( t >= 0.0f && t <= 1.0f, "ow!" )

   //---------------------------------------------------------------------------
   // Interpolate values for the point.

   // Interpolate the distance coord:
   dest->fDist = p1->fDist + (p2->fDist - p1->fDist) * t;

   // Interpolate the texture coords, if used:
   if( flags & PointArray::usesTextures )
      dest->fTextureCoord.interpolate( p1->fTextureCoord, 
         p2->fTextureCoord, t );

   // Interpolate vertex intensities, if used:
   if( flags & PointArray::usesIntensities ) 
      dest->fColor.color.interpolate( 
         p1->fColor.color, p2->fColor.color, t );

   // Interpolate vertex haze, if used:
   if (flags & PointArray::usesHazes)
      dest->fColor.haze = p1->fColor.haze + 
               t * (p2->fColor.haze - p1->fColor.haze);
   
}

int PointArray::addProjectedPoint( Point3F const &point )
{
   AssertFatal( fpRC != 0 || surface != 0,
      "TS::PointArray::addPoint: No render context or surface installed" );

   // make space for the new point:
   fVerts.increment();
   // place the projected point into the pointArray
   TransformedVertex *v = &fVerts[fVerts.size() - 1];
   v->fPoint = point;
   v->fStatus = TransformedVertex::Projected;
   if(point.x < screenViewport.upperL.x)
      v->fStatus |= ClipLeft;
   else if(point.x > screenViewport.lowerR.x)
      v->fStatus |= ClipRight;
   if(point.y < screenViewport.upperL.y)
      v->fStatus |= ClipTop;
   else if(point.y > screenViewport.lowerR.y)
      v->fStatus |= ClipBottom;
   return fVerts.size() - 1;
}


int __cdecl clipIntersectProjected(TransformedVertex* p1,
                                   TransformedVertex* p2,
                                   TransformedVertex* dest,
                                   float,
                                   int flags,
                                   int planeIndex,
                                   RectF* screenViewport,
                                   bool)
{
   // calculate interpolation fraction:

   RealF t; // t is the fraction.

   Point3F *pT1 = &p1->fPoint;
   Point3F *pT2 = &p2->fPoint;

   //---------------------------------------------------------------------------
   // Clip interpolate the endpoints:

   switch(planeIndex)
   {
      case 3:
         t = (screenViewport->upperL.x - pT1->x) / (pT2->x - pT1->x);
         dest->fPoint.x = screenViewport->upperL.x;
         dest->fPoint.y = pT1->y + (pT2->y - pT1->y) * t;
         dest->fPoint.z = pT1->z + (pT2->z - pT1->z) * t;
         break;
      case 4:
         t = (screenViewport->lowerR.x - pT1->x) / (pT2->x - pT1->x);
         dest->fPoint.x = screenViewport->lowerR.x;
         dest->fPoint.y = pT1->y + (pT2->y - pT1->y) * t;
         dest->fPoint.z = pT1->z + (pT2->z - pT1->z) * t;
         break;
      case 5:
         t = (screenViewport->upperL.y - pT1->y) / (pT2->y - pT1->y);
         dest->fPoint.y = screenViewport->upperL.y;
         dest->fPoint.x = pT1->x + (pT2->x - pT1->x) * t;
         dest->fPoint.z = pT1->z + (pT2->z - pT1->z) * t;
         break;
      case 6:
         t = (screenViewport->lowerR.y - pT1->y) / (pT2->y - pT1->y);
         dest->fPoint.y = screenViewport->lowerR.y;
         dest->fPoint.x = pT1->x + (pT2->x - pT1->x) * t;
         dest->fPoint.z = pT1->z + (pT2->z - pT1->z) * t;
         break;
   }
   
   //---------------------------------------------------------------------------
   // determine clip code of interpolated point:

   dest->fStatus = TransformedVertex::Projected;
   
   if(planeIndex < 5)
   {
      if( dest->fPoint.y < screenViewport->upperL.y)
         dest->fStatus |= ClipTop;
      else if(dest->fPoint.y > screenViewport->lowerR.y)
         dest->fStatus |= ClipBottom;
   }

   //---------------------------------------------------------------------------
   // Interpolate values for the point.

   // Interpolate the texture coords, if used:
   if( flags & PointArray::usesTextures )
      dest->fTextureCoord.interpolate( p1->fTextureCoord, 
         p2->fTextureCoord, t );

   // Interpolate vertex intensities, if used:
   if( flags & PointArray::usesIntensities ) 
      dest->fColor.color.interpolate( 
         p1->fColor.color, p2->fColor.color, t );

   // Interpolate vertex haze, if used:
   if (flags & PointArray::usesHazes)
      dest->fColor.haze = p1->fColor.haze + 
               t * (p2->fColor.haze - p1->fColor.haze);
   return dest->fStatus;
}

//------------------------------------------------------------------------------
// Return an index to a GFXPoly formed from the specified set of points.
// The count argument is the original number of vertices in the poly.
// The vertex argument is a pointer to an array of indices into the transformed
// point array.
// The optional texture argument is a pointer to an array of indices into the
// textureArray, which must be set up before calling this function.
//

void PointArray::drawProjectedPoly(int                    count,
                                           VertexIndexPair const *indices,
                                           int                    offset)
{
   clipIntersectFunc = clipIntersectProjected;
   drawPoly(count, indices, offset);
   clipIntersectFunc = clip_intersect;
}

void PointArray::drawLine(int i1,
                                  int i2,
                                  int color)
{
   int planeIndex;
   int curClipMask = 1;
   numClippingPlanes = 7;

   TransformedVertex *v1 = &fVerts[i1];
   TransformedVertex *v2 = &fVerts[i2];

   for(planeIndex = 0; planeIndex < numClippingPlanes; planeIndex++, curClipMask <<= 1)
   {
      // is it gone?
      if(v1->fStatus & v2->fStatus & curClipMask)
         return;

      // does the line cross the plane?
      if((v1->fStatus | v2->fStatus) & curClipMask )
      {
         // needs clipping
         fVerts.increment();
         TransformedVertex *dest = &fVerts.last();
         clipIntersectFunc(v1, v2, dest, farDist, usesFlags, planeIndex, &screenViewport, false);

         if(v1->fStatus & curClipMask)
            v1 = dest;
         else
            v2 = dest;
      }
   }
   surface->drawLine2d(&Point2I(Int32(v1->fPoint.x), Int32(v1->fPoint.y)),
                                  &Point2I(Int32(v2->fPoint.x), Int32(v2->fPoint.y)),
                                  color);
}

void PointArray::drawTriangleClip( const VertexIndexPair * indices, int offset)
{
   int idx0, idx1, idx2;
   if (usesFlags & usesTextures)
   {
      TransformedVertex * vert0, * vert1, * vert2;

      idx0 = indices->fVertexIndex + offset;
      vert0 = &fVerts[idx0];
      vert0->fTextureCoord = fpTextureArray[indices->fTextureIndex];

      indices++;

      idx1 = indices->fVertexIndex + offset;
      vert1 = &fVerts[idx1];
      vert1->fTextureCoord = fpTextureArray[indices->fTextureIndex];

      indices++;

      idx2 = indices->fVertexIndex + offset;
      vert2 = &fVerts[idx2];
      vert2->fTextureCoord = fpTextureArray[indices->fTextureIndex];
   }
   else
   {
      idx0 = indices->fVertexIndex + offset;
      indices++;
      idx1 = indices->fVertexIndex + offset;
      indices++;
      idx2 = indices->fVertexIndex + offset;
   }

   int indexList1[64];
   int indexList2[64];
   indexList1[0] = idx0;
   indexList1[1] = idx1;
   indexList1[2] = idx2;
   int *pointSrc = indexList1;
   int *pointDst = indexList2;
   
   TransformedVertex *prev;
   TransformedVertex *cur; 

   int curClipMask = 1;
   numClippingPlanes = 7;
   
   int count = 3;
   int vertexIndex;
   
   fVerts.reserve(fVerts.size() + (numClippingPlanes << 1));

   for(int planeIndex = 0; planeIndex < numClippingPlanes; planeIndex++)
   {
      if(curClipMask & fVisibility)
      {
         int nextCount = 0;
         prev = &fVerts[pointSrc[count-1]];

         for(vertexIndex = 0; vertexIndex < count; vertexIndex++)
         {
            cur = &fVerts[pointSrc[vertexIndex]];
            // is it out->in or in->out?
            // if so, emit the clipIntersect of the points.
            if((prev->fStatus ^ cur->fStatus) & curClipMask)
            {
               pointDst[nextCount++] = fVerts.size();
               fVerts.increment();
               TransformedVertex *dest = &fVerts.last();
               fVisibility |= clipIntersectFunc(prev, cur, dest, farDist, usesFlags, planeIndex, &screenViewport, isOrtho);
            }

            // if the current point is inside, emit it too.
            if(!(cur->fStatus & curClipMask))
               pointDst[nextCount++] = pointSrc[vertexIndex];
            prev = cur;
         }
         if((count = nextCount) < 3) // if there aren't enough points to make a triangle, return.
            return;
         int *temp = pointSrc;
         pointSrc = pointDst;
         pointDst = temp;
      }
      curClipMask <<= 1;
   }

   // if we made it this far then:
   // count is number of points to draw and
   // pointSrc holds the indices.

   int prevIndex = count-1;

   for(vertexIndex = 0; vertexIndex < count; vertexIndex++)
   {
      prev = &fVerts[pointSrc[prevIndex]];
      surface->addVertex(&prev->fPoint, 
                         &prev->fTextureCoord,
                         &prev->fColor,
                         buildEdgeKey(pointSrc[prevIndex], pointSrc[vertexIndex]));
      prevIndex = vertexIndex;
   }
   surface->emitPoly();
}

void PointArray::drawTriangle( const VertexIndexPair * indices, int offset)
{
#ifdef DEBUG
   if( usesFlags & usesIntensities ) {
      for( int i = 0; i < 3; i++ )
         {
         AssertFatal( fVerts[offset + indices[i].fVertexIndex].fStatus & 
            TransformedVertex::Lit,
            "TS::PointArray::drawPoly: vertex not lit" );
         }
   }
#endif

   if (fVisibility != (ClipAllVis & ClipMask))
   {
      drawTriangleClip(indices,offset);
      return;
   }
      
   int idx0, idx1, idx2;
   TransformedVertex * vert0, * vert1, * vert2;
   if (usesFlags & usesTextures)
   {
      idx0 = indices->fVertexIndex + offset;
      vert0 = &fVerts[idx0];
      vert0->fTextureCoord = fpTextureArray[indices->fTextureIndex];

      indices++;

      idx1 = indices->fVertexIndex + offset;
      vert1 = &fVerts[idx1];
      vert1->fTextureCoord = fpTextureArray[indices->fTextureIndex];

      indices++;

      idx2 = indices->fVertexIndex + offset;
      vert2 = &fVerts[idx2];
      vert2->fTextureCoord = fpTextureArray[indices->fTextureIndex];
   }
   else
   {
      idx0 = indices->fVertexIndex + offset;
      vert0 = &fVerts[idx0];

      indices++;

      idx1 = indices->fVertexIndex + offset;
      vert1 = &fVerts[idx1];

      indices++;

      idx2 = indices->fVertexIndex + offset;
      vert2 = &fVerts[idx2];
   }

   // we already know we don't need to clip, so don't worry about that stuff
   surface->addVertex(&vert2->fPoint, 
                      &vert2->fTextureCoord,
                      &vert2->fColor,
                      buildEdgeKey(idx2, idx0));
   surface->addVertex(&vert0->fPoint, 
                      &vert0->fTextureCoord,
                      &vert0->fColor,
                      buildEdgeKey(idx0, idx1));
   surface->addVertex(&vert1->fPoint, 
                      &vert1->fTextureCoord,
                      &vert1->fColor,
                      buildEdgeKey(idx1, idx2));
   surface->emitPoly();
}

void PointArray::drawPoly( int count, VertexIndexPair const* indices,
   int offset )
{
#ifdef DEBUG
   if( usesFlags & usesIntensities ) {
      for( int i = 0; i < count; i++ )
         {
         AssertFatal( fVerts[offset + indices[i].fVertexIndex].fStatus & 
            TransformedVertex::Lit,
            "TS::PointArray::drawPoly: vertex not lit" );
         }
   }
#endif
   
   int vertexIndex;
   int orVisFlags = 0, andVisFlags = ClipMask;
   int indexList[64];

   for(vertexIndex = 0; vertexIndex < count; vertexIndex++, indices++)
   {
      indexList[vertexIndex] = indices->fVertexIndex + offset;
      TransformedVertex *vert = &fVerts[offset + indices->fVertexIndex];

      int vis = vert->fStatus;
      if (usesFlags & usesTextures)
         vert->fTextureCoord = fpTextureArray[indices->fTextureIndex];
      if (gPointArrayHack)
      {
         vert->fColor.color.red = gPointArrayHackCoords[indices->fTextureIndex].x;
         vert->fColor.color.green = gPointArrayHackCoords[indices->fTextureIndex].y;
         vert->fColor.color.blue = 0;
         usesFlags |= usesIntensities;
      }

      orVisFlags |= vis;
      andVisFlags &= vis;
   }
   if(andVisFlags)
   {
      if(gPointArrayHack)
      {
         usesFlags &= ~usesIntensities;
      }
      return;
   }

   int *pointSrc = indexList;
   TransformedVertex *prev;
   
   if(orVisFlags & ClipMask)
   {
      int planeIndex;
      int dstVertex[64];
      int curClipMask = 1;
      int *pointDst = dstVertex;
      int nextCount;
      TransformedVertex *cur; 
      numClippingPlanes = 7;

      fVerts.reserve(fVerts.size() + (numClippingPlanes << 1));

      for(planeIndex = 0; planeIndex < numClippingPlanes; planeIndex++)
      {
         if(curClipMask & orVisFlags)
         {
            nextCount = 0;
            prev = &fVerts[pointSrc[count-1]];

            for(vertexIndex = 0; vertexIndex < count; vertexIndex++)
            {
               cur = &fVerts[pointSrc[vertexIndex]];
               // is it out->in or in->out?
               // if so, emit the clipIntersect of the points.
               if((prev->fStatus ^ cur->fStatus) & curClipMask)
               {
                  pointDst[nextCount++] = fVerts.size();
                  fVerts.increment();
                  TransformedVertex *dest = &fVerts.last();
                  orVisFlags |= clipIntersectFunc(prev, cur, dest, farDist, usesFlags, planeIndex, &screenViewport, isOrtho);
               }

               // if the current point is inside, emit it too.
               if(!(cur->fStatus & curClipMask))
                  pointDst[nextCount++] = pointSrc[vertexIndex];
               prev = cur;
            }
            if((count = nextCount) < 3) // if there aren't enough points to make a triangle, return.
            {
               if(gPointArrayHack)
                  usesFlags &= ~usesIntensities;
               return;
            }
            int *temp = pointSrc;
            pointSrc = pointDst;
            pointDst = temp;
         }
         curClipMask <<= 1;
      }
   }
   if(gPointArrayHack)
      usesFlags &= ~usesIntensities;
   // if we made it this far then:
   // count is number of points to draw and
   // pointSrc holds the indices.

   int prevIndex = count-1;

   for(vertexIndex = 0; vertexIndex < count; vertexIndex++)
   {
      prev = &fVerts[pointSrc[prevIndex]];
      surface->addVertex(&prev->fPoint, 
                         &prev->fTextureCoord,
                         &prev->fColor,
                         buildEdgeKey(pointSrc[prevIndex], pointSrc[vertexIndex]));
      prevIndex = vertexIndex;
   }
   surface->emitPoly();
}

bool PointArray::testPolyVis( int count, VertexIndexPair const* indices,
                                      int offset )
{
   int vertexIndex;
   int orVisFlags = 0, andVisFlags = ClipMask;
   int indexList[64];

   for(vertexIndex = 0; vertexIndex < count; vertexIndex++, indices++)
   {
      indexList[vertexIndex] = indices->fVertexIndex + offset;
      TransformedVertex *vert = &fVerts[offset + indices->fVertexIndex];

      int vis = vert->fStatus;

      orVisFlags |= vis;
      andVisFlags &= vis;
   }
   if(andVisFlags)
      return false;

   int *pointSrc = indexList;
   TransformedVertex *prev;
   
   if(orVisFlags & ClipMask)
   {
      int planeIndex;
      int dstVertex[64];
      int curClipMask = 1;
      int *pointDst = dstVertex;
      int nextCount;
      TransformedVertex *cur; 
      numClippingPlanes = 7;

      fVerts.reserve(fVerts.size() + (numClippingPlanes << 1));

      for(planeIndex = 0; planeIndex < numClippingPlanes; planeIndex++)
      {
         if(curClipMask & orVisFlags)
         {
            nextCount = 0;
            prev = &fVerts[pointSrc[count-1]];

            for(vertexIndex = 0; vertexIndex < count; vertexIndex++)
            {
               cur = &fVerts[pointSrc[vertexIndex]];
               // is it out->in or in->out?
               // if so, emit the clipIntersect of the points.
               if((prev->fStatus ^ cur->fStatus) & curClipMask)
               {
                  pointDst[nextCount++] = fVerts.size();
                  fVerts.increment();
                  TransformedVertex *dest = &fVerts.last();
                  orVisFlags |= clipIntersectFunc(prev, cur, dest, farDist, usesFlags, planeIndex, &screenViewport, isOrtho);
               }

               // if the current point is inside, emit it too.
               if(!(cur->fStatus & curClipMask))
                  pointDst[nextCount++] = pointSrc[vertexIndex];
               prev = cur;
            }
            if((count = nextCount) < 3) // if there aren't enough points to make a triangle, return.
               return false;
            int *temp = pointSrc;
            pointSrc = pointDst;
            pointDst = temp;
         }
         curClipMask <<= 1;
      }
   }

   // if we made it this far then:
   //  poly is visible
   return true;
}

}; // namespace TS
