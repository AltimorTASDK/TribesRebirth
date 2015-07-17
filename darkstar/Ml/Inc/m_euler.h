//================================================================
//   
//	$Workfile:   m_euler.h  $
//	$Revision:   1.2  $
// $Version: 1.6.2 $
//	
// DESCRIPTION:
//    Euler angle Classes.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _M_EULER_H_
#define _M_EULER_H_

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class EulerF
{
public:
   float x,y,z;

	EulerF() {};
	EulerF( float _x, float _y, float _z );
	EulerF( RMat3F const & mat );
	EulerF( EulerF const &e );
	
	EulerF& set( float _x = 0, float _y = 0, float _z = 0 );
	EulerF& set( EulerF const &e );
	
	DLLAPI EulerF& set( RMat3F const &mat );
	DLLAPI RMat3F * makeMatrix( RMat3F * mat ) const;

	EulerF& operator+=(const EulerF& c);
	EulerF& operator-=(const EulerF& c);

	EulerF& operator*=(const float c);
	EulerF& operator/=(const float c);
	
	EulerF operator+(const EulerF& c) const;
	EulerF operator-(const EulerF& c) const;

	EulerF operator*(const float c) const;
	EulerF operator/(const float c) const;
	
	int operator ==( const EulerF& c ) const;
	int operator !=( const EulerF& c ) const;
	
	void reduce();
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

//------------------------------------------------------

// EulerF implementation:

inline EulerF& EulerF::set( float _x, float _y, float _z)
{
	x = _x; y = _y; z = _z;
	return *this;
}

inline EulerF& EulerF::set( EulerF const &e)
{
	x = e.x; y = e.y; z = e.z;
	return *this;
}

inline EulerF::EulerF( float _x, float _y, float _z)
{
	set(_x,_y,_z);
}

inline EulerF::EulerF( EulerF const &e )
{
	set(e);
}

inline EulerF& EulerF::operator+=(const EulerF& c)
{
	x += c.x; 
   y += c.y; 
   z += c.z;
	return *this;
}	

inline EulerF& EulerF::operator-=(const EulerF& c)
{
	x -= c.x; 
   y -= c.y; 
   z -= c.z;
	return *this;
}	

inline EulerF& EulerF::operator*=(const float c)
{
	x *= c; 
   y *= c; 
   z *= c;
	return *this;
}	

inline EulerF& EulerF::operator/=(const float c)
{
	x /= c; 
   y /= c; 
   z /= c;
	return *this;
}	

inline EulerF EulerF::operator+(const EulerF& c) const
{
   return EulerF (x + c.x, y + c.y, z + c.z);
}	

inline EulerF EulerF::operator-(const EulerF& c) const
{
   return EulerF (x - c.x, y - c.y, z - c.z);
}	

inline EulerF EulerF::operator*(const float c) const
{
   return EulerF (x * c, y * c, z * c);
}	

inline EulerF EulerF::operator/(const float c) const
{
   return EulerF (x / c, y / c, z / c);
}	

inline int EulerF::operator ==( const EulerF& c ) const
{ 
   return IsEqual(x, c.x) && IsEqual(y, c.y) && IsEqual(z, c.z);
}

inline int EulerF::operator !=( const EulerF& c ) const
{
   return !IsEqual(x, c.x) || !IsEqual(y, c.y) || !IsEqual(z, c.z);
}

inline void EulerF::reduce()
{
   x = m_reduce( x );
   y = m_reduce( y );
   z = m_reduce( z );
}

#endif
