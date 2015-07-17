//================================================================
//   
// $Workfile:   m_mat.cpp  $
// $Revision:   1.5  $
// $Version$
//	
// DESCRIPTION:
//    RMat and TMat classes implementation.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#include "ml.h"

//------------------------------------------------------
// EulerF implementation:

extern "C" {
	void __cdecl m_RMat3F_set( RMat3F *, const EulerF * );
}

DLLAPI EulerF& EulerF::set( RMat3F const &mat )
{
   if( mat.flags & RMat3F::Matrix_HasRotation )
   {
      float sinx = mat.m[1][2];
      x = m_asin(sinx);
      if( IsEqual(sinx, 1.0f) || IsEqual(sinx, -1.0f) )
      {
         y = (float) 0.0;
         z = (float) m_atan(mat.m[0][0], mat.m[0][1]);
      }
      else
      {
         y = (float) m_atan(mat.m[2][2], -mat.m[0][2]);
         z = (float) m_atan(mat.m[1][1], -mat.m[1][0]);
      }
   }
   else
      set();
   return *this;
}

DLLAPI RMat3F * EulerF::makeMatrix( RMat3F * mat ) const
{
   m_RMat3F_set( mat, this );
   return mat;
}

//------------------------------------------------------
// EulerD implementation:
#if 0
DLLAPI EulerD::RMat3 * EulerD::makeMatrix( RMat3 * mat ) const
   {
   int num_axis;
   if( x )
      num_axis = 3;
   else if( y )
      num_axis = 2;
   else if( z )
      num_axis = 1;
   else
      num_axis = 0;

   mat->flags |= RMat3::Matrix_HasRotation;
   mat->flags &= ~RMat3::Matrix_HasScale;

   switch( num_axis )
      {
      case 0:
         mat->identity();
         break;
      case 1:
         //  cos(z)   sin(z)   0
         //  -sin(z)  cos(z)   0
         //  0        0        1
         m_sincos( z, &mat->m[0][1], &mat->m[0][0] );
         mat->m[1][0] = - mat->m[0][1];
         mat->m[1][1] = mat->m[0][0];
         mat->m[0][2] = mat->m[1][2] = mat->m[2][0] = mat->m[2][1] = Real(0.0);
         mat->m[2][2] = Real(1.0);
         break;
      case 2:
      case 3:
         {
         // the matrix looks like this:
         //  r1 - (r4 * sin(x))     r2 + (r3 * sin(x))   -cos(x) * sin(y)
         //  -cos(x) * sin(z)       cos(x) * cos(z)      sin(x)
         //  r3 + (r2 * sin(x))     r4 - (r1 * sin(x))   cos(x) * cos(y)
         //
         // where:
         //  r1 = cos(y) * cos(z)
         //  r2 = cos(y) * sin(z)
         //  r3 = sin(y) * cos(z)
         //  r4 = sin(y) * sin(z)
         Real cx,sx;
         m_sincos( x, &sx, &cx );
         Real cy,sy;
         m_sincos( y, &sy, &cy );
         Real cz,sz;
         m_sincos( z, &sz, &cz );
         Real r1 = cy * cz;
         Real r2 = cy * sz;
         Real r3 = sy * cz;
         Real r4 = sy * sz;
         mat->m[0][0] = r1 - (r4 * sx);
         mat->m[0][1] = r2 + (r3 * sx);
         mat->m[0][2] = -cx * sy;
         mat->m[1][0] = -cx * sz;
         mat->m[1][1] = cx * cz;
         mat->m[1][2] = sx;
         mat->m[2][0] = r3 + (r2 * sx);
         mat->m[2][1] = r4 - (r1 * sx);
         mat->m[2][2] = cx * cy;
         }
         break;
      }
   return mat;
}
#endif