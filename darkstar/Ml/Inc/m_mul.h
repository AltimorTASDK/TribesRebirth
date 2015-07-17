//================================================================
//   
// $Workfile:   m_mul.h  $
// $Revision:   1.4  $
// $Version$
//	
// DESCRIPTION:
//    Overloaded Functions to multiply Points, RMats and TMats.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _M_MUL_H_    
#define _M_MUL_H_

class RMat2F;
class TMat2F;
class RMat3F;
class TMat3F;
class RMat2D;
class TMat2D;
class RMat3D;
class TMat3D;

class Point2F;
class Point2I;
class Point2D;
class Point3F;
class Point3I;
class Point3D;

class QuatF;
class Box2F;
class Box3F;

//---------------------------------------------------------------------------
// assembly routines

extern "C" {
	void __cdecl m_Point3F_TMat3F_mul( const Point3F * a, const TMat3F *b , Point3F * );
	void __cdecl m_TMat3F_TMat3F_mul( const TMat3F *, const TMat3F *, TMat3F * );
	void __cdecl m_Point3F_RMat3F_mul( const Point3F * a, const RMat3F *b , Point3F * );
}

//---------------------------------------------------------------------------
// Float versions:
// Multiply matrix with matrix

extern RMat2F& DLLAPI m_mul( const RMat2F&, const RMat2F&, RMat2F* );
extern TMat2F& DLLAPI m_mul( const TMat2F&, const TMat2F&, TMat2F* );
extern TMat2F& DLLAPI m_mul( const RMat2F&, const TMat2F&, TMat2F* );
extern TMat2F& DLLAPI m_mul( const TMat2F&, const RMat2F&, TMat2F* );

extern RMat3F& DLLAPI m_mul( const RMat3F&, const RMat3F&, RMat3F* );
extern TMat3F& DLLAPI m_mul( const RMat3F&, const TMat3F&, TMat3F* );
extern TMat3F& DLLAPI m_mul( const TMat3F&, const RMat3F&, TMat3F* );
extern TMat3F& DLLAPI m_mul( const Point3F & trans, const TMat3F &, TMat3F * );

//extern TMat3F& DLLAPI m_mul( const TMat3F&, const TMat3F&, TMat3F* );
// ** function inlined for speed **
inline TMat3F & m_mul( const TMat3F&a, const TMat3F&b, TMat3F*c )
{
   AssertWarn( &a != c, "m_mul: dest should not be same as source" );
   m_TMat3F_TMat3F_mul( &a, &b, c);
   return *c;
}

// Multiply point/vector with matrix

extern Point2F& DLLAPI m_mul( const Point2F&, const RMat2F&, Point2F* );
extern Point2F& DLLAPI m_mul( const Point2F&, const TMat2F&, Point2F* );

//extern Point3F& DLLAPI m_mul( const Point3F&, const RMat3F&, Point3F* );
// ** function inlined for speed **
inline Point3F & m_mul( const Point3F&a, const RMat3F&m, Point3F*r )
{
   AssertWarn( &a != r, "m_mul: dest should not be same as source" );
   m_Point3F_RMat3F_mul( &a, &m, r);
   return *r;
}

//extern Point3F& DLLAPI m_mul( const Point3F&, const TMat3F&, Point3F* );
// ** function inlined for speed **
inline Point3F & m_mul( const Point3F&a, const TMat3F&m, Point3F*r )
{
   AssertWarn( &a != r, "m_mul: dest should not be same as source" );
   m_Point3F_TMat3F_mul( &a, &m, r);
   return *r;
}

extern Point3F& DLLAPI m_mulInverse( const Point3F&, const RMat3F&, Point3F* );
extern Point3F& DLLAPI m_mulInverse( const Point3F&, const TMat3F&, Point3F* );


// Quaternion

extern Point3F& DLLAPI m_mul( const Point3F&, const QuatF&, Point3F* );
extern QuatF&   DLLAPI m_mul( const QuatF&, const QuatF&, QuatF* );

                                            
// Box

extern Box3F& DLLAPI m_mul( const Box3F&, const RMat3F&, Box3F* );
extern Box3F& DLLAPI m_mul( const Box3F&, const TMat3F&, Box3F* );


//---------------------------------------------------------------------------
// Double versions:
// Multiply matrix with matrix

extern RMat2D& DLLAPI m_mul( const RMat2D&, const RMat2D&, RMat2D* );
extern TMat2D& DLLAPI m_mul( const TMat2D&, const TMat2D&, TMat2D* );
extern TMat2D& DLLAPI m_mul( const RMat2D&, const TMat2D&, TMat2D* );
extern TMat2D& DLLAPI m_mul( const TMat2D&, const RMat2D&, TMat2D* );

extern RMat3D& DLLAPI m_mul( const RMat3D&, const RMat3D&, RMat3D* );
extern TMat3D& DLLAPI m_mul( const TMat3D&, const TMat3D&, TMat3D* );
extern TMat3D& DLLAPI m_mul( const RMat3D&, const TMat3D&, TMat3D* );
extern TMat3D& DLLAPI m_mul( const TMat3D&, const RMat3D&, TMat3D* );

// Multiply point/vector with matrix

extern Point2D& DLLAPI m_mul( const Point2D&, const RMat2D&, Point2D* );
extern Point2D& DLLAPI m_mul( const Point2D&, const TMat2D&, Point2D* );

extern Point3D& DLLAPI m_mul( const Point3D&, const RMat3D&, Point3D* );
extern Point3D& DLLAPI m_mul( const Point3D&, const TMat3D&, Point3D* );

#endif   // _M_MUL_H_
