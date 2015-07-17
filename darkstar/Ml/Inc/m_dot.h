//================================================================
//   
// $Workfile:   m_dot.h  $
// $Revision:   1.2  $
// $Version$
//	$Date:   13 Dec 1995 19:26:58  $
//	
// DESCRIPTION:
//    Dot and Cross product functions.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _M_DOT_H_
#define _M_DOT_H_

//------------------------------------------------------
// Float Versions:
// Dot product

#ifndef M_NO_FLOAT

extern "C"
{
   RealF __cdecl m_asm_dot2  (const Point2F *, const Point2F *);
   RealF __cdecl m_asm_dot3  (const Point3F *, const Point3F *);
   void  __cdecl m_asm_cross3(const Point3F *, const Point3F *, const Point3F *);
}

inline RealF m_dot( const Point2F& a, const Point2F& b )
{
   //return RealF(a.x * b.x + a.y * b.y);
   return (m_asm_dot2(&a, &b));
}

inline RealF m_dot( const Point3F& a, const Point3F& b )
{
   //return RealF(a.x * b.x + a.y * b.y + a.z * b.z);
   return (m_asm_dot3(&a, &b));
}

inline Point3F& m_cross( const Point3F& a, const Point3F& b, Point3F* r )
{
	//r->x = (RealF)(a.y*b.z - a.z*b.y);
	//r->y = (RealF)(a.z*b.x - a.x*b.z);
	//r->z = (RealF)(a.x*b.y - a.y*b.x);
	//return (*r);
   m_asm_cross3(&a, &b, r);
   return (*r);
}

inline RealF m_cross( const Point2F& a, const Point2F& b)
{
	return (RealF)(a.x*b.y - a.y*b.x);
}

inline RealF m_dotSign( const Point2F& a, const Point2F& b )
{
	RealF dot = m_dot( a, b );
   return dot / dot;
}

inline RealF m_dotSign( const Point3F& a, const Point3F& b )
{
	RealF dot = m_dot( a, b );
   return dot / dot;
}

#endif   // M_NO_FLOAT

//------------------------------------------------------
// Double Versions:
// Dot product

extern "C" {
	Int32 __cdecl m_dotSign2( const Point2I* a, const Point2I* b );
	Int32	__cdecl m_dotSign3( const Point3I* a, const Point3I* b );
	Int32	__cdecl m_dotSign3e( const Point3I* a, const Point3I* b, Int32 err );
}


//------------------------------------------------------
// Integer Versions:
// Dot product

inline Int32 m_dot( const Point2I& a, const Point2I& b )
{
   return Int32(a.x * b.x + a.y * b.y);
}

inline Int32 m_dot( const Point3I& a, const Point3I& b )
{
   return Int32(a.x * b.x + a.y * b.y + a.z * b.z);
}

inline Int32 	m_dotSign( const Point2I& a, const Point2I& b )
{
	return m_dotSign2( &a, &b );
}

inline Int32 	m_dotSign( const Point3I& a, const Point3I& b )
{
	return m_dotSign3( &a, &b );
}

inline Int32 	m_dotSign( const Point3I& a, const Point3I& b, Int32 err )
{
	return m_dotSign3e( &a, &b, err );
}

//------------------------------------------------------
// Integer Versions:
// Cross product

inline Int32 m_cross( const Point2I& a, const Point2I& b)
{
	return (Int32)(a.x*b.y - a.y*b.x);
}

inline Point3I& m_cross( const Point3I& a, const Point3I& b, Point3I* r )
{
	r->x = (Int32)(a.y*b.z - a.z*b.y);
	r->y = (Int32)(a.z*b.x - a.x*b.z);
	r->z = (Int32)(a.x*b.y - a.y*b.x);
	return *r;
}

#endif
