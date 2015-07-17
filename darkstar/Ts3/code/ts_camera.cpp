//---------------------------------------------------------------------------

//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
// $Version$
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#include "ts_camera.h"
#include "ts_BoundingBox.h"
#include "ts_vertex.h"


extern "C"  void __cdecl ts_ProjectF( Point3F const *src, Point4F *dst, 
   Point2F const *center, Point3F const *scale );

extern "C"  void __cdecl ts_TransformProjectF( Point3F const *src, 
   Point4F *dst, TMat3F const *tmat, Point2F const *center );

extern "C"  void __cdecl ts_TransformProjectArrayF( Point3F const *src, 
   Point4F *dst, TMat3F const *tmat, Point2F const *center,
   int count, int srcStride, int dstStride );

extern "C"  Bool __fastcall ts_Point3F_Point3F_Point3F_vis_test( 
   Point3F const & a, Point3F const &b, Point3F const &vec );


namespace TS
{
   //---------------------------------------------------------------------------
   // Camera methods
   //---------------------------------------------------------------------------

   //---------------------------------------------------------------------------
   // lock the camera, prepare for rendering

   void Camera::lock()
      {
      AssertFatal( fTOC.size() == 1 && 
         fTOS.size() == 1 && fTOW.size() == 2, 
         "TS::Camera::lock: invalid transform stack state" );
      AssertFatal( fVisibility.size() == 1, 
         "TS::Camera::lock: invalid visibility stack state" );
      AssertFatal( !isLocked(), "TS::Camera::lock: camera already locked" );

      // Build TWC:
      buildProjectionMatrix();

      // Determine viewport clipping planes:

		if (fWVPChanged)
		{
	      fLeftPlane   = fWorldViewport.upperL.x * fInvNearDist;
   	   fRightPlane  = fWorldViewport.lowerR.x * fInvNearDist;
      	fTopPlane    = fWorldViewport.upperL.y * fInvNearDist;
      	fBottomPlane = fWorldViewport.lowerR.y * fInvNearDist;
			fLeftLenSq   = 1.0f + fLeftPlane * fLeftPlane; // square of length of normal to left plane
			fRightLenSq  = 1.0f + fRightPlane * fRightPlane;
			fTopLenSq    = 1.0f + fTopPlane * fTopPlane;
			fBottomLenSq = 1.0f + fBottomPlane * fBottomPlane;
			fLeftLen     = m_sqrt(fLeftLenSq);
			fRightLen     = m_sqrt(fRightLenSq);
			fTopLen     = m_sqrt(fTopLenSq);
			fBottomLen     = m_sqrt(fBottomLenSq);
			fWVPChanged=false;
		}

      fTOC.push( fTWC );   // TOC = TOW + TWC

      fTOS.push( fTCS );   // TOS = TOW + TWC + TCS
      fTOS.push( fTWC );

      fLocked = TRUE;
      }

   //---------------------------------------------------------------------------
   // unlock the camera

   void Camera::unlock()
      {
      AssertFatal( isLocked(), "TS::Camera::unlock: camera not locked" );

      fTOC.pop();

      fTOS.pop();
      fTOS.pop();

      AssertFatal( fTOC.size() == 1 &&
         fTOS.size() == 1 && fTOW.size() == 2, 
         "TS::Camera::unlock: pushTransform not balanced by popTransform" );
      AssertFatal( fVisibility.size() == 1,
         "TS::Camera::unlock: pushVisibility not balanced by popVisibility" );
      fLocked = FALSE;

      }


   //---------------------------------------------------------------------------
   // transform a point and determine clipping code:

   inline void Camera::getVisibilityFlags( TransformedVertex *pDest ) const
   {
      // do hither check & possibly project:
      pDest->fStatus = ClipAllVis;

      if( pDest->fTransformedPoint.z < 0.0f )
         pDest->fStatus |= ClipNear;
      else if( pDest->fTransformedPoint.z > pDest->fTransformedPoint.w )
         pDest->fStatus |= ClipFar;

      if( pDest->fTransformedPoint.x < -pDest->fTransformedPoint.w )
         pDest->fStatus |= ClipLeft;
      else if( pDest->fTransformedPoint.x > pDest->fTransformedPoint.w )
         pDest->fStatus |= ClipRight;

      if( pDest->fTransformedPoint.y < -pDest->fTransformedPoint.w )
         pDest->fStatus |= ClipBottom;
      else if( pDest->fTransformedPoint.y > pDest->fTransformedPoint.w )
         pDest->fStatus |= ClipTop;
   }

   //---------------------------------------------------------------------------
   // Undo divide done  by transformProject2()

   void Camera::transformProject2Inverse( Point4F const & src, Point4F *pDest ) const
      {
      AssertFatal( pDest != 0, 
         "TS::Camera::TransformProject2Inverse: invalid destination pointer." );
   
      RealF w = 1.0f / src.w;
      pDest->x = (src.x - fVc.x) * w / fVs.x;
      pDest->y = (src.y - fVc.y) * w / fVs.y;
      pDest->z = src.z * w;
      pDest->w = w;
      }

   void Camera::transformProject2Inverse( TransformedVertex *pVert ) const
      {
      AssertFatal( pVert  != 0, 
         "TS::Camera::TransformProject2Inverse: invalid destination pointer." );
      //transformProject2Inverse( pVert->fPoint, &pVert->fTransformedPoint );
      pVert->fStatus &= ~TransformedVertex::Projected;
      }

   //---------------------------------------------------------------------------
   // Determining Visibility of Spheres
   //
   // Determines the visibility of a sphere; that is, determines
   // whether a sphere is totally inside, totally outside, or partially
   // inside the view volume defined by the hither distance, the focal
   // length, and the viewport.
   //

   int Camera::testVisibility( SphereF const &bs ) const
      {
      AssertFatal( isLocked(),
         "TS::Camera::testVisibility: must call lock() first" );

      int result = getVisibility();
      
      if( result == ClipAllVis || result == ClipNoneVis )
         return result;

      // Assume there are no visibility problems to start with:
      result = ClipAllVis;

   	// Transform the center point.  If the entire sphere is not behind the
   	// hither plane, then perform a more detail test.

   	Point3F transformedLoc;
      m_mul(bs.center, fTOC, &transformedLoc);
   	//transform(bs.center,&transformedLoc);
		if(type == OrthographicCameraType)
      {
         if(transformedLoc.x > fWorldViewport.upperL.x && transformedLoc.x < fWorldViewport.lowerR.x &&
            transformedLoc.z < fWorldViewport.upperL.y && transformedLoc.z > fWorldViewport.lowerR.y &&
            transformedLoc.y > fNearDist && transformedLoc.y < fFarDist)
            return ClipAllVis;
         else
            return ClipNoneVis;
      }

      if( (transformedLoc.x - fRightLen * bs.radius) > (transformedLoc.y * fRightPlane) )
         return ClipNoneVis;
      if( (transformedLoc.x - fLeftLen * bs.radius) < (transformedLoc.y * fLeftPlane) )
         result |= ClipLeft;

      if( (transformedLoc.x + fLeftLen * bs.radius) < (transformedLoc.y * fLeftPlane) )
         return ClipNoneVis;
      if( (transformedLoc.x + fRightLen * bs.radius) > (transformedLoc.y * fRightPlane) )
         result |= ClipRight;

      if( (transformedLoc.z - fTopLen * bs.radius) > (transformedLoc.y * fTopPlane) )
         return ClipNoneVis;
      if( (transformedLoc.z - fBottomLen * bs.radius) < (transformedLoc.y * fBottomPlane) )
         result |= ClipBottom;

      if( (transformedLoc.z + fBottomLen * bs.radius) < (transformedLoc.y * fBottomPlane) )
         return ClipNoneVis;
      if( (transformedLoc.z + fTopLen * bs.radius) > (transformedLoc.y * fTopPlane) )
         result |= ClipTop;

      if( (transformedLoc.y - bs.radius) < fNearDist )
         result |= ClipNear;

      if( (transformedLoc.y + bs.radius) < fNearDist )
         return ClipNoneVis;

      if( (transformedLoc.y - bs.radius) > fFarDist )
         return ClipNoneVis;

      if( (transformedLoc.y + bs.radius) > fFarDist )
         result |= ClipFar;
      
   	return result;
      }

   //---------------------------------------------------------------------------
   // Determining Visibility of Points
   //
   // Determines the visibility of a point; that is, determines
   // whether a point is totally inside, totally outside, or partially
   // inside the view volume defined by the hither distance, the focal
   // length, and the viewport.
   //

   int Camera::testVisibility( const Point3F &loc ) const
   {
      AssertFatal( isLocked(),
         "TS::Camera::testVisibility: must call lock() first" );

      int result = getVisibility();
      
      if( result == ClipAllVis || result == ClipNoneVis )
      	return result;

      // Assume there are no visibility problems to start with:
      result = ClipAllVis;

   	// Transform the point.  If any clip codes are set, the point is not in
   	// the view volume.

      TransformedVertex transformedVert;
      fptrTransformProjectPrepare(fTOC, fTOS, &fVs, &fVc, fInvNearDist);
      int ret = fptrTransformProject(&loc, &transformedVert);
      if( ret & ClipMask )
         return ClipNoneVis;

   	return result;
   }

   //---------------------------------------------------------------------------
   // Determining Visibility of BoundingBoxes
   //
   // Determines the visibility of a boundingBox; first tests the bounding 
   // sphere.  If the result is partially visible, then the box is tested.
   //

   int Camera::testVisibility( BoundingBox const &bb ) const
      {
		return testVisibility(bb.getBox());
      }

   //---------------------------------------------------------------------------
   // Determine Visibility of a Box3F -- tests box and sphere together
   int Camera::testVisibility( Box3F const &box, float fd ) const
   {
      AssertFatal( isLocked(),
         "TS::Camera::testVisibility: must call lock() first" );

      int result = getVisibility();
      
      if( result == ClipAllVis || result == ClipNoneVis )
         return result;

      // If we haven't clipped at all yet, need to clip against all planes still
		if (result == ClipUnknown)
	      result = ClipMask & ~ClipFarSphere; // clip far sphere not used here

   	// Transform the center point.

		const TMat3F & TOC = fTOC;
   	Point3F boxCenter, transformedLoc;
		boxCenter.x = 0.5f * (box.fMin.x + box.fMax.x);
		boxCenter.y = 0.5f * (box.fMin.y + box.fMax.y);
		boxCenter.z = 0.5f * (box.fMin.z + box.fMax.z);
		float radiusSq = m_sqr((box.fMax.x-box.fMin.x)*0.5f) +
		                 m_sqr((box.fMax.y-box.fMin.y)*0.5f) +
							  m_sqr((box.fMax.z-box.fMin.z)*0.5f);
      m_mul(boxCenter, TOC, &transformedLoc);

		if(type == OrthographicCameraType)
      {
         return ClipAllVis;
			if (fd<0.0f)
				fd = fFarDist;
         if(transformedLoc.x > fWorldViewport.upperL.x && transformedLoc.x < fWorldViewport.lowerR.x &&
            transformedLoc.z < fWorldViewport.upperL.y && transformedLoc.z > fWorldViewport.lowerR.y &&
            transformedLoc.y > fNearDist && transformedLoc.y < fd)
            return ClipAllVis;
         else
            return ClipNoneVis;
      }
		
		// strategy:  first do sphere tests that give immediate results
		//            then sphere tests that will eliminate need for box tests that give
		//                  immediate results
		//            then do those box tests that give immediate results
		//            then do partial result box tests (e.g., ClipLeft, ClipTop...)

		float centerToRight, ctrSq;
		if (result&ClipRight)
		{
			// distance of box/sphere center to right plane
			centerToRight  = transformedLoc.x - transformedLoc.y * fRightPlane;
			// get square of centerToRight but keep sign
			ctrSq = centerToRight * fabs(centerToRight); 
			if ( ctrSq > fRightLenSq * radiusSq )
				return ClipNoneVis;
		}

		float centerToLeft, ctlSq;
		if (result&ClipLeft)
		{
			// distance of box/sphere center to left plane
			centerToLeft   = transformedLoc.x - transformedLoc.y * fLeftPlane;
			// get square of centerToLeft but keep sign
			ctlSq = centerToLeft * fabs(centerToLeft);
      	if( ctlSq < -fLeftLenSq * radiusSq )
         	return ClipNoneVis;
		}

		float centerToTop, cttSq;
		if (result&ClipTop)
		{
			// distance of box/sphere center to top plane
			centerToTop    = transformedLoc.z - transformedLoc.y * fTopPlane;
			// get square of centerToTop but keep sign
			cttSq = centerToTop * fabs(centerToTop);
      	if( cttSq > fTopLenSq * radiusSq )
         	return ClipNoneVis;
		}

		float centerToBottom, ctbSq;
		if (result&ClipBottom)
		{
			// distance of box/sphere center to bottom plane
			centerToBottom = transformedLoc.z - transformedLoc.y * fBottomPlane;
			// get square of centerToBottom but keep sign
			ctbSq = centerToBottom * fabs(centerToBottom);
      	if( ctbSq < -fBottomLenSq * radiusSq )
         	return ClipNoneVis;
		}

		float centerToNear, ctnSq;
		if (result&ClipNear)
		{
			// distance of box/sphere center to near plane
			centerToNear   = transformedLoc.y - fNearDist;
			// get square of centerToNear but keep sign
			ctnSq = centerToNear * fabs(centerToNear);
      	if( ctnSq < -radiusSq )
      	   return ClipNoneVis;
		}

		float centerToFar, ctfSq;
		if (result&ClipFar)
		{
			// use passed in far distance or cameras?
			if (fd<0.0f)
				fd = fFarDist;
			// distance of box center/sphere center to far plane
			centerToFar    = transformedLoc.y - fd;
			// get square of centerToFar but keep sign
			ctfSq = centerToFar * fabs(centerToFar);
      	if( ctfSq > radiusSq)
         	return ClipNoneVis;
		}

		// sphere not totally outside of viewcone

		// get x,y,z vectors of box
		Point3F x,y,z;
		TOC.getRow(0,&x);
		TOC.getRow(1,&y);
		TOC.getRow(2,&z);

		// get radial dimensions of box in its own space
		float rx,ry,rz;
		rx = box.len_x();
		ry = box.len_y();
		rz = box.len_z();

		// does sphere hit right plane?
		float rightDist;
      if( result&ClipRight && ctrSq > -fRightLenSq * radiusSq)
		{
			// do box check on right plane

			// extent of box (from box center) along right plane normal
			rightDist =  rx * fabs(x.x - x.y * fRightPlane) + 
			             ry * fabs(y.x - y.y * fRightPlane) +
	   	             rz * fabs(z.x - z.y * fRightPlane);

	      if( centerToRight > rightDist )
   	      return ClipNoneVis;
		}
      else
      {
         result &= ~ClipRight;
      }


		// does sphere hit left plane?
		float leftDist;
      if( result&ClipLeft && ctlSq < fLeftLenSq * radiusSq )
		{
			// do box check on left plane

			// extent of box (from box center) along left plane normal
			leftDist =   rx * fabs(x.x - x.y * fLeftPlane) + 
			             ry * fabs(y.x - y.y * fLeftPlane) +
		                rz * fabs(z.x - z.y * fLeftPlane);

	      if( centerToLeft < -leftDist )
   	      return ClipNoneVis;
		}
      else
      {
         result &= ~ClipLeft;
      }

		// does sphere hit top plane?
		float topDist;
      if( result&ClipTop && cttSq > -fTopLenSq * radiusSq )
		{
			// do box check on top plane

			// extent of box (from box center) along top plane normal
			topDist =    rx * fabs(x.z - x.y * fTopPlane) + 
			             ry * fabs(y.z - y.y * fTopPlane) +
		   	          rz * fabs(z.z - z.y * fTopPlane);

	      if( centerToTop > topDist )
   	      return ClipNoneVis;
		}
      else
      {
         result &= ~ClipTop;
      }


		// does sphere hit bottom plane?
		float bottomDist;
      if( result&ClipBottom && ctbSq < fBottomLenSq * radiusSq )
		{
			// do box check on bottom plane

			// extent of box (from box center) along bottom plane normal
			bottomDist = rx * fabs(x.z - x.y * fBottomPlane) + 
			             ry * fabs(y.z - y.y * fBottomPlane) +
		   	          rz * fabs(z.z - z.y * fBottomPlane);

	      if( centerToBottom < -bottomDist )
   	      return ClipNoneVis;
		}
      else
      {
         result &= ~ClipBottom;
      }


		float nearfarDist;
		if (result&(ClipNear|ClipFar))
		{
			// extent of box (from box center) along near/far plane normal
			nearfarDist = rx * fabs(x.y) + ry * fabs(y.y) + rz * fabs(z.y);
			// do box check on near plane
      	if( (result&ClipNear) && (centerToNear < -nearfarDist) )
  	      	return ClipNoneVis;
			// do box check on far plane
      	if( (result&ClipFar) && (centerToFar >  nearfarDist) )
  	      	return ClipNoneVis;
		}

      if( result&ClipLeft && centerToLeft >= leftDist )
         result &= ~ClipLeft;

      if( result&ClipRight && centerToRight <= -rightDist)
         result &= ~ClipRight;

      if( result&ClipBottom && centerToBottom >= bottomDist )
         result &= ~ClipBottom;

      if( result&ClipTop && centerToTop <= -topDist )
         result &= ~ClipTop;

      if( result&ClipNear && centerToNear >= nearfarDist )
         result &= ~ClipNear;
      
      if( result&ClipFar && centerToFar <= -nearfarDist)
         result &= ~ClipFar;

   	return result;
   }

   //---------------------------------------------------------------------------
	// build bounding box around viewcone
   //---------------------------------------------------------------------------

   void PerspectiveCamera::getViewconeBox( Box3F & box, float fd ) const
   {
		// use fd for far dist, unless fd<0, then use camera's far distance
		if (fd<0.0f)
			fd = fFarDist;

		Point3F farPoint, farX, farZ;

		// get farPoint -- straight ahead of camera fFarDist in world coord.s
		fTCW.getRow(1,&farPoint);
		farPoint *= fd;
		farPoint += fTCW.p;

		// get farX -- vector to right edge of view cone at far dist. in world coord.s
		fTCW.getRow(0,&farX);
		farX *= fd * fInvNearDist * fWorldViewport.upperL.x;

		// get farZ -- vector to top edge of view cone at far dist. in world coord.s
		fTCW.getRow(2,&farZ);
		farZ *= fd * fInvNearDist * fWorldViewport.lowerR.y;

		// take absolute values of farX and farZ:
		farX.x = fabs(farX.x);
		farX.y = fabs(farX.y);
		farX.z = fabs(farX.z);
		farZ.x = fabs(farZ.x);
		farZ.y = fabs(farZ.y);
		farZ.z = fabs(farZ.z);

		// get box min accounting for far points only
		box.fMin.x = farPoint.x - farX.x - farZ.x;
		box.fMin.y = farPoint.y - farX.y - farZ.y;
		box.fMin.z = farPoint.z - farX.z - farZ.z;

		// get box max accounting for far points only
		box.fMax.x = farPoint.x + farX.x + farZ.x;
		box.fMax.y = farPoint.y + farX.y + farZ.y;
		box.fMax.z = farPoint.z + farX.z + farZ.z;

		// now account for near points
		box.fMin.setMin(fTCW.p);
		box.fMax.setMax(fTCW.p);
	}	


   //---------------------------------------------------------------------------
   // transform & project an array of packed vertices

   int Camera::transformProject( Int32 nRaw,
                                    PackedVertex const *pRaw,
                                    Point3F const & scale,
                                    Point3F const & origin,
                                    TransformedVertex *pDest ) const
      {
      AssertFatal( pRaw  != 0, 
         "TS::Camera::transformProject: invalid source pointer." );
      AssertFatal( nRaw  > 0, 
         "TS::Camera::transformProject: invalid count." );
      AssertFatal( pDest  != 0, 
         "TS::Camera::transformProject: invalid destination pointer." );
      int and = ClipMask; 
      int or = 0;

      fptrTransformProjectPrepare(fTOC, fTOS, &fVs, &fVc, fInvNearDist);
      for( int i = 0; i < nRaw; i++, pDest++, pRaw++ )
      {
         Vertex raw;
         raw.set( *pRaw, scale, origin );

         int ret = fptrTransformProject(&raw.fPoint, pDest);
         and &= ret;
         or |= ret;
      }
      if( and )
         return ClipNoneVis;
      else
         return or;
      }


   //---------------------------------------------------------------------------
   // transform & project an array of vertices:

   int Camera::transformProject( Int32 nRaw,
                                    Point3F const *pRaw,
                                    TransformedVertex *pDest ) const
   {
      AssertFatal( pRaw  != 0, 
         "TS::Camera::transformProject: invalid source pointer." );
      AssertFatal( nRaw  > 0, 
         "TS::Camera::transformProject: invalid count." );
      AssertFatal( pDest  != 0, 
         "TS::Camera::transformProject: invalid destination pointer." );
      int and = ClipMask;
      int or = 0;

      fptrTransformProjectPrepare(fTOC, fTOS, &fVs, &fVc, fInvNearDist);
      for( int i = 0; i < nRaw; i++, pDest++, pRaw++ )
      {
         int ret = fptrTransformProject(pRaw, pDest);
         and &= ret;
         or |= ret;
      }
      if( and )
         return ClipNoneVis;
      else
         return or;
   }

   void Camera::project( TransformedVertex *pVert ) const
      {
      AssertFatal( pVert  != 0, 
         "TS::Camera::TransformProject2: invalid destination pointer." );
      double invW = 1.0f / pVert->fTransformedPoint.w;
      
      pVert->fPoint.x = float(pVert->fTransformedPoint.x * invW * fVs.x + fVc.x);
      pVert->fPoint.y = float(pVert->fTransformedPoint.y * invW * fVs.y + fVc.y);
      pVert->fPoint.z = float(invW);

      pVert->fStatus |= TransformedVertex::Projected;
      }

   //---------------------------------------------------------------------------
   // PerspectiveCamera methods
   //---------------------------------------------------------------------------

   void PerspectiveCamera::buildProjectionMatrix()
      {
      type = PerspectiveCameraType;
      // Build camera to screen matrix
      
      // with:
      //    N is near clip distance, also projection plane
      //    F is far clip distance

      // shear and scale to produce regular parallelpiped view volume:
      // shear (and swap y & z, to give z-depth, y-up):
      // 1        0        0
      // -Wcx/N   -Wcy/N   1
      // 0        1        0

      // scale:
      // 1/Wsx    0        0
      // 0        1/Wsy    0
      // 0        0        1


      // Do first part of perspective projection with:
      // 1        0        0              0
      // 0        1        0              0
      // 0        0        1/(N(1-N/F))   1/N
      // 0        0        -1/(1-N/F)     0

      // combined (shear * scale * perspective):
      // 1/Wsx          0              0              0
      // -Wcx/(Wsx*N)   -Wcy/(Wsy*N)   1/(N(1-N/F))   1/N
      // 0              1/Wsy          0              0
      // 0              0              -1/(1-N/F)     0

      // to finish we will need to do:
      // Xs = (x / w) * Vsx + Vcx
      // Ys = (y / w) * Vsx + Vcx
      // Zs = z / w

	   // world viewport to screen viewport scale.
      fTCS.identity();
      fTCS.m[0][0] = 1.0f / fWs.x;
      fTCS.m[2][1] = 1.0f / fWs.y;
      fTCS.m[1][0] = -fWc.x / (fWs.x * fNearDist);
      fTCS.m[1][1] = -fWc.y / (fWs.y * fNearDist);
      fTCS.m[1][2] = 1.0f / (fNearDist * (1.0f - fNearDist / fFarDist) );
      fTCS.m[2][2] = 0.0f;
      fTCS.p.z = -1.0f / (1.0f - fNearDist / fFarDist);
      fTCS.flags = RMat3F::Matrix_HasRotation | RMat3F::Matrix_HasScale | 
         RMat3F::Matrix_HasTranslation;
      //fvTCS.set(0, fInvNearDist, 0, 0);
      }

   //---------------------------------------------------------------------------
   // transform into clipping space:

   void PerspectiveCamera::transform( Point3F const &raw, TransformedVertex *pDest ) const
      {
      AssertFatal( pDest  != 0, 
         "TS::PerspectiveCamera::transform: invalid destination pointer." );
      m_mul( raw, getTOS(), (Point3F *) &pDest->fTransformedPoint );
      // w = (camera space y) / NearDist
      TMat3F const & toc = getTOC();
      pDest->fTransformedPoint.w = (raw.x * toc.m[0][1] + 
                  raw.y * toc.m[1][1] + 
                  raw.z * toc.m[2][1] + 
                  toc.p.y) * fInvNearDist;
      getVisibilityFlags(pDest);
      }


   //---------------------------------------------------------------------------

   RealF PerspectiveCamera::projectRadius( RealF dist, RealF radius ) const
      {
		AssertFatal(dist != 0.0f,"TS::PerspectiveCamera::projectRadius: Projecting radius at 0 distance");
      // transform (scale) radius:
      TMat3F const &toc = getTOC();
      if(toc.flags & TMat3F::Matrix_HasScale)
      {
         Point3F row;
         toc.getRow(0, &row );
         RealF sradius = radius * row.len();

         // do projection thing, using radius, all we want is the size
         // what was the fWc.x stuff here for?  doesn't work
         RealF xp = (sradius / fWs.x);// - (dist * fWc.x / (fWs.x * fNearDist));
         return (xp * fNearDist * fVs.x) / dist;
      }
      else
      {
         return (radius * fNearDist * fVs.x) / (fWs.x * dist);
      }
      }

   RealF PerspectiveCamera::projectionDistance( RealF pixel, RealF radius) const
      {
      // transform (scale) radius:
      TMat3F const &toc = getTOC();
      if(toc.flags & TMat3F::Matrix_HasScale)
      {
         Point3F row;
         getTOC().getRow(0, &row );

   		// Returns the distance at which radius projects to pixel size.
   		// What happened to screen center offset used in projectRadius?
         RealF xp = (radius * row.len()) / fWs.x;
   		return (xp * fNearDist * fVs.x) / pixel;
      }
      else
      {
         return (radius * fNearDist * fVs.x) / (fWs.x * pixel);
      }
      }

   RealF PerspectiveCamera::transformProjectRadius( Point3F const &loc,
         RealF radius ) const
      {
      // transform to camera space to get correct depth:
      TMat3F const &toc = getTOC();
      Point3F tloc;
      m_mul( loc, toc, &tloc );
      // transform (scale) radius:
      if(toc.flags & TMat3F::Matrix_HasScale)
      {
         Point3F row;
         getTOC().getRow(0, &row );
         radius *= row.len();
      }

      // do projection thing, using radius:
      // what was the fWc.x stuff here for?  doesn't work
      RealF xp = (radius / fWs.x);// - (tloc.y * fWc.x / (fWs.x * fNearDist));
      if (tloc.y < 0.001f)
         tloc.y = 0.001f;
      RealF wp = tloc.y / fNearDist;

      // all we want is the size:
      return (xp / wp) * fVs.x;
      }
   //---------------------------------------------------------------------------
   // cast a ray!

   #pragma argsused

   void PerspectiveCamera::castRay(Point2I &pixel, Point3F *ray)
      {
      /*
      ray->x = (pixel.x - fScreenCenter.x) / fScreenScale.x;
      ray->z = ( - pixel.y + fScreenCenter.y) / fScreenScale.z;
      ray->y = 1.0f;
      */
      }

   //---------------------------------------------------------------------------
   // OrthographicCamera methods
   //---------------------------------------------------------------------------

   //---------------------------------------------------------------------------
   // Build the projection matrix for an orthographic projection:

   void OrthographicCamera::buildProjectionMatrix()
      {
      type = OrthographicCameraType;
      // Build camera to screen matrix
      
      // with:
      //    N is near clip distance, also projection plane
      //    F is far clip distance

      // translate and scale to produce regular parallelpiped view volume:
      // translate (and swap y & z, to give z-depth, y-up):
      // 1        0        0  0
      // 0        0        1  0
      // 0        1        0  0
      // -Wcx     -Wcy     0  1

      // scale:
      // 1/Wsx    0        0  0
      // 0        1/Wsy    0  0
      // 0        0        1  0
      // 0        0        0  1


      // Do first part of orthographic parallel projection with:
      // 1        0        0              0
      // 0        1        0              0
      // 0        0        1/(F-N)        0
      // 0        0        -N/(F-N)       1/N

      // combined (shear * scale * ortho projection):
      // 1/Wsx       0           0        0
      // 0           0           1/(F-N)  0
      // 0           1/Wsy       0        0
      // -Wcx/Wsx    -Wcy/Wsy    -N/(F-N) 1/N

      // to finish we will need to do:
      // Xs = (x / w) * Vsx + Vcx
      // Ys = (y / w) * Vsx + Vcx
      // Zs = z / w

	   // world viewport to screen viewport scale.
      fTCS.identity();
      fTCS.m[1][1] = 0.0f;
      fTCS.m[2][2] = 0.0f;
      fTCS.m[0][0] = 1.0f / fWs.x;
      fTCS.m[2][1] = 1.0f / fWs.y;
      fTCS.p.x = -fWc.x / fWs.x;
      fTCS.p.y = -fWc.y / fWs.y;
      fTCS.m[1][2] = 1.0f / (fFarDist - fNearDist);
      fTCS.p.z = - fNearDist / (fFarDist - fNearDist);

      fTCS.flags = RMat3F::Matrix_HasRotation | RMat3F::Matrix_HasScale | 
         RMat3F::Matrix_HasTranslation;
      }

   //---------------------------------------------------------------------------
   // transform into clipping space:

   void OrthographicCamera::transform( Point3F const &raw, TransformedVertex *pDest ) const
      {
      AssertFatal( pDest  != 0, 
         "TS::OrthographicCamera::transform: invalid destination pointer." );
      m_mul( raw, getTOS(), (Point3F *) &pDest->fTransformedPoint );
      pDest->fTransformedPoint.w = fInvNearDist;
      TMat3F const & toc = getTOC();
      float scale = raw.x * toc.m[0][1] + 
                    raw.y * toc.m[1][1] + 
                    raw.z * toc.m[2][1] + 
                    toc.p.y;       
      pDest->fTransformedPoint.x *= scale; 
      pDest->fTransformedPoint.y *= scale; 
      pDest->fTransformedPoint.z *= scale; 
      pDest->fTransformedPoint.w *= scale; 

      getVisibilityFlags(pDest);
      }

   //---------------------------------------------------------------------------

   #pragma argsused

   RealF OrthographicCamera::projectRadius( RealF ,RealF radius ) const
      {
      // transform (scale) radius:
      Point3F row;
      getTOC().getRow(0, &row );
      RealF sradius = radius * row.len();

      // projected radius is independent of location:
      //return ((sradius / fWs.x) - (fWc.x / fWs.x)) * fNearDist * fVs.x;
      return (sradius / fWs.x) * fNearDist * fVs.x;
      }

   RealF OrthographicCamera::projectionDistance( RealF pixel, RealF radius) const
      {
		// All distances the same, so just return the projection plane.
		//return fNearDist;
      if(radius * fNearDist * fVs.x / fWs.x > pixel)
         return fFarDist;
      else
         return fNearDist;
      }

   RealF OrthographicCamera::transformProjectRadius( Point3F const &,
         RealF radius ) const
      {
      // transform (scale) radius:
      Point3F row;
      getTOC().getRow(0, &row );
      RealF sradius = radius * row.len();

      // projected radius is independent of location:
      //return ((sradius / fWs.x) - (fWc.x / fWs.x)) * fNearDist * fVs.x;
      return (sradius / fWs.x) * fNearDist * fVs.x;
      }

   void OrthographicCamera::getViewconeBox( Box3F & box, float fd ) const
   {
		// use fd for far dist, unless fd<0, then use camera's far distance
		if (fd<0.0f)
			fd = fFarDist;

      // FIXME:
      // all right... we're assuming that the ortho cam is pointing
      // straight down

      Point3F minPos(fTCW.p.x + fWorldViewport.upperL.x, fTCW.p.y + fWorldViewport.lowerR.y, fTCW.p.z - fd);
      Point3F maxPos(fTCW.p.x + fWorldViewport.lowerR.x, fTCW.p.y + fWorldViewport.upperL.y, fTCW.p.z);

		// now account for near points
		box(&minPos, &maxPos);
	}	

   //---------------------------------------------------------------------------
};  // namespace TS
