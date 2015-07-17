//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "ml.h"

extern "C" {
	void __cdecl m_QuatF_set( QuatF *, const EulerF * );
}

QuatF& QuatF::set( EulerF const & e )
{
#if 1
   m_QuatF_set(this, &e);
#else
   Real cx, sx;
   Real cy, sy;
   Real cz, sz;
   Real cycz, sysz, sycz, cysz;


   m_sincos( -e.x * Real(0.5), &sx, &cx );
   m_sincos( -e.y * Real(0.5), &sy, &cy );
   m_sincos( -e.z * Real(0.5), &sz, &cz );

   // Qyaw(z)   = [ (0, 0, sin z/2), cos z/2 ]
   // Qpitch(x) = [ (sin x/2, 0, 0), cos x/2 ]
   // Qroll(y)  = [ (0, sin y/2, 0), cos y/2 ]
   // this = Qresult = Qyaw*Qpitch*Qroll  ZXY
   // 
   // The code that folows is a simplification of:
   //    roll*=pitch;
   //    roll*=yaw;
   //    *this = roll;

      cycz = cy*cz;   
      sysz = sy*sz;
      sycz = sy*cz;
      cysz = cy*sz;
      w = cycz*cx + sysz*sx;
      x = cycz*sx + sysz*cx;
      y = sycz*cx - cysz*sx;
      z = cysz*cx - sycz*sx;
#endif
   return *this;
}

AngAxisF & AngAxisF::set( QuatF const & q )
{
   angle = m_acos( q.w ) * float(2.0);
   float sinHalfAngle = m_sin( angle * float(0.5) );
	axis.set( q.x / sinHalfAngle, q.y / sinHalfAngle, q.z / sinHalfAngle );
   return *this;
}

AngAxisF & AngAxisF::set( RMat3F const & mat )
{
   QuatF q( mat );
   set( q );
   return *this;
}

RMat3F * AngAxisF::makeMatrix( RMat3F * mat ) const
{
   QuatF q( *this );
   return q.makeMatrix( mat );
}

QuatF& QuatF::operator *=( QuatF const & c )
{
   QuatF prod;
   prod.w = w*c.w - x*c.x - y*c.y - z*c.z;
   prod.x = w*c.x + x*c.w + y*c.z - z*c.y;
   prod.y = w*c.y - x*c.z + y*c.w + z*c.x;
   prod.z = w*c.z + x*c.y - y*c.x + z*c.w;
   *this = prod;
   return (*this);
}


QuatF& QuatF::operator /=( QuatF const & c )
{
   QuatF temp = c;
   return ( (*this) *= temp.inverse() );   
}


QuatF& QuatF::operator +=( QuatF const & c )
{
   x += c.x;   
   y += c.y;   
   z += c.z;   
   w += c.w;   
   return *this;
}

QuatF& QuatF::operator -=( QuatF const & c )
{
   x -= c.x;   
   y -= c.y;   
   z -= c.z;   
   w -= c.w;   
   return *this;
}


QuatF& QuatF::operator *=( float a )
{
   x *= a;   
   y *= a;   
   z *= a;   
   w *= a;   
   return *this;
}

QuatF& QuatF::operator /=( float a )
{
   x /= a;   
   y /= a;   
   z /= a;   
   w /= a;   
   return *this;
}


QuatF& QuatF::square()
{
   float t = w*2.0f;
   w = (w*w) - (x*x + y*y + z*z);
   x *= t;
   y *= t;
   z *= t;
   return *this;
}


QuatF& QuatF::inverse()
{
   float magnitude = w*w + x*x + y*y + z*z;
   float invMagnitude;
   if( magnitude == 1.0f )    // special case unit quaternion
   {
      x = -x;
      y = -y;
      z = -z;
   }
   else                       // else scale
   {
      if( magnitude == 0.0f )
         invMagnitude = 1.0f;
      else
         invMagnitude = 1.0f / magnitude;
      w *= invMagnitude;
      x *= -invMagnitude;
      y *= -invMagnitude;
      z *= -invMagnitude;
   }                        
	return *this;
}

QuatF& QuatF::set( AngAxisF const & a )
{
   float sinHalfAngle, cosHalfAngle;
   m_sincos( a.angle * float(0.5), &sinHalfAngle, &cosHalfAngle );
   x = a.axis.x * sinHalfAngle;
   y = a.axis.y * sinHalfAngle;
   z = a.axis.z * sinHalfAngle;
   w = cosHalfAngle;
   return *this;
}

QuatF& QuatF::set( RMat3F const & m )
{
   if( m.flags & RMat3F::Matrix_HasRotation )
   {
      w = m_sqrt( m.trace() + float(1.0) ) * float(0.5);
      float c = (float(4.0) * w);
      x = (m.m[2][1] - m.m[1][2]) / c;
      y = (m.m[0][2] - m.m[2][0]) / c;
      z = (m.m[1][0] - m.m[0][1]) / c;
   }
   else
      identity();
   return *this;
}


QuatF & QuatF::normalize()
{
   float l = m_sqrt( x*x + y*y + z*z + w*w );
   if( l == float(0.0) )
      identity();
   else
   {
      x /= l;
      y /= l;
      z /= l;
      w /= l;
   }
   return *this;
}

RMat3F * QuatF::makeMatrix( RMat3F * mat ) const
{
   if( IsIdentity() )
      mat->identity();
   else
   {
      float xs = x * float(2.0);
      float ys = y * float(2.0);
      float zs = z * float(2.0);
      float wx = w * xs;
      float wy = w * ys;
      float wz = w * zs;
      float xx = x * xs;
      float xy = x * ys;
      float xz = x * zs;
      float yy = y * ys;
      float yz = y * zs;
      float zz = z * zs;
      mat->m[0][0] = float(1.0) - (yy + zz);
      mat->m[0][1] = xy - wz;
      mat->m[0][2] = xz + wy;
      mat->m[1][0] = xy + wz;
      mat->m[1][1] = float(1.0) - (xx + zz);
      mat->m[1][2] = yz - wx;
      mat->m[2][0] = xz - wy;
      mat->m[2][1] = yz + wx;
      mat->m[2][2] = float(1.0) - (xx + yy);
      mat->flags &= ~RMat3F::Matrix_HasScale;
      mat->flags |= RMat3F::Matrix_HasRotation;
   }
   return mat;
}

QuatF & QuatF::slerp( QuatF const & q, float t )
{
   return interpolate( *this, q, t );
}

QuatF & QuatF::interpolate( QuatF const & q1, 
   QuatF const & q2, float t )
{
   //-----------------------------------
   // Calculate the cosine of the angle:

   double cosOmega = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
    
   //-----------------------------------
   // adjust signs if necessary:

   float sign2;
   if ( cosOmega < 0.0 ) 
   {
      cosOmega = -cosOmega;
      sign2 = -1.0f;
   } 
   else  
      sign2 = 1.0f;

   //-----------------------------------
   // calculate interpolating coeffs:

   double scale1, scale2;
   if ( (1.0 - cosOmega) > 0.00001 ) 
   { 
      // standard case
      double omega = m_acos(cosOmega);
      double sinOmega = m_sin(omega);
      scale1 = m_sin((1.0 - t) * omega) / sinOmega;
      scale2 = sign2 * m_sin(t * omega) / sinOmega;
   } 
   else 
   { 
      // if quats are very close, just do linear interpolation
      scale1 = 1.0 - t;
      scale2 = sign2 * t;
   }


   //-----------------------------------
   // actually do the interpolation:

   x = float(scale1 * q1.x + scale2 * q2.x);
   y = float(scale1 * q1.y + scale2 * q2.y);
   z = float(scale1 * q1.z + scale2 * q2.z);
   w = float(scale1 * q1.w + scale2 * q2.w);

   return *this;
}

#if 0
//------------------------------------------------------------------------------
QuatD& QuatD::set( Euler const & e )
{
   Real cx, sx;
   Real cy, sy;
   Real cz, sz;
   Real cycz, sysz, sycz, cysz;

   m_sincos( -e.x * Real(0.5), &sx, &cx );
   m_sincos( -e.y * Real(0.5), &sy, &cy );
   m_sincos( -e.z * Real(0.5), &sz, &cz );

   // Qyaw(z)   = [ (0, 0, sin z/2), cos z/2 ]
   // Qpitch(x) = [ (sin x/2, 0, 0), cos x/2 ]
   // Qroll(y)  = [ (0, sin y/2, 0), cos y/2 ]
   // this = Qresult = Qyaw*Qpitch*Qroll  ZXY
   // 
   // The code that folows is a simplification of:
   //    roll*=pitch;
   //    roll*=yaw;
   //    *this = roll;

      cycz = cy*cz;   
      sysz = sy*sz;
      sycz = sy*cz;
      cysz = cy*sz;
      w = cycz*cx + sysz*sx;
      x = cycz*sx + sysz*cx;
      y = sycz*cx - cysz*sx;
      z = cysz*cx - sycz*sx;

   return *this;
}
#endif
