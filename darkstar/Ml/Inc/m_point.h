//----------------------------------------------------------------------------
//   
// $Workfile:   m_point.h  $
// $Revision:   1.19  $
// $Version$
//	
// DESCRIPTION:
//    Point classes: 2D, 3D, and 4D.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//----------------------------------------------------------------------------
#include <streams.h>
#include <m_base.h>


#ifndef _M_POINT_H_
#define _M_POINT_H_

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif


class Point2I 
{
public:
	Int32	x, y;

	//
	Point2I();
	explicit Point2I( const Int32 _x, const Int32 _y );

	// access operators
	Int32& operator[](int i) { return (&x)[i]; }     
	const Int32& operator[](int i) const { return (&x)[i]; }  

	// set point to average of two points
	Point2I& average( const Point2I& a, const Point2I& b );
	Point2I& neg( void );							// mirror about origin

	Point2I& setMin( const Point2I& a );
	Point2I& setMax( const Point2I& a );

	Point2I& set( const Int32 _x = 0, const Int32 _y = 0 );
	Bool isZero() const;

   int operator ==( const Point2I& c ) const;
	int operator !=( const Point2I& c ) const;

   Point2I& operator()  (const Point2I &in_pt);
   Point2I& operator()  (Int32 _x, Int32 _y);

	Point2I& operator +=( const Int32 c );
	Point2I& operator +=( const Point2I& c );

	Point2I& operator -=( const Int32 c );
	Point2I& operator -=( const Point2I& c );

	Point2I& operator <<=( const Int32 c );
	Point2I& operator >>=( const Int32 c );

	Point2I& operator *=( const Int32 c );		// scale the vector
	Point2I& operator *=( const Point2I& c );
	Point2I& operator /=( const Int32 c );
	Point2I& operator /=( const Point2I& c );

   //-------------------------------------- Temporary constructing versions
	Point2I operator +( const Int32 c ) const;
	Point2I operator +( const Point2I& c ) const;

	Point2I operator -( const Int32 c ) const;
	Point2I operator -( const Point2I& c ) const;

	Point2I operator <<( const Int32 c ) const;
	Point2I operator >>( const Int32 c ) const;

	Point2I operator *( const Int32 c ) const;		// scale the vector
	Point2I operator *( const Point2I& c ) const;
	Point2I operator /( const Int32 c ) const;
	Point2I operator /( const Point2I& c ) const;

	Point2I& normalize( Int32 newLen );
	Point2I& normalize( Int32 newLen, Int32 len );
	Point2I& normalizef( Int32 newLen );
	Point2I& normalizef( Int32 newLen, Int32 len );

	Int32 len( void ) const;
	Int32 lenf( void ) const;
	Int32 lenvf( void ) const;
	Bool check_lenvf( const Int32 l ) const;

   Point2I& interpolate(Point2I const &p1,Point2I const &p2,RealF t1);

#ifndef M_NOFILEIO
   Bool     read( StreamIO & sio );
   Bool     write( StreamIO & sio );
#endif
};



class Point2S
{
public:
   Int16   x, y;

   Point2S()  {}
   explicit Point2S( const Int16 _x, const Int16 _y );
   explicit Point2S( const Point2I & c ) {  x = c.x; y = c.y;  }

   // access operators
   Int16& operator[](int i) { return (&x)[i]; }     
   const Int16& operator[](int i) const { return (&x)[i]; }  

   // set point to average of two points
   Point2S & average( const Point2S & a, const Point2S & b );
   Point2S & neg( void );                     // mirror about origin

   Point2S & setMin( const Point2S & a );
   Point2S & setMax( const Point2S & a );

   Point2S & set( const Int16 _x = 0, const Int16 _y = 0 );
   
   Bool isZero() const { return !x && !y;  }
   int operator ==( const Point2S & c ) const    {  return (x == c.x) && (y == c.y);      }
   int operator !=( const Point2S & c ) const    {  return (x != c.x) || (y != c.y);      }

   Point2S & operator()  (const Point2S & in_pt);
   Point2S & operator()  (Int16 _x, Int16 _y);

   Point2S & operator +=( const Int16 c );
   Point2S & operator +=( const Point2S & c );

   Point2S & operator -=( const Int16 c );
   Point2S & operator -=( const Point2S & c );

   Point2S & operator <<=( const Int32 c );
   Point2S & operator >>=( const Int32 c );

   Point2S & operator *=( const Int16 c );      // scale the vector
   Point2S & operator *=( const Point2S & c );
   Point2S & operator /=( const Int16 c );
   Point2S & operator /=( const Point2S & c );
   
   void operator=( const Point2I & c )  { x = c.x; y = c.y; }
};



//----------------------------------------------------------------------------

class Point3I
{
public:

   Int32	x,y,z;

	//
	Point3I();
	explicit Point3I( const Int32 _x, const Int32 _y, const Int32 _z );

	// access operators
	Int32& operator[](int i) { return (&x)[i]; }     
	const Int32& operator[](int i) const { return (&x)[i]; }  

	// set point to average of two points
	Point3I& average( const Point3I& a, const Point3I& b );
	
	Point3I& neg( void );							// mirror about origin

	Point3I& setMin( const Point3I& a );
	Point3I& setMax( const Point3I& a );
            
   Point3I& set( const Int32 _x = 0, const Int32 _y = 0, const Int32 _z = 0 );
   Bool isZero() const;

	int operator ==( const Point3I& c ) const;
	int operator !=( const Point3I& c ) const;

	Point3I& operator +=( const Int32 c );
	Point3I& operator +=( const Point3I& c );
	Point3I& operator -=( const Int32 c );
	Point3I& operator -=( const Point3I& c );

	Point3I& operator <<=( const Int32 c );
	Point3I& operator >>=( const Int32 c );

	Point3I& operator *=( const Int32 c );		// scale the vector
	Point3I& operator *=( const Point3I& c );  // scale the vector
	Point3I& operator /=( const Int32 c );     // scale the vector
	Point3I& operator /=( const Point3I& c );  // scale the vector

   //-------------------------------------- Temporary constructing versions
	Point3I operator +( const Int32 c ) const;
	Point3I operator +( const Point3I& c ) const;
	Point3I operator -( const Int32 c ) const;
	Point3I operator -( const Point3I& c ) const;

	Point3I operator <<( const Int32 c ) const;
	Point3I operator >>( const Int32 c ) const;

	Point3I operator *( const Int32 c ) const;		// scale the vector
	Point3I operator *( const Point3I& c ) const;  // scale the vector
	Point3I operator /( const Int32 c ) const;     // scale the vector
	Point3I operator /( const Point3I& c ) const;  // scale the vector

   Point3I& normalize( Int32 newLen );
	Point3I& normalize( Int32 newLen, Int32 len ); 
	Point3I& normalizef( Int32 newLen );
	Point3I& normalizef( Int32 newLen, Int32 len ); 

	Int32 len( void ) const;
	Int32 lenf( void ) const;
	Int32 lenvf( void ) const;
	Bool check_lenvf( const Int32 l ) const;

   Point3I& interpolate(Point3I const &p1,Point3I const &p2,RealF t1);

#ifndef M_NOFILEIO
   Bool     read( StreamIO & sio );
   Bool     write( StreamIO & sio );
#endif
};

//----------------------------------------------------------------------------
// Point2I methods
//----------------------------------------------------------------------------

inline Point2I::Point2I() 
{
}

inline Point2I::Point2I( const Int32 _x, const Int32 _y )
{
	x = _x;
	y = _y;
}

inline Point2I& Point2I::average( const Point2I& a, 
            const Point2I& b )
{
   // set point to average of two points
	x = (a.x + b.x) / 2;
	y = (a.y + b.y) / 2;
	return *this;
}

inline Point2I& Point2I::neg( void )							// mirror about origin
{
	x = -x;
	y = -y;
	return *this;
}

inline Point2I& Point2I::setMin( const Point2I& a )
{
	if( x > a.x ) x = a.x;
	if( y > a.y ) y = a.y;
	return *this;
}

inline Point2I& Point2I::setMax( const Point2I& a )
{
	if( x < a.x ) x = a.x;
	if( y < a.y ) y = a.y;
	return *this;
}

inline Point2I& Point2I::set( const Int32 _x, const Int32 _y )
{
	x = _x;
	y = _y;
	return *this;
}

inline Bool Point2I::isZero() const
{
   return !x && !y;
}

inline int Point2I::operator ==( const Point2I& c ) const
{ 
   return (x == c.x) && (y == c.y);
}

inline int Point2I::operator !=( const Point2I& c ) const
{
   return (x != c.x) || (y != c.y);
}

inline Point2I& Point2I::operator() (const Point2I &pt)
{
   return (*this = pt);
}

inline Point2I& Point2I::operator() (Int32 _x, Int32 _y)
{
   x = _x;
   y = _y;
   return *this;
}

inline Point2I& Point2I::operator +=( const Int32 c )
{
	x += c;
	y += c;
	return *this;
}

inline Point2I& Point2I::operator +=( const Point2I& c )
{
	x += c.x;
	y += c.y;
	return *this;
}

inline Point2I& Point2I::operator -=( const Int32 c )
{
	x -= c;
	y -= c;
	return *this;
}

inline Point2I& Point2I::operator -=( const Point2I& c )
{
	x -= c.x;
	y -= c.y;
	return *this;
}

inline Point2I& Point2I::operator *=( const Int32 c )
{ 
	x *= c; 
	y *= c; 
	return *this; 
}

inline Point2I& Point2I::operator *=( const Point2I& c )
{ 
	x *= c.x; 
	y *= c.y; 
	return *this; 
}

inline Point2I& Point2I::operator /=( const Int32 c )
{ 
	x /= c; 
	y /= c;
	return *this; 
}

inline Point2I& Point2I::operator /=( const Point2I& c )
{ 
	x /= c.x; 
	y /= c.y;
	return *this; 
}

inline Point2I Point2I::operator +( const Int32 c ) const
{
   return Point2I(x + c, y + c);
}

inline Point2I Point2I::operator +( const Point2I& c ) const
{
   return Point2I(x + c.x, y + c.y);
}

inline Point2I Point2I::operator -( const Int32 c ) const
{
   return Point2I(x - c, y - c);
}

inline Point2I Point2I::operator -( const Point2I& c ) const
{
   return Point2I(x - c.x, y - c.y);
}

inline Point2I Point2I::operator *( const Int32 c ) const
{ 
   return Point2I(x * c, y * c);
}

inline Point2I Point2I::operator *( const Point2I& c ) const
{ 
   return Point2I(x * c.x, y * c.y);
}

inline Point2I Point2I::operator /( const Int32 c ) const
{ 
   return Point2I(x / c, y / c);
}

inline Point2I Point2I::operator /( const Point2I& c ) const
{ 
   return Point2I(x / c.x, y / c.y);
}

inline Int32 Point2I::len( void ) const
{ 
	return m_hyp( x, y ); 
}

inline Int32 Point2I::lenf( void ) const
{
	return m_hypf( x, y ); 
}

inline Int32 Point2I::lenvf( void ) const
{
	return m_hypvf( x, y ); 
}

inline Bool Point2I::check_lenvf( const Int32 l ) const
{
	return m_hypvf( x, y ) <= l; 
}

inline Point2I& Point2I::normalize(Int32 newLen)
{
   return normalize( newLen, len() );
}

inline Point2I& Point2I::normalizef(Int32 newLen)
{
   return normalize( newLen, lenf() );
}

inline Point2I& Point2I::interpolate( Point2I const &p1, Point2I const &p2, RealF t1 )
{
   Int32 t2 = Int32(1) - t1;
   x = p1.x * t2 + p2.x * t1;
   y = p1.y * t2 + p2.y * t1;
   return *this;
}

#ifndef M_NOFILEIO
inline Bool Point2I::read( StreamIO & sio )
{
   return sio.read( &x ) && sio.read( &y );
}

inline Bool Point2I::write( StreamIO & sio )
{
   return sio.write( x ) && sio.write( y );
}

#endif

//----------------------------------------------------------------------------
// Point2S methods
//    Short (16 bit) version of Point2I.  
//----------------------------------------------------------------------------

inline Point2S::Point2S( const Int16 _x, const Int16 _y )
{
	x = _x;
	y = _y;
}

inline Point2S& Point2S::average( const Point2S& a, 
            const Point2S& b )
{
   // set point to average of two points
	x = (a.x + b.x) / 2;
	y = (a.y + b.y) / 2;
	return *this;
}

inline Point2S& Point2S::neg( void )							// mirror about origin
{
	x = -x;
	y = -y;
	return *this;
}

inline Point2S& Point2S::setMin( const Point2S& a )
{
	if( x > a.x ) x = a.x;
	if( y > a.y ) y = a.y;
	return *this;
}

inline Point2S& Point2S::setMax( const Point2S& a )
{
	if( x < a.x ) x = a.x;
	if( y < a.y ) y = a.y;
	return *this;
}

inline Point2S& Point2S::set( const Int16 _x, const Int16 _y )
{
	x = _x;
	y = _y;
	return *this;
}


inline Point2S & Point2S::operator() (const Point2S & pt)
{
   return (*this = pt);
}

inline Point2S & Point2S::operator() (Int16 _x, Int16 _y)
{
   x = _x;
   y = _y;
   return *this;
}

inline Point2S & Point2S::operator +=( const Int16 c )
{
	x += c;
	y += c;
	return *this;
}

inline Point2S & Point2S::operator +=( const Point2S & c )
{
	x += c.x;
	y += c.y;
	return *this;
}

inline Point2S & Point2S::operator -=( const Int16 c )
{
	x -= c;
	y -= c;
	return *this;
}

inline Point2S & Point2S::operator -=( const Point2S & c )
{
	x -= c.x;
	y -= c.y;
	return *this;
}

inline Point2S & Point2S::operator *=( const Int16 c )
{ 
	x *= c; 
	y *= c; 
	return *this; 
}

inline Point2S & Point2S::operator *=( const Point2S & c )
{ 
	x *= c.x; 
	y *= c.y; 
	return *this; 
}

inline Point2S & Point2S::operator /=( const Int16 c )
{ 
	x /= c; 
	y /= c;
	return *this; 
}

inline Point2S & Point2S::operator /=( const Point2S & c )
{ 
	x /= c.x; 
	y /= c.y;
	return *this; 
}

inline Point2S& Point2S::operator <<=( const Int32 c )
{ 
	x <<= c; 
	y <<= c; 
	return *this; 
}

inline Point2S& Point2S::operator >>=( const Int32 c )
{ 
	x >>= c;
	y >>= c;
	return *this; 
}

//----------------------------------------------------------------------------
// Point3I methods
//----------------------------------------------------------------------------

inline Point3I::Point3I() 
{
}

inline Point3I::Point3I( const Int32 _x, const Int32 _y, const Int32 _z )
{ 
   x = _x;
   y = _y;
	z = _z; 
}

inline Point3I& Point3I::average( const Point3I& a, 
            const Point3I& b )
{ 
   // set point to average of two points
	x = (a.x + b.x) / 2; 
	y = (a.y + b.y) / 2; 
	z = (a.z + b.z) / 2; 
	return *this;
}

inline Point3I& Point3I::neg( void )
{ 
   // mirror about origin
	x = -x;
	y = -y;
	z = -z;
   return *this;
}

inline Point3I& Point3I::setMin( const Point3I& a )
{
	if( x > a.x ) x = a.x; 
	if( y > a.y ) y = a.y; 
	if( z > a.z ) z = a.z; 
   return *this;
}

inline Point3I& Point3I::setMax( const Point3I& a )
{ 
	if( x < a.x ) x = a.x;
	if( y < a.y ) y = a.y;
	if( z < a.z ) z = a.z;
   return *this;
}

inline Point3I& Point3I::set( const Int32 _x, const Int32 _y, const Int32 _z )
{
	x = _x;
	y = _y;
	z = _z;
   return *this;
}

inline Bool Point3I::isZero() const
{
   return IsZero(x) && IsZero(y) && IsZero(z);
}

inline int Point3I::operator ==( const Point3I& c ) const
{ 
   return IsEqual(x, c.x) && IsEqual(y, c.y) && IsEqual(z, c.z);
}

inline int Point3I::operator !=( const Point3I& c ) const
{ 
   return !IsEqual(x, c.x) || !IsEqual(y, c.y) || !IsEqual(z, c.z);
}

inline Point3I& Point3I::operator +=( const Int32 c )
{ 
	x += c;
	y += c;
	z += c;
   return *this;
}

inline Point3I& Point3I::operator +=( const Point3I& c )
{ 
	x += c.x; 
	y += c.y; 
	z += c.z; 
   return *this;
}

inline Point3I& Point3I::operator -=( const Int32 c )
{ 
	x -= c;
	y -= c;
	z -= c;
   return *this;
}

inline Point3I& Point3I::operator -=( const Point3I& c )
{ 
	x -= c.x; 
	y -= c.y; 
	z -= c.z; 
   return *this;
}

inline Point3I& Point3I::operator *=( const Int32 c )		// scale the vector
{ 
	x *= c; 
	y *= c; 
	z *= c; 
   return *this;
}

inline Point3I& Point3I::operator *=( const Point3I& c )		// scale the vector
{ 
	x *= c.x; 
	y *= c.y; 
	z *= c.z; 
	return *this; 
}

inline Point3I& Point3I::operator /=( const Int32 c )
{ 
	x /= c;
	y /= c;
	z /= c;
   return *this;
}

inline Point3I& Point3I::operator /=( const Point3I& c )		// scale the vector
{ 
	x /= c.x; 
	y /= c.y; 
	z /= c.z; 
	return *this; 
}

inline Point3I Point3I::operator +( const Int32 c ) const
{ 
   return Point3I(x + c, y + c, z + c);
}

inline Point3I Point3I::operator +( const Point3I& c ) const
{ 
   return Point3I(x + c.x, y + c.y, z + c.z);
}

inline Point3I Point3I::operator -( const Int32 c ) const
{ 
   return Point3I(x - c, y - c, z - c);
}

inline Point3I Point3I::operator -( const Point3I& c ) const
{ 
   return Point3I(x - c.x, y - c.y, z - c.z);
}

inline Point3I Point3I::operator *( const Int32 c ) const
{ 
   return Point3I(x * c, y * c, z * c);
}

inline Point3I Point3I::operator *( const Point3I& c ) const
{ 
   return Point3I(x * c.x, y * c.y, z * c.z);
}

inline Point3I Point3I::operator /( const Int32 c ) const
{ 
   return Point3I(x / c, y / c, z / c);
}

inline Point3I Point3I::operator /( const Point3I& c ) const
{ 
   return Point3I(x / c.x, y / c.y, z / c.z);
}

inline Int32 Point3I::len( void ) const
{ 
	return m_hyp( x, y, z );
}

inline Int32 Point3I::lenf( void ) const
{
	return m_hypf( x, y, z );
}

inline Int32 Point3I::lenvf( void ) const
{
	return m_hypvf( x, y, z );
}

inline Bool Point3I::check_lenvf( const Int32 l ) const
{
	return m_hypvf( x, y, z ) <= l; 
}

inline Point3I& Point3I::normalize(Int32 newLen)
{
   return normalize( newLen, len() );
}

inline Point3I& Point3I::normalizef(Int32 newLen)
{
   return normalize( newLen, lenf() );
}

inline Point3I& Point3I::interpolate( Point3I const &p1, Point3I const &p2, RealF t1 )
{
   RealF t2 = 1.0f - t1;
   x = p1.x * t2 + p2.x * t1;
   y = p1.y * t2 + p2.y * t1;
   z = p1.z * t2 + p2.z * t1;
   return *this;
}

#ifndef M_NOFILEIO
inline Bool Point3I::read( StreamIO & sio )
{
   return sio.read( &x ) && sio.read( &y ) && sio.read( &z );
}

inline Bool Point3I::write( StreamIO & sio )
{
   return sio.write( x ) && sio.write( y ) && sio.write( z );
}

#endif

//----------------------------------------------------------------------------
// Int32 specializations:
//----------------------------------------------------------------------------

inline Point2I& Point2I::operator <<=( const Int32 c )
{ 
	x <<= c; 
	y <<= c; 
	return *this; 
}

inline Point2I& Point2I::operator >>=( const Int32 c )
{ 
	x >>= c;
	y >>= c;
	return *this; 
}

inline Point3I& Point3I::operator <<=( const Int32 c )
{ 
	x <<= c; 
	y <<= c; 
	z <<= c; 
	return *this; 
}

inline Point3I& Point3I::operator >>=( const Int32 c )
{ 
	x >>= c;
	y >>= c;
	z >>= c;
	return *this; 
}

inline Point2I Point2I::operator <<( const Int32 c ) const
{ 
   return Point2I(x << c, y << c);
}

inline Point2I Point2I::operator >>( const Int32 c ) const
{ 
   return Point2I(x >> c, y >> c);
}

inline Point3I Point3I::operator <<( const Int32 c ) const
{ 
   return Point3I(x << c, y << c, z << c);
}

inline Point3I Point3I::operator >>( const Int32 c ) const
{ 
   return Point3I(x >> c, y >> c, z >> c);
}

inline Point2I& Point2I::normalize(Int32 newLen, Int32 len)
{
	x = m_muldiv( x, newLen, len );
	y = m_muldiv( y, newLen, len );
	return *this;
}

inline Point2I& Point2I::normalizef(Int32 newLen, Int32 len)
{
	Int32 fix = ((1 << 11) * newLen) / len;
	x = (x * fix) >> 11;
	y = (y * fix) >> 11;
	return *this;
}

inline Point3I& Point3I::normalize(Int32 newLen, Int32 len)
{
	x = m_muldiv( x, newLen, len );
	y = m_muldiv( y, newLen, len );
	z = m_muldiv( z, newLen, len );
	return *this;
}

inline Point3I& Point3I::normalizef(Int32 newLen, Int32 len)
{
	Int32 fix = ((1 << 11) * newLen) / len;
	x = (x * fix) >> 11;
	y = (y * fix) >> 11;
	z = (z * fix) >> 11;
	return *this;
}

class Point2F 
{
public:
	float	x, y;

	//
	Point2F();
	explicit Point2F( const float _x, const float _y );

	// access operators
	float& operator[](int i) { return (&x)[i]; }     
	const float& operator[](int i) const { return (&x)[i]; }  

	// set point to average of two points
	Point2F& average( const Point2F& a, const Point2F& b );
	Point2F& neg( void );							// mirror about origin

	Point2F& setMin( const Point2F& a );
	Point2F& setMax( const Point2F& a );

	Point2F& set( const float _x = 0, const float _y = 0 );
	Bool isZero() const;

   int operator ==( const Point2F& c ) const;
	int operator !=( const Point2F& c ) const;

   Point2F& operator()  (const Point2F &in_pt);
   Point2F& operator()  (float _x, float _y);

	Point2F& operator +=( const float c );
	Point2F& operator +=( const Point2F& c );

	Point2F& operator -=( const float c );
	Point2F& operator -=( const Point2F& c );

	Point2F& operator *=( const float c );		// scale the vector
	Point2F& operator *=( const Point2F& c );
	Point2F& operator /=( const float c );
	Point2F& operator /=( const Point2F& c );

	Point2F operator +( const float c ) const;
	Point2F operator +( const Point2F& c ) const;

	Point2F operator -( const float c ) const;
	Point2F operator -( const Point2F& c ) const;

	Point2F operator *( const float c ) const;
	Point2F operator *( const Point2F& c ) const;
	Point2F operator /( const float c ) const;
	Point2F operator /( const Point2F& c ) const;

   Point2F& normalize();
	Point2F& normalize( float newLen );
	Point2F& normalize( float newLen, float len );
	Point2F& normalizef();
	Point2F& normalizef( float newLen );
	Point2F& normalizef( float newLen, float len );

   Point2F operator-() const;

	float len( void ) const;
	float lenf( void ) const;
	float lenvf( void ) const;
	Bool check_lenvf( const float l ) const;

   Point2F& interpolate(Point2F const &p1,Point2F const &p2,RealF t1);

#ifndef M_NOFILEIO
   Bool     read( StreamIO & sio );
   Bool     write( StreamIO & sio );
#endif
};


//----------------------------------------------------------------------------

class Point3F
{
public:
   float	x, y, z;

	//
	Point3F();
	explicit Point3F( const float _x, const float _y, const float _z );

	// access operators
	float& operator[](int i) { return (&x)[i]; }     
	const float& operator[](int i) const { return (&x)[i]; }  

	// set point to average of two points
	Point3F& average( const Point3F& a, const Point3F& b );
	
	Point3F& neg( void );							// mirror about origin

	Point3F& setMin( const Point3F& a );
	Point3F& setMax( const Point3F& a );
            
   Point3F& set( const float _x = 0, const float _y = 0, const float _z = 0 );
   Bool isZero() const;

	int operator ==( const Point3F& c ) const;
	int operator !=( const Point3F& c ) const;

	Point3F& operator +=( const float c );
	Point3F& operator +=( const Point3F& c );
	Point3F& operator -=( const float c );
	Point3F& operator -=( const Point3F& c );

	Point3F& operator *=( const float c );		// scale the vector
	Point3F& operator *=( const Point3F& c );  // scale the vector
	Point3F& operator /=( const float c );     // scale the vector
	Point3F& operator /=( const Point3F& c );  // scale the vector

	Point3F operator +( const float c ) const;
	Point3F operator +( const Point3F& c ) const;
	Point3F operator -( const float c ) const;
	Point3F operator -( const Point3F& c ) const;

	Point3F operator *( const float c ) const;		// scale the vector
	Point3F operator *( const Point3F& c ) const;  // scale the vector
	Point3F operator /( const float c ) const;     // scale the vector
	Point3F operator /( const Point3F& c ) const;  // scale the vector

   Point3F& normalize();
   Point3F& normalize( float newLen );
	Point3F& normalize( float newLen, float len ); 
   Point3F& normalizef();
	Point3F& normalizef( float newLen );
	Point3F& normalizef( float newLen, float len ); 

   Point3F operator-() const;

	float len( void ) const;
	float lenf( void ) const;
	float lenvf( void ) const;
	Bool check_lenvf( const float l ) const;
   float lenSq() const;
   Point3F& interpolate(Point3F const &p1,Point3F const &p2,RealF t1);

   operator Point2F &() const { return *((Point2F *) this); }
   operator Point2F *() const { return ((Point2F *) this); }

#ifndef M_NOFILEIO
   Bool     read( StreamIO & sio );
   Bool     write( StreamIO & sio );
#endif
};


//----------------------------------------------------------------------------

class Point4F
{
public:
   float x, y, z,	w;

	//
	Point4F();
	explicit Point4F( const float _x, const float _y, const float _z, const RealF _w );

   Point4F& set( const float _x = 0, const float _y = 0, const float _z = 0,
      const RealF _w = 0 );

	// access operators
	float& operator[](int i) { return (&x)[i]; }     
	const float& operator[](int i) const { return (&x)[i]; }  

   Point4F& interpolate( Point4F const &p1, Point4F const &p2, 
      RealF t1 );

   operator Point3F &() const { return *((Point3F *) this); }
   operator Point3F *() const { return ((Point3F *) this); }
#ifndef M_NOFILEIO
   Bool     read( StreamIO & sio );
   Bool     write( StreamIO & sio );
#endif
};

//----------------------------------------------------------------------------
// RealF specializations:
//----------------------------------------------------------------------------

extern "C" {
	RealF __cdecl m_Point3F_len( const Point3F * );
	RealF __cdecl m_Point3F_lenf( const Point3F * );
	void  __cdecl m_Point3F_Point3I( const Point3F *, Point3I * );
	void  __cdecl m_Point3F_normalizef( const Point3F * );
}


//----------------------------------------------------------------------------
// Point2F methods
//----------------------------------------------------------------------------

inline Point2F::Point2F() 
{
}

inline Point2F::Point2F( const float _x, const float _y )
{
	x = _x;
	y = _y;
}

inline Point2F& Point2F::average( const Point2F& a, 
            const Point2F& b )
{
   // set point to average of two points
	x = (a.x + b.x) / 2;
	y = (a.y + b.y) / 2;
	return *this;
}

inline Point2F& Point2F::neg( void )							// mirror about origin
{
	x = -x;
	y = -y;
	return *this;
}

inline Point2F
Point2F::operator-() const
{
   return Point2F(-x, -y);
}

inline Point2F& Point2F::setMin( const Point2F& a )
{
	if( x > a.x ) x = a.x;
	if( y > a.y ) y = a.y;
	return *this;
}

inline Point2F& Point2F::setMax( const Point2F& a )
{
	if( x < a.x ) x = a.x;
	if( y < a.y ) y = a.y;
	return *this;
}

inline Point2F& Point2F::set( const float _x, const float _y )
{
	x = _x;
	y = _y;
	return *this;
}

inline Bool Point2F::isZero() const
{
   return IsZero(x) && IsZero(y);
}

inline int Point2F::operator ==( const Point2F& c ) const
{ 
   return IsEqual(x, c.x) && IsEqual(y, c.y);
}

inline int Point2F::operator !=( const Point2F& c ) const
{
   return !IsEqual(x, c.x) || !IsEqual(y, c.y);
}

inline Point2F& Point2F::operator() (const Point2F &pt)
{
   return (*this = pt);
}

inline Point2F& Point2F::operator() (float _x, float _y)
{
   x = _x;
   y = _y;
   return *this;
}

inline Point2F& Point2F::operator +=( const float c )
{
	x += c;
	y += c;
	return *this;
}

inline Point2F& Point2F::operator +=( const Point2F& c )
{
	x += c.x;
	y += c.y;
	return *this;
}

inline Point2F& Point2F::operator -=( const float c )
{
	x -= c;
	y -= c;
	return *this;
}

inline Point2F& Point2F::operator -=( const Point2F& c )
{
	x -= c.x;
	y -= c.y;
	return *this;
}

inline Point2F& Point2F::operator *=( const float c )
{ 
	x *= c; 
	y *= c; 
	return *this; 
}

inline Point2F& Point2F::operator *=( const Point2F& c )
{ 
	x *= c.x; 
	y *= c.y; 
	return *this; 
}

inline Point2F& Point2F::operator /=( const float c )
{ 
	x /= c; 
	y /= c;
	return *this; 
}

inline Point2F& Point2F::operator /=( const Point2F& c )
{ 
	x /= c.x; 
	y /= c.y;
	return *this; 
}

inline Point2F Point2F::operator +( const float c ) const
{
   return Point2F(x + c, y + c);
}

inline Point2F Point2F::operator +( const Point2F& c ) const
{
   return Point2F(x + c.x, y + c.y);
}

inline Point2F Point2F::operator -( const float c ) const
{
   return Point2F(x - c, y - c);
}

inline Point2F Point2F::operator -( const Point2F& c ) const
{
   return Point2F(x - c.x, y - c.y);
}

inline Point2F Point2F::operator *( const float c ) const
{ 
   return Point2F(x * c, y * c);
}

inline Point2F Point2F::operator *( const Point2F& c ) const
{ 
   return Point2F(x * c.x, y * c.y);
}

inline Point2F Point2F::operator /( const float c ) const
{ 
   return Point2F(x / c, y / c);
}

inline Point2F Point2F::operator /( const Point2F& c ) const
{ 
   return Point2F(x / c.x, y / c.y);
}

inline float Point2F::len( void ) const
{ 
	return m_hyp( x, y ); 
}

inline float Point2F::lenf( void ) const
{
	return m_hypf( x, y ); 
}

inline float Point2F::lenvf( void ) const
{
	return m_hypvf( x, y ); 
}

inline Bool Point2F::check_lenvf( const float l ) const
{
	return m_hypvf( x, y ) <= l; 
}

inline Point2F& Point2F::normalize(float newLen)
{
   return normalize( newLen, len() );
}

inline Point2F& Point2F::normalizef(float newLen)
{
   return normalize( newLen, lenf() );
}

inline Point2F& Point2F::normalize()
{
   return normalize( float(1) );
}

inline Point2F& Point2F::normalize(float newLen, float len)
{
	return *this *= newLen / len;
}

inline Point2F& Point2F::normalizef()
{
   return normalizef( float(1.0) );
}

inline Point2F& Point2F::normalizef(float newLen, float len)
{
	return *this *= newLen / len;
}

inline Point2F& Point2F::interpolate( Point2F const &p1, Point2F const &p2, RealF t1 )
{
   float t2 = float(1) - t1;
   x = p1.x * t2 + p2.x * t1;
   y = p1.y * t2 + p2.y * t1;
   return *this;
}

#ifndef M_NOFILEIO
inline Bool Point2F::read( StreamIO & sio )
{
   return sio.read( &x ) && sio.read( &y );
}

inline Bool Point2F::write( StreamIO & sio )
{
   return sio.write( x ) && sio.write( y );
}

#endif

//----------------------------------------------------------------------------
// Point3F methods
//----------------------------------------------------------------------------

inline Point3F::Point3F() 
{
}

inline Point3F::Point3F( const float _x, const float _y, const float _z )
{ 
   x = _x;
   y = _y;
	z = _z; 
}

inline Point3F& Point3F::average( const Point3F& a, 
            const Point3F& b )
{ 
   // set point to average of two points
	x = (a.x + b.x) / 2; 
	y = (a.y + b.y) / 2; 
	z = (a.z + b.z) / 2; 
	return *this;
}

inline Point3F& Point3F::neg( void )
{ 
   // mirror about origin
	x = -x;
	y = -y;
	z = -z;
   return *this;
}

inline Point3F
Point3F::operator-() const
{
   return Point3F(-x, -y, -z);
}

inline Point3F& Point3F::setMin( const Point3F& a )
{
	if( x > a.x ) x = a.x; 
	if( y > a.y ) y = a.y; 
	if( z > a.z ) z = a.z; 
   return *this;
}

inline Point3F& Point3F::setMax( const Point3F& a )
{ 
	if( x < a.x ) x = a.x;
	if( y < a.y ) y = a.y;
	if( z < a.z ) z = a.z;
   return *this;
}

inline Point3F& Point3F::set( const float _x, const float _y, const float _z )
{
	x = _x;
	y = _y;
	z = _z;
   return *this;
}

inline Bool Point3F::isZero() const
{
   return IsZero(x) && IsZero(y) && IsZero(z);
}

inline int Point3F::operator ==( const Point3F& c ) const
{ 
   return IsEqual(x, c.x) && IsEqual(y, c.y) && IsEqual(z, c.z);
}

inline int Point3F::operator !=( const Point3F& c ) const
{ 
   return !IsEqual(x, c.x) || !IsEqual(y, c.y) || !IsEqual(z, c.z);
}

inline Point3F& Point3F::operator +=( const float c )
{ 
	x += c;
	y += c;
	z += c;
   return *this;
}

inline Point3F& Point3F::operator +=( const Point3F& c )
{ 
	x += c.x; 
	y += c.y; 
	z += c.z; 
   return *this;
}

inline Point3F& Point3F::operator -=( const float c )
{ 
	x -= c;
	y -= c;
	z -= c;
   return *this;
}

inline Point3F& Point3F::operator -=( const Point3F& c )
{ 
	x -= c.x; 
	y -= c.y; 
	z -= c.z; 
   return *this;
}

inline Point3F& Point3F::operator *=( const float c )		// scale the vector
{ 
	x *= c; 
	y *= c; 
	z *= c; 
   return *this;
}

inline Point3F& Point3F::operator *=( const Point3F& c )		// scale the vector
{ 
	x *= c.x; 
	y *= c.y; 
	z *= c.z; 
	return *this; 
}

inline Point3F& Point3F::operator /=( const float c )
{ 
	x /= c;
	y /= c;
	z /= c;
   return *this;
}

inline Point3F& Point3F::operator /=( const Point3F& c )		// scale the vector
{ 
	x /= c.x; 
	y /= c.y; 
	z /= c.z; 
	return *this; 
}

inline Point3F Point3F::operator +( const float c ) const
{
   return Point3F(x + c, y + c, z + c);
}

inline Point3F Point3F::operator +( const Point3F& c ) const
{ 
   return Point3F(x + c.x, y + c.y, z + c.z);
}

inline Point3F Point3F::operator -( const float c ) const
{ 
   return Point3F(x - c, y - c, z - c);
}

inline Point3F Point3F::operator -( const Point3F& c ) const
{ 
   return Point3F(x - c.x, y - c.y, z - c.z);
}

inline Point3F Point3F::operator *( const float c )  const		// scale the vector
{ 
   return Point3F(x * c, y * c, z * c);
}

inline Point3F Point3F::operator *( const Point3F& c ) const		// scale the vector
{ 
   return Point3F(x * c.x, y * c.y, z * c.z);
}

inline Point3F Point3F::operator /( const float c ) const
{ 
   return Point3F(x / c, y / c, z / c);
}

inline Point3F Point3F::operator /( const Point3F& c )  const		// scale the vector
{ 
   return Point3F(x / c.x, y / c.y, z / c.z);
}

inline Bool Point3F::check_lenvf( const float l ) const
{
	return m_hypvf( x, y, z ) <= l; 
}

inline Point3F& Point3F::normalize(float newLen)
{
   return normalize( newLen, len() );
}

inline Point3F& Point3F::normalizef(float newLen)
{
   return normalize( newLen, lenf() );
}

inline Point3F& Point3F::normalize()
{
   return normalize( float(1) );
}

inline Point3F& Point3F::normalizef()
{
   m_Point3F_normalizef( this );
   return *this;
}

inline float Point3F::lenvf( void ) const
{
   return m_Point3F_lenf( this );
}

inline float Point3F::lenf( void ) const
{
   return m_Point3F_lenf( this );
}

inline float Point3F::len( void ) const
{
   return m_Point3F_len( this );
}

inline float
Point3F::lenSq() const
{
   return (x*x) + (y*y) + (z*z);
}

inline Point3F& Point3F::normalize(float newLen, float len)
{
	return *this *= newLen / len;
}

inline Point3F& Point3F::normalizef(float newLen, float len)
{
	return *this *= newLen / len;
}

inline Point3F& Point3F::interpolate( Point3F const &p1, Point3F const &p2, RealF t1 )
{
   RealF t2 = 1.0f - t1;
   x = p1.x * t2 + p2.x * t1;
   y = p1.y * t2 + p2.y * t1;
   z = p1.z * t2 + p2.z * t1;
   return *this;
}

#ifndef M_NOFILEIO
inline Bool Point3F::read( StreamIO & sio )
{
   return sio.read( &x ) && sio.read( &y ) && sio.read( &z );
}

inline Bool Point3F::write( StreamIO & sio )
{
   return sio.write( x ) && sio.write( y ) && sio.write( z );
}

#endif

//----------------------------------------------------------------------------
// Point4F methods
//----------------------------------------------------------------------------

inline Point4F::Point4F() 
{
}

inline Point4F::Point4F( const float _x, const float _y, const float _z, const RealF _w )
{ 
   x = _x;
   y = _y;
   z = _z;
	w = _w; 
}

inline Point4F& Point4F::set( const float _x, const float _y, const float _z, const RealF _w )
{
	x = _x;
	y = _y;
	z = _z;
	w = _w;
   return *this;
}

inline Point4F& Point4F::interpolate( Point4F const &p1, Point4F const &p2, RealF t1 )
{
   RealF t2 = 1.0f - t1;
   x = p1.x * t2 + p2.x * t1;
   y = p1.y * t2 + p2.y * t1;
   z = p1.z * t2 + p2.z * t1;
   w = p1.w * t2 + p2.w * t1;
   return *this;
}

#ifndef M_NOFILEIO
inline Bool Point4F::read( StreamIO & sio )
{
   return sio.read( &x ) && sio.read( &y ) && sio.read( &z ) && 
      sio.read( &w );
}

inline Bool Point4F::write( StreamIO & sio )
{
   return sio.write( x ) && sio.write( y ) && sio.write( z ) && sio.write( w );
}

#endif

typedef Point2F  Vector2F;
typedef Point3F  Vector3F;

class Point2D;
class Point3D;

//----------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_M_POINT_H_


