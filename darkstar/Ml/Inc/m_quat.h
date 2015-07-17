//================================================================
//   
//	$Workfile:   m_quat.h  $
//	$Revision:   1.2  $
// $Version: 1.6.2 $
//	
// DESCRIPTION:
//    Quaternion Classes.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _M_QUAT_H_
#define _M_QUAT_H_

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class RMat3F;

//------------------------------------------------------
// rotation about an arbitrary axis through the origin:

class DLLAPI AngAxisF
{
public:
   Point3F axis;
   float  angle;

	AngAxisF();
	AngAxisF( Point3F const & _axis, float _angle );
	AngAxisF( RMat3F const &m );
	AngAxisF( QuatF const &q );

   AngAxisF& set( Point3F const & _axis, float _angle );
   AngAxisF& set( RMat3F const & m );
   AngAxisF& set( QuatF const & q );

   int operator ==( AngAxisF const & c ) const;
	int operator !=( AngAxisF const & c ) const;

   RMat3F * makeMatrix( RMat3F * mat ) const;
};

//------------------------------------------------------
// unit quaternion class:

class DLLAPI QuatF
{
public:
   float  x,y,z,w;

	QuatF();
	QuatF( float _x, float _y, float _z, float w );
   QuatF( RMat3F const & m );
   QuatF( AngAxisF const & a );
   QuatF( EulerF const & e );

   QuatF& set( float _x, float _y, float _z, float _w );
   QuatF& set( RMat3F const & m );
   QuatF& set( AngAxisF const & m );
   QuatF& set( EulerF const & e );

   int operator ==( QuatF const & c ) const;
	int operator !=( QuatF const & c ) const;
   QuatF& operator *=( QuatF const & c );
   QuatF& operator /=( QuatF const & c );
   QuatF& operator +=( QuatF const & c );
   QuatF& operator -=( QuatF const & c );
   QuatF& operator *=( float a );
   QuatF& operator /=( float a );

   QuatF& square();
   QuatF& neg();
   float  dot( QuatF const &q ) const;

   RMat3F* makeMatrix( RMat3F * mat ) const;
   QuatF& normalize();
   QuatF& inverse();
	int    IsIdentity() const;
   QuatF& identity();
   QuatF& slerp( QuatF const & q, float t );
   QuatF& interpolate( QuatF const & q1, QuatF const & q2, float t );
   float  angleBetween( QuatF const & q );
};


//------------------------------------------------------
// AngAxisF implementation:

inline AngAxisF::AngAxisF()
{
}

inline AngAxisF::AngAxisF( Point3F const & _axis, float _angle )
{
	set(_axis,_angle);
}

inline AngAxisF::AngAxisF( RMat3F const & mat )
{
	set(mat);
}

inline AngAxisF::AngAxisF( QuatF const & quat )
{
	set(quat);
}

inline AngAxisF& AngAxisF::set( Point3F const & _axis, float _angle )
{
   axis = _axis;
   angle = _angle;
	return *this;
}

inline int AngAxisF::operator ==( AngAxisF const & c ) const
{ 
   return IsEqual(angle, c.angle) && (axis == c.axis);
}

inline int AngAxisF::operator !=( AngAxisF const & c ) const
{
   return !IsEqual(angle, c.angle) || (axis != c.axis);
}

//------------------------------------------------------
// quaternion implementation:

inline QuatF::QuatF()
{
}

inline QuatF::QuatF( float _x, float _y, float _z, float _w )
{
	set( _x, _y, _z, _w );
}

inline QuatF::QuatF( AngAxisF const & a )
{
	set( a );
}

inline QuatF::QuatF( EulerF const & e )
{
	set(e);
}

inline QuatF& QuatF::set( float _x, float _y, float _z, float _w )
{
   x = _x;
   y = _y;
   z = _z;
   w = _w;
	return *this;
}

inline int QuatF::operator ==( QuatF const & c ) const
{ 
   QuatF a = *this;
   QuatF b = c;
   a.normalize();
   b.normalize();
   b.inverse();
   a *= b;
   return a.IsIdentity();
}

inline int QuatF::IsIdentity() const
{ 
   return IsZero( x ) && IsZero( y ) && IsZero( z );
}

inline QuatF& QuatF::identity()
{ 
   x = 0.0f;
   y = 0.0f;
   z = 0.0f;
   w = 1.0f;
   return *this;
}

inline int QuatF::operator !=( QuatF const & c ) const
{
   return ! operator==( c );
}

inline QuatF::QuatF( RMat3F const & m )
{
	set( m );
}


inline QuatF& QuatF::neg() 
{ 
   x = -x;
   y = -y;
   z = -z;
   w = -w;
   return *this;
}

inline float QuatF::dot( QuatF const &q ) const
{
   return (w*q.w + x*q.x + y*q.y + z*q.z);
}   


inline float QuatF::angleBetween( QuatF const & q )
{
   // angle between to quaternions
   return m_acos(x * q.x + y * q.y + z * q.z + w * q.w);
}   



//---------------------------------------------------------------------------
// TQuatF classes:

class TQuatF : public QuatF
{
public:

   enum 
   {
      Matrix_HasRotation = 1,
      Matrix_HasTranslation = 2,
      Matrix_HasScale = 4,
   };

	Point3F p;
   Int32 flags;

	TQuatF();
	TQuatF( Bool ident );
	TQuatF( EulerF const & e, Point3F const & p );
	TQuatF( AngAxisF const & aa, Point3F const & p );
	TQuatF( QuatF const & q, Point3F const & p );

   TQuatF& set( EulerF const & euler, Point3F const & p );
   TQuatF& set( AngAxisF const & aa, Point3F const & p );
   TQuatF& set( QuatF const & quat, Point3F const & p );

	TQuatF& inverse( void );
	TQuatF& identity( void );
};

//---------------------------------------------------------------------------
//
// - TQuatF
//

inline TQuatF::TQuatF()
{
   // Beware: no initialization is done!
}

inline TQuatF::TQuatF( Bool ident )
{
   if( ident )
      identity();
}

inline TQuatF::TQuatF( EulerF const & euler, Point3F const & p )
{
   set( euler, p );
}

inline TQuatF::TQuatF( AngAxisF const & aa, Point3F const & p )
{
   set( aa, p );
}

inline TQuatF::TQuatF( QuatF const & quat, Point3F const & p )
{
   set( quat, p );
}

inline TQuatF& TQuatF::set( EulerF const & e, Point3F const & t )
{
	p = t;
	QuatF::set( e );
   flags |= Matrix_HasTranslation;
	return *this;
}

inline TQuatF& TQuatF::set( AngAxisF const & aa, Point3F const & t )
{
	p = t;
	QuatF::set( aa );
   flags |= Matrix_HasTranslation;
	return *this;
}

inline TQuatF& TQuatF::set( QuatF const & q, Point3F const & t )
{
	p = t;
	QuatF::set( q.x, q.y, q.z, q.w );
   flags |= Matrix_HasTranslation;
	return *this;
}

inline TQuatF& TQuatF::inverse( void )
{
	QuatF::inverse();
   if( flags & Matrix_HasTranslation )
   {
   	p.neg();
	   Point3F p2 = p;
      m_mul( p2, *((QuatF*)this), &p );
   }
	return *this;
}

inline TQuatF& TQuatF::identity( void )
{
	QuatF::identity();
   p.set();
   flags &= ~Matrix_HasTranslation;
	return *this;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif
