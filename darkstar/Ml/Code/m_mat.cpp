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

#include <m_mat.h>
#include <m_euler.h>

//---------------------------------------------------------------------------
// RMat3T
//---------------------------------------------------------------------------

template<class _Real>
void RMat3T<_Real>::pitch( _Real angle )
   {
	RMat3 rmat, temp;
	rmat.set(Euler(angle, Real(0.0), Real(0.0)));
	m_mul(*this, rmat, &temp);
	*this = temp;
   }

template<class _Real>
void RMat3T<_Real>::yaw( _Real angle )
   {
	RMat3 rmat, temp;
	rmat.set(Euler(Real(0.0), Real(0.0), angle));
	m_mul(*this, rmat, &temp);
	*this = temp;
   }

template<class _Real>
void RMat3T<_Real>::roll( _Real angle )
   {
	RMat3 rmat, temp;
	rmat.set(Euler(Real(0.0), angle, Real(0.0)));
	m_mul(*this, rmat, &temp);
	*this = temp;
   }

//---------------------------------------------------------------------------
// RMat3F
//---------------------------------------------------------------------------

extern "C" {
	void __cdecl m_RMat3F_set( RMat3F *, const EulerF * );
}

RMat3F& RMat3F::set( const Euler& e ) 
{ 
#if 1
   m_RMat3F_set( this, &e );
#else
   int num_axis;
   if( e.x )
      num_axis = 3;
   else if( e.y )
      num_axis = 2;
   else if( e.z )
      num_axis = 1;
   else
      num_axis = 0;

   switch( num_axis )
      {
      case 0:
         identity();
         break;
      case 1:
         //  cos(z)   sin(z)   0
         //  -sin(z)  cos(z)   0
         //  0        0        1
         m_sincos( e.z, &m[0][1], &m[0][0] );
         m[1][0] = - m[0][1];
         m[1][1] = m[0][0];
         m[0][2] = m[1][2] = m[2][0] = m[2][1] = Real(0.0);
         m[2][2] = Real(1.0);
         flags &= ~Matrix_HasScale;
         flags |= Matrix_HasRotation;
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
         m_sincos( e.x, &sx, &cx );
         Real cy,sy;
         m_sincos( e.y, &sy, &cy );
         Real cz,sz;
         m_sincos( e.z, &sz, &cz );
         Real r1 = cy * cz;
         Real r2 = cy * sz;
         Real r3 = sy * cz;
         Real r4 = sy * sz;
         m[0][0] = r1 - (r4 * sx);
         m[0][1] = r2 + (r3 * sx);
         m[0][2] = -cx * sy;
         m[1][0] = -cx * sz;
         m[1][1] = cx * cz;
         m[1][2] = sx;
         m[2][0] = r3 + (r2 * sx);
         m[2][1] = r4 - (r1 * sx);
         m[2][2] = cx * cy;
         flags &= ~Matrix_HasScale;
         flags |= Matrix_HasRotation;
         }
         break;
      }
#endif
   return *this;
}

//---------------------------------------------------------------------------
// RMat3D
//---------------------------------------------------------------------------

RMat3D& RMat3D::set( const Euler& e ) 
{ 
   int num_axis;
   if( e.x )
      num_axis = 3;
   else if( e.y )
      num_axis = 2;
   else if( e.z )
      num_axis = 1;
   else
      num_axis = 0;

   switch( num_axis )
      {
      case 0:
         identity();
         break;
      case 1:
         //  cos(z)   sin(z)   0
         //  -sin(z)  cos(z)   0
         //  0        0        1
         m_sincos( e.z, &m[0][1], &m[0][0] );
         m[1][0] = - m[0][1];
         m[1][1] = m[0][0];
         m[0][2] = m[1][2] = m[2][0] = m[2][1] = Real(0.0);
         m[2][2] = Real(1.0);

         flags &= ~Matrix_HasScale;
         flags |= Matrix_HasRotation;
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
         m_sincos( e.x, &sx, &cx );
         Real cy,sy;
         m_sincos( e.y, &sy, &cy );
         Real cz,sz;
         m_sincos( e.z, &sz, &cz );
         Real r1 = cy * cz;
         Real r2 = cy * sz;
         Real r3 = sy * cz;
         Real r4 = sy * sz;
         m[0][0] = r1 - (r4 * sx);
         m[0][1] = r2 + (r3 * sx);
         m[0][2] = -cx * sy;
         m[1][0] = -cx * sz;
         m[1][1] = cx * cz;
         m[1][2] = sx;
         m[2][0] = r3 + (r2 * sx);
         m[2][1] = r4 - (r1 * sx);
         m[2][2] = cx * cy;

         flags &= ~Matrix_HasScale;
         flags |= Matrix_HasRotation;
         }
         break;
      }
   return *this;
}

//---------------------------------------------------------------------------
