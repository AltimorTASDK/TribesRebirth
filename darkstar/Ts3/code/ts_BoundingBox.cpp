//---------------------------------------------------------------------------

//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
// $Version$
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#include "ts_BoundingBox.h"
#include <ts_RenderContext.h>

namespace TS
{
   //---------------------------------------------------------------------------
   // Collision methods
   //---------------------------------------------------------------------------

   Bool BoundingBox::collide( const TMat3F & tmat, 
      const SphereF & sphere ) const
      {
      // move the sphere's center point into our space:
      Point3F center;
      m_mul( sphere.center, tmat, &center );

      // see if the spheres intersect:
   	if( m_distf( center, fSphere.center ) > (fSphere.radius + sphere.radius) )
   		return FALSE;
   	else
   		return TRUE;
      }

   Bool BoundingBox::collide( const TMat3F & tmat, 
      const Box3F & box ) const
      {
      // get corner of the box in our space:
      Point3F tmin;
      m_mul( box.fMin, tmat, &tmin );

      // get box dimensions:
      RealF xe = box.len_x();
      RealF ye = box.len_y();
      RealF ze = box.len_z();

      // get box edge vectors in our space:
      Point3F vi,vj,vk;
      vi.set( xe * tmat.m[0][0], xe * tmat.m[0][1], xe * tmat.m[0][2] );
      vj.set( ye * tmat.m[1][0], ye * tmat.m[1][1], ye * tmat.m[1][2] );
      vk.set( ze * tmat.m[2][0], ze * tmat.m[2][1], ze * tmat.m[2][2] );

      // construct eight corners of box, in our space:
      Point3F corners[8];

      corners[0] = tmin;

      corners[1] = tmin;          // i
      corners[1] += vi;

      corners[2] = tmin;          // j
      corners[2] += vj;

      corners[3] = tmin;          // k
      corners[3] += vk;

      corners[4] = corners[1];    // i+j
      corners[4] += vj;

      corners[5] = corners[1];    // i+k
      corners[5] += vk;

      corners[6] = corners[2];    // j+k
      corners[6] += vk;

      corners[7] = corners[4];    // i+j+k
      corners[7] += vk;

      int code[8];
      for( int i = 0; i < 8; i++ )
          {
          code[i] = 0;
          if( corners[i].x < fBox.fMin.x )
              code[i] += 1;
          else if( corners[i].x > fBox.fMax.x )
              code[i] += 2;
          if( corners[i].y < fBox.fMin.y )
              code[i] += 4;
          else if( corners[i].y > fBox.fMax.y )
              code[i] += 8;
          if( corners[i].z < fBox.fMin.z )
              code[i] += 16;
          else if( corners[i].z > fBox.fMax.z )
              code[i] += 32;
          // is the corner inside our box?
          if( !code[i] )
              return TRUE;
          }
      static int edges[][2] = { {0,1},{0,2},{0,3}, 
                                {4,1},{4,2},{4,7}, 
                                {5,1},{5,3},{5,7}, 
                                {6,2},{6,3},{6,7}};
      static float t, x, y, z;
      for( int e = 0; e < 12; e++ )
         {
         // trivial reject the edge?
         if( code[edges[e][0]] & code[edges[e][1]] )
            continue;
         // do the intersection:
         if( code[edges[e][0]] & 1 )  // to left?
            {
            t = (float)(fBox.fMin.x - corners[edges[e][0]].x) /
                      (float)(corners[edges[e][1]].x - corners[edges[e][0]].x);
            y = corners[edges[e][0]].y + t * 
               (corners[edges[e][1]].y - corners[edges[e][0]].y);
            if( y <= fBox.fMax.y && y >= fBox.fMin.y )
               {
               z = corners[edges[e][0]].z + t * 
                  (corners[edges[e][1]].z - corners[edges[e][0]].z);
               if( z <= fBox.fMax.z && z >= fBox.fMin.z )
                  return TRUE;
               }
            }
         else if( code[edges[e][0]] & 2 )  // to right?
            {
            t = (float)(fBox.fMax.x - corners[edges[e][0]].x) /
                      (float)(corners[edges[e][1]].x - corners[edges[e][0]].x);
            y = corners[edges[e][0]].y + t * 
               (corners[edges[e][1]].y - corners[edges[e][0]].y);
            if( y <= fBox.fMax.y && y >= fBox.fMin.y )
               {
               z = corners[edges[e][0]].z + t * 
                  (corners[edges[e][1]].z - corners[edges[e][0]].z);
               if( z <= fBox.fMax.z && z >= fBox.fMin.z )
                  return TRUE;
               }
            }

         if( code[edges[e][0]] & 4 )  // to front?
            {
            t = (float)(fBox.fMin.y - corners[edges[e][0]].y) /
                      (float)(corners[edges[e][1]].y - corners[edges[e][0]].y);
            x = corners[edges[e][0]].x + t * 
               (corners[edges[e][1]].x - corners[edges[e][0]].x);
            if( x <= fBox.fMax.x && x >= fBox.fMin.x )
               {
               z = corners[edges[e][0]].z + t * 
                  (corners[edges[e][1]].z - corners[edges[e][0]].z);
               if( z <= fBox.fMax.z && z >= fBox.fMin.z )
                  return TRUE;
               }
            }
         else if( code[edges[e][0]] & 8 )  // to back?
            {
            t = (float)(fBox.fMax.y - corners[edges[e][0]].y) /
                      (float)(corners[edges[e][1]].y - corners[edges[e][0]].y);
            x = corners[edges[e][0]].x + t * 
               (corners[edges[e][1]].x - corners[edges[e][0]].x);
            if( x <= fBox.fMax.x && x >= fBox.fMin.x )
               {
               z = corners[edges[e][0]].z + t * 
                  (corners[edges[e][1]].z - corners[edges[e][0]].z);
               if( z <= fBox.fMax.z && z >= fBox.fMin.z )
                  return TRUE;
               }
            }

         if( code[edges[e][0]] & 16 )  // to bottom?
            {
            t = (float)(fBox.fMin.z - corners[edges[e][0]].z) /
                      (float)(corners[edges[e][1]].z - corners[edges[e][0]].z);
            x = corners[edges[e][0]].x + t * 
               (corners[edges[e][1]].x - corners[edges[e][0]].x);
            if( x <= fBox.fMax.x && x >= fBox.fMin.x )
               {
               y = corners[edges[e][0]].y + t * 
                  (corners[edges[e][1]].y - corners[edges[e][0]].y);
               if( y <= fBox.fMax.y && y >= fBox.fMin.y )
                  return TRUE;
               }
            }
         else if( code[edges[e][0]] & 32 )  // to top?
            {
            t = (float)(fBox.fMax.z - corners[edges[e][0]].z) /
                      (float)(corners[edges[e][1]].z - corners[edges[e][0]].z);
            x = corners[edges[e][0]].x + t * 
               (corners[edges[e][1]].x - corners[edges[e][0]].x);
            if( x <= fBox.fMax.x && x >= fBox.fMin.x )
               {
               y = corners[edges[e][0]].y + t * 
                  (corners[edges[e][1]].y - corners[edges[e][0]].y);
               if( y <= fBox.fMax.y && y >= fBox.fMin.y )
                  return TRUE;
               }
            }
         }
      return FALSE;
      }

   Bool BoundingBox::collide( const TMat3F & tmat, 
      const LineSeg3F & lseg ) const
      {
   	// get start point in our space:
   	Point3F corners[2];

   	m_mul( lseg.start, tmat, &corners[0] );
   	m_mul( lseg.end, tmat, &corners[1] );

   	int code[2];
   	for( int i = 0; i < 2; i++ )
   	   {
   		code[i] = 0;
   		if( corners[i].x < fBox.fMin.x )
   		   code[i] += 1;
   		else if( corners[i].x > fBox.fMax.x )
   		   code[i] += 2;
   		if( corners[i].y < fBox.fMin.y )
   		   code[i] += 4;
   		else if( corners[i].y > fBox.fMax.y )
   		   code[i] += 8;
   		if( corners[i].z < fBox.fMin.z )
   		   code[i] += 16;
   		else if( corners[i].z > fBox.fMax.z )
   		   code[i] += 32;
   		
   		// is the corner inside our box?
   		if( !code[i] )
   		   return TRUE;
   	   }

   	static float t, x, y, z; // RJK: Why is this static?
   	
   	// trivial reject the edge?
   	if( code[0] & code[1] )
   		return FALSE;

   	// do the intersection:
      if( code[0] & 1 )  // to left?
   	   {
   		t = (float)(fBox.fMin.x - corners[0].x) /
   					(float)(corners[1].x - corners[0].x);
   		y = corners[0].y + t * (corners[1].y - corners[0].y);
   		if( y <= fBox.fMax.y && y >= fBox.fMin.y )
            {
   		   z = corners[0].z + t * (corners[1].z - corners[0].z);
   		   if( z <= fBox.fMax.z && z >= fBox.fMin.z )
         		return TRUE;
            }
   		}
   	 else if( code[0] & 2 )  // to right?
   		{
   		t = (float)(fBox.fMax.x - corners[0].x) /
   					(float)(corners[1].x - corners[0].x);
   		y = corners[0].y + t * (corners[1].y - corners[0].y);
   		if( y <= fBox.fMax.y && y >= fBox.fMin.y )
            {
      		z = corners[0].z + t * (corners[1].z - corners[0].z);
   	   	if( z <= fBox.fMax.z && z >= fBox.fMin.z )
         		return TRUE;
            }
   		}

   	 if( code[0] & 4 )  // to front?
   		{
   		t = (float)(fBox.fMin.y - corners[0].y) /
   					(float)(corners[1].y - corners[0].y);
   		x = corners[0].x + t * (corners[1].x - corners[0].x);
   		if( x <= fBox.fMax.x && x >= fBox.fMin.x )
            {
      		z = corners[0].z + t * (corners[1].z - corners[0].z);
   	   	if( z <= fBox.fMax.z && z >= fBox.fMin.z )
   		      return TRUE;
            }
   		}
   	 else if( code[0] & 8 )  // to back?
   		{
   		t = (float)(fBox.fMax.y - corners[0].y) /
   					(float)(corners[1].y - corners[0].y);
   		x = corners[0].x + t * (corners[1].x - corners[0].x);
   		if( x <= fBox.fMax.x && x >= fBox.fMin.x )
            {
   		   z = corners[0].z + t * (corners[1].z - corners[0].z);
   		   if( z <= fBox.fMax.z && z >= fBox.fMin.z )
   		      return TRUE;
            }
   		}

   	 if( code[0] & 16 )  // to bottom?
   		{
   		t = (float)(fBox.fMin.z - corners[0].z) /
   					(float)(corners[1].z - corners[0].z);
   		x = corners[0].x + t * (corners[1].x - corners[0].x);
   		if( x <= fBox.fMax.x && x >= fBox.fMin.x )
            {
   		   y = corners[0].y + t * (corners[1].y - corners[0].y);
   		   if( y <= fBox.fMax.y && y >= fBox.fMin.y )
   		      return TRUE;
            }
   		}
   	 else if( code[0] & 32 )  // to top?
   		{
   		t = (float)(fBox.fMax.z - corners[0].z) /
   					(float)(corners[1].z - corners[0].z);
   		x = corners[0].x + t * (corners[1].x - corners[0].x);
   		if( x <= fBox.fMax.x && x >= fBox.fMin.x )
            {
   		   y = corners[0].y + t * (corners[1].y - corners[0].y);
   		   if( y <= fBox.fMax.y && y >= fBox.fMin.y )
   		      return TRUE;
            }
   		}

   	return FALSE;
      }

   Bool BoundingBox::collide( const TMat3F & tmat, 
      const BoundingBox * otherPart ) const
      {
      if( collide( tmat, otherPart->getSphere() ) && 
          collide( tmat, otherPart->getBox() ) )
          return TRUE;
      else
          return FALSE;
      }

   //---------------------------------------------------------------------------
   // Render
   //---------------------------------------------------------------------------

   #pragma argsused
  
   void BoundingBox::render( RenderContext & rc, int color ) const
   {
      // only draw the box if it is entirely visible:
      if( rc.getCamera()->testVisibility( *this ) == ClipAllVis )
   	   {
         // get corner of the box in camera space:
         Point3F tmin;
         const TMat3F & tmat = rc.getCamera()->getTOC();
         m_mul( fBox.fMin, tmat, &tmin );

         // get box dimensions:
         RealF xe = fBox.len_x();
         RealF ye = fBox.len_y();
         RealF ze = fBox.len_z();

         // get box edge vectors in camera space:
         Point3F vi,vj,vk;
         vi.set( xe * tmat.m[0][0], xe * tmat.m[0][1], xe * tmat.m[0][2] );
         vj.set( ye * tmat.m[1][0], ye * tmat.m[1][1], ye * tmat.m[1][2] );
         vk.set( ze * tmat.m[2][0], ze * tmat.m[2][1], ze * tmat.m[2][2] );

         // construct eight corners of box, in camera space:
         Point3F corners[8];

         corners[0] = tmin;

         corners[1] = tmin;          // i
         corners[1] += vi;

         corners[2] = tmin;          // j
         corners[2] += vj;

         corners[3] = tmin;          // k
         corners[3] += vk;

         corners[4] = corners[1];    // i+j
         corners[4] += vj;

         corners[5] = corners[1];    // i+k
         corners[5] += vk;

         corners[6] = corners[2];    // j+k
         corners[6] += vk;

         corners[7] = corners[4];    // i+j+k
         corners[7] += vk;

         // project into screen coords:
         Point4F pcorners[8];
//         rc.getCamera()->project( 8, corners, pcorners );
//         static int edges[][2] = { {0,1},{0,2},{0,3}, 
//                                   {4,1},{4,2},{4,7}, 
//                                   {5,1},{5,3},{5,7}, 
//                                   {6,2},{6,3},{6,7}};
//          // draw the lines!
//         for( int e = 0; e < 12; e++ )
//            {
//            // can't draw with float points???
//            //rc.getSurface()->drawLine2d( &pcorners[edges[e][0]], 
//            //   &pcorners[edges[e][1]], color );
//            }
   	   }
   }

   //---------------------------------------------------------------------------
}; // namespace TS
