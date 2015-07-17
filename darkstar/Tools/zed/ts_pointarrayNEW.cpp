//------------------------------------------------------------------------------

//	$Workfile$
//	$Revision$
//	$Version$

//------------------------------------------------------------------------------

#include <g_surfac.h>

#include "ts_PointArray.h"
#include "ts_RenderContext.h"

namespace TS
{
   void __cdecl clipIntersect (TransformedVertex *p1, TransformedVertex *p2, TransformedVertex *dest, float farDist, int flags, int planeIndex, RectF *);
   //---------------------------------------------------------------------------

   //int PointArray::PolyClipState::fClipCodes[] = 
   //   { ClipNear, ClipFar, ClipLeft, ClipRight, ClipTop, ClipBottom };

   //---------------------------------------------------------------------------

   COREAPI_CD PointArray::PointArray()
   	{
      usesFlags = 0;
		fUseBackFaceTest = FALSE;
      fpTextureArray = 0;
		fpRC = 0;
		// 0 is reserved key for GFX
		fEdgeResetKey = 1;
      farDist = 10000.0f;
      clipIntersectFunc = clipIntersect;
	   }
   COREAPI_CD PointArray::PointArray( RenderContext *context )
      {
      usesFlags = 0;
      fpTextureArray = 0;
      setRenderContext(context);
      clipIntersectFunc = clipIntersect;
      }

   //---------------------------------------------------------------------------

   COREAPI_CD PointArray::~PointArray()
      {
      }

   //---------------------------------------------------------------------------

   void COREAPI PointArray::reset()
      {
      fVerts.clear();
		// 0 is reserved key for GFX
		fEdgeResetKey = (fEdgeResetKey + 1) & ((1 << ResetKeySize) - 1);
		if (!fEdgeResetKey)
			fEdgeResetKey++;
      }
   void COREAPI PointArray::setFarDist(float dist)
   {
      farDist = dist;
   }
   //---------------------------------------------------------------------------
   // add points to transformed/projected vertex array

   int COREAPI PointArray::addPoints( int count, PackedVertex const *verts, 
      Point3F const &scale, Point3F const origin, int *vis )
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

   int COREAPI PointArray::addPoints( int count, TransformedVertex const *verts,
                                      int *vis )
   {
      int start = fVerts.size();
      fVerts.setSize( start + count );

      for ( int i = 0; i < count; i++ )
         fVerts[ start + i ] = verts[ i ];
      
      // do project and set visibility flags...
      *vis = fpRC->getCamera()->project( count, &fVerts[ start ] );
         
      return start;
   }

   //------------------------------------------------------------------------------
	//
	UInt32 COREAPI PointArray::buildEdgeKey(int a, int b)
	{
      return 0;
#if 0
      // Always encode the edge with the lowest index first.
		int k;
		if((a | b) > 2047) // check if it overflowed key size
			return 0;
		if (a > b)
			k = (b << IndexKeySize) | a;
		else
			k = (a << IndexKeySize) | b;
		return (((fEdgeResetKey << (IndexKeySize * 2))) | k) << 1;
#endif
	}

   //------------------------------------------------------------------------------
   // find the intersection with a clipping plane:

   void __cdecl clipIntersect (TransformedVertex *p1, TransformedVertex *p2, TransformedVertex *dest, float farDist, int flags, int planeIndex, RectF *)
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
   int COREAPI PointArray::addProjectedPoint( Point3F const &point )
      {

      AssertFatal( fpRC != 0,
      	"TS::PointArray::addPoint: No render context installed" );

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


   void __cdecl clipIntersectProjected (TransformedVertex *p1, TransformedVertex *p2, TransformedVertex *dest, float, int flags, int planeIndex, RectF *screenViewport)
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
   }

   //------------------------------------------------------------------------------
   // add a point to a clipped poly

   void COREAPI PointArray::clip( int *vertexList, int count)
   {
      int planeIndex;
      int dstVertex[32];
      int curClipMask = 1;
      int *pointSrc = vertexList;
      int *pointDst = dstVertex;
      int vertexIndex;
      int nextCount;
      TransformedVertex *prev;
      TransformedVertex *cur; 
      numClippingPlanes = 7;

      for(planeIndex = 0; planeIndex < numClippingPlanes; planeIndex++)
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
               clipIntersectFunc(prev, cur, dest, farDist, usesFlags, planeIndex, &screenViewport);
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
         curClipMask <<= 1;
      }
      // if we made it this far then:
      // count is number of points to draw and
      // pointSrc holds the indices.

      GFXSurface *surface = fpRC->getSurface();
      Camera *cam = fpRC->getCamera();

      if(fUseTriangleFan)
      {
         int triCount;
         TransformedVertex *p1, *p2, *p3;
         p1 = &fVerts[pointSrc[0]];
         p3 = &fVerts[pointSrc[1]];
         if( !(p1->fStatus & TransformedVertex::Projected) )
             cam->project( p1 );
         if( !(p3->fStatus & TransformedVertex::Projected) )
             cam->project( p3 );

         for(triCount = 2; triCount < count; triCount++)
         {
            p2 = p3;
            p3 = &fVerts[pointSrc[triCount]];
            if( !(p3->fStatus & TransformedVertex::Projected) )
                cam->project( p3 );
            if(fUseBackFaceTest)
            {
               // if it's backfaced, loop to the next tri.
               if( ((p3->fPoint.x - p2->fPoint.x) * 
                    (p1->fPoint.y - p2->fPoint.y) - 
                    (p3->fPoint.y - p2->fPoint.y) * 
                    (p1->fPoint.x - p2->fPoint.x)) < 0.0f )
                  continue;
            }
            surface->addVertex(&p1->fPoint,
                               &p1->fTextureCoord,
                               &p1->fColor,
                               buildEdgeKey(pointSrc[0], pointSrc[triCount-1]));
            surface->addVertex(&p2->fPoint,
                               &p2->fTextureCoord,
                               &p2->fColor,
                               buildEdgeKey(pointSrc[triCount-1], pointSrc[triCount]));
            surface->addVertex(&p3->fPoint,
                               &p3->fTextureCoord,
                               &p3->fColor,
                               buildEdgeKey(pointSrc[triCount], pointSrc[0]));
            surface->emitPoly();
         }
      }
      else
      {
         int prevIndex = count-1;

         for(vertexIndex = 0; vertexIndex < count; vertexIndex++)
         {
            prev = &fVerts[pointSrc[prevIndex]];
            if( !(prev->fStatus & TransformedVertex::Projected) )
                cam->project( prev );
            surface->addVertex(&prev->fPoint, 
                               &prev->fTextureCoord,
                               &prev->fColor,
                               buildEdgeKey(pointSrc[prevIndex], pointSrc[vertexIndex]));
            prevIndex = vertexIndex;
         }
         surface->emitPoly();
      }
   }

   //------------------------------------------------------------------------------
   // Return an index to a GFXPoly formed from the specified set of points.
   // The count argument is the original number of vertices in the poly.
   // The vertex argument is a pointer to an array of indices into the transformed
   // point array.
   // The optional texture argument is a pointer to an array of indices into the
   // textureArray, which must be set up before calling this function.
   //

   void COREAPI PointArray::drawProjectedPoly (int count, VertexIndexPair const *indices,
         int offset )
   {
      clipIntersectFunc = clipIntersectProjected;
      drawPoly(count, indices, offset);
      clipIntersectFunc = clipIntersect;
   }

   void COREAPI PointArray::drawPoly( int count, VertexIndexPair const* indices,
      int offset )
      {
#ifdef DEBUG
      if( usesFlags & usesIntensities )
         for( int i = 0; i < count; i++ )
            {
            AssertFatal( fVerts[offset + indices[i].fVertexIndex].fStatus & 
               TransformedVertex::Lit,
               "TS::PointArray::drawPoly: vertex not lit" );
            }
#endif

   	if( (fVisibility & ClipMask) != ClipAllVis ) 
   	{
			// The clipping code and GFX keep pointers into the
			// vertex list.  We need to make sure that it doesn't
			// realloc if clipped vertices are added.
			fVerts.reserve(fVerts.size() + (count << 1));
			fIndex.clear();
			//
         int i;
         int indexList[32];

   		for (i = 0; i < count; i++, indices++)
         {
            if (usesFlags & usesTextures)
               fVerts[offset + indices->fVertexIndex].fTextureCoord = fpTextureArray[indices->fTextureIndex];
            indexList[i] = indices->fVertexIndex + offset;
   		}
         clip( indexList, count );

   	}
   	else if( fVisibility != ClipNoneVis )
   	{
   		// All points in the buffer are in front of the
   		// hither plane.
         TransformedVertex *prev;
         int vertexIndex;

         VertexIndexPair const* prevPair = indices + count - 1;
         VertexIndexPair const* curPair;

         GFXSurface *surface = fpRC->getSurface();
         for(vertexIndex = 0; vertexIndex < count; vertexIndex++)
         {
            curPair = indices + vertexIndex;

            prev = &fVerts[prevPair->fVertexIndex + offset];
            surface->addVertex(&prev->fPoint, 
                               (usesFlags & usesTextures) ? &fpTextureArray[prevPair->fTextureIndex] : NULL,
                               &prev->fColor,
                               buildEdgeKey(prevPair->fVertexIndex + offset, 
                                            curPair->fVertexIndex + offset));
            prevPair = curPair;
         }
         surface->emitPoly();

   	}
      }
};
