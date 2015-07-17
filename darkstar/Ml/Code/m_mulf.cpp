//================================================================
//   
//	$Workfile:   m_mulf.cpp  $
//	$Revision:   1.3  $
// $Version: 1.6.2 $
//	$Date:   13 Dec 1995 19:27:48  $
// $Log:   R:\darkstar\develop\core\ml\vcs\m_mulf.cpv  $
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#include "ml.h"

extern "C" {
	void __cdecl m_Point3F_TMat3F_mulInverse( const Point3F * a, 
	   const TMat3F *b , Point3F * );
	void __cdecl m_Point3F_RMat3F_mulInverse( const Point3F * a, 
	   const RMat3F *b , Point3F * );
}

RMat2F& m_mul( const RMat2F & a, const RMat2F & b, RMat2F * r ) 
{ 
	AssertWarn( &a != r && &b != r, "m_mul: dest should not be same as source" );
   r->m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0];
   r->m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1];
   r->m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0];
   r->m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1];
   r->flags = (a.flags | b.flags) & (RMat2F::Matrix_HasRotation | RMat2F::Matrix_HasScale);
   return *r;
}

TMat2F& m_mul( const TMat2F & a, const TMat2F & b, TMat2F* r )
{ 
	AssertWarn( &a != r && &b != r, "m_mul: dest should not be same as source" );
   r->m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0];
   r->m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1];
   r->m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0];
   r->m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1];
   r->p.x = a.p.x * b.m[0][0] + a.p.y * b.m[1][0] + b.p.x;
   r->p.y = a.p.x * b.m[0][1] + a.p.y * b.m[1][1] + b.p.y;
   r->flags = a.flags | b.flags;
   return *r;
}

TMat2F& m_mul( const RMat2F & a, const TMat2F & b, TMat2F* r )
{ 
	AssertWarn( &b != r, "m_mul: dest should not be same as source" );
   r->m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0];
   r->m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1];
   r->m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0];
   r->m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1];
   r->p.x = b.p.x;
   r->p.y = b.p.y;
   r->flags = a.flags | b.flags;
   return *r;
}

TMat2F& m_mul( const TMat2F & a, const RMat2F & b, TMat2F* r )
{ 
	AssertWarn( &a != r, "m_mul: dest should not be same as source" );
   r->m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0];
   r->m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1];
   r->m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0];
   r->m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1];
   r->p.x = a.p.x * b.m[0][0] + a.p.y * b.m[1][0];
   r->p.y = a.p.x * b.m[0][1] + a.p.y * b.m[1][1];
   r->flags = a.flags | b.flags;
   return *r;
}

RMat3F& m_mul( const RMat3F & a, const RMat3F & b, RMat3F* r ) 
{ 
	AssertWarn( &a != r && &b != r, "m_mul: dest should not be same as source" );
   r->m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] + a.m[0][2] * b.m[2][0];
   r->m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[2][1];
   r->m[0][2] = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] + a.m[0][2] * b.m[2][2];
   r->m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[1][2] * b.m[2][0];
   r->m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[2][1];
   r->m[1][2] = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[1][2] * b.m[2][2];
   r->m[2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] + a.m[2][2] * b.m[2][0];
   r->m[2][1] = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[2][1];
   r->m[2][2] = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] + a.m[2][2] * b.m[2][2];
   r->flags = (a.flags | b.flags) & (RMat3F::Matrix_HasRotation | RMat3F::Matrix_HasScale);
   return *r;
}

#if 0
TMat3F& m_mul( const TMat3F & a, const TMat3F & b, TMat3F* r )
{ 
	AssertWarn( &a != r && &b != r, "m_mul: dest should not be same as source" );

#if 1
   m_TMat3F_TMat3F_mul(&a,&b,r);
#else
   r->m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] + a.m[0][2] * b.m[2][0];
   r->m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[2][1];
   r->m[0][2] = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] + a.m[0][2] * b.m[2][2];
   r->m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[1][2] * b.m[2][0];
   r->m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[2][1];
   r->m[1][2] = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[1][2] * b.m[2][2];
   r->m[2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] + a.m[2][2] * b.m[2][0];
   r->m[2][1] = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[2][1];
   r->m[2][2] = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] + a.m[2][2] * b.m[2][2];
   r->p.x = a.p.x * b.m[0][0] + a.p.y * b.m[1][0] + a.p.z * b.m[2][0] + b.p.x;
   r->p.y = a.p.x * b.m[0][1] + a.p.y * b.m[1][1] + a.p.z * b.m[2][1] + b.p.y;
   r->p.z = a.p.x * b.m[0][2] + a.p.y * b.m[1][2] + a.p.z * b.m[2][2] + b.p.z;
#endif
   r->flags = a.flags | b.flags;

   return *r;
}
#endif

TMat3F& m_mul( const RMat3F & a, const TMat3F & b, TMat3F* r )
{ 
	AssertWarn( &b != r, "m_mul: dest should not be same as source" );
   r->m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] + a.m[0][2] * b.m[2][0];
   r->m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[2][1];
   r->m[0][2] = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] + a.m[0][2] * b.m[2][2];
   r->m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[1][2] * b.m[2][0];
   r->m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[2][1];
   r->m[1][2] = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[1][2] * b.m[2][2];
   r->m[2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] + a.m[2][2] * b.m[2][0];
   r->m[2][1] = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[2][1];
   r->m[2][2] = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] + a.m[2][2] * b.m[2][2];
   r->p.x = b.p.x;
   r->p.y = b.p.y;
   r->p.z = b.p.z;
   r->flags = a.flags | b.flags;
   return *r;
}

TMat3F& m_mul( const TMat3F & a, const RMat3F & b, TMat3F* r )
{ 
	AssertWarn( &a != r, "m_mul: dest should not be same as source" );
   r->m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] + a.m[0][2] * b.m[2][0];
   r->m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[2][1];
   r->m[0][2] = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] + a.m[0][2] * b.m[2][2];
   r->m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[1][2] * b.m[2][0];
   r->m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[2][1];
   r->m[1][2] = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[1][2] * b.m[2][2];
   r->m[2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] + a.m[2][2] * b.m[2][0];
   r->m[2][1] = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[2][1];
   r->m[2][2] = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] + a.m[2][2] * b.m[2][2];
   r->p.x = a.p.x * b.m[0][0] + a.p.y * b.m[1][0] + a.p.z * b.m[2][0];
   r->p.y = a.p.x * b.m[0][1] + a.p.y * b.m[1][1] + a.p.z * b.m[2][1];
   r->p.z = a.p.x * b.m[0][2] + a.p.y * b.m[1][2] + a.p.z * b.m[2][2];
   r->flags = a.flags | b.flags;
   return *r;
}

// This is like TMat x TMat = Tmat where the first TMat only has translation.
TMat3F& m_mul( const Point3F & trans, const TMat3F & b, TMat3F* r )
{ 
	AssertWarn( &b != r, "m_mul: dest should not be same as source" );
   *r = b;
   m_mul( trans, b, &r->p );
   return *r;
}

// - Point/Vector * Matrix

Point2F& m_mul( const Point2F& a, const RMat2F& m, Point2F* r )
{
	AssertWarn( &a != r, "m_mul: dest should not be same as source" );
   r->x = a.x * m.m[0][0] + a.y * m.m[1][0];
   r->y = a.x * m.m[0][1] + a.y * m.m[1][1];
   return *r;
}

Point2F& m_mul( const Point2F& a, const TMat2F& m, Point2F* r )
{
	AssertWarn( &a != r, "m_mul: dest should not be same as source" );
   r->x = a.x * m.m[0][0] + a.y * m.m[1][0] + m.p.x;
   r->y = a.x * m.m[0][1] + a.y * m.m[1][1] + m.p.y;
   return *r;
}

#if 0
Point3F& m_mul( const Point3F& a, const RMat3F& m, Point3F* r )
{
	AssertWarn( &a != r, "m_mul: dest should not be same as source" );
#if 1
   m_Point3F_RMat3F_mul( &a, &m, r );
#else
   r->x = a.x * m.m[0][0] + a.y * m.m[1][0] + a.z * m.m[2][0];
   r->y = a.x * m.m[0][1] + a.y * m.m[1][1] + a.z * m.m[2][1];
   r->z = a.x * m.m[0][2] + a.y * m.m[1][2] + a.z * m.m[2][2];
#endif
   return *r;
}
#endif

#if 0
Point3F& m_mul( const Point3F& a, const TMat3F& m, Point3F* r )
{
	AssertWarn( &a != r, "m_mul: dest should not be same as source" );
#if 1
   m_Point3F_TMat3F_mul( &a, &m, r );
#else
   r->x = a.x * m.m[0][0] + a.y * m.m[1][0] + a.z * m.m[2][0] + m.p.x;
   r->y = a.x * m.m[0][1] + a.y * m.m[1][1] + a.z * m.m[2][1] + m.p.y;
   r->z = a.x * m.m[0][2] + a.y * m.m[1][2] + a.z * m.m[2][2] + m.p.z;
#endif
   return *r;
}
#endif

Point3F& m_mulInverse( const Point3F& a, const RMat3F& m, Point3F* r )
{
	AssertWarn( &a != r, "m_mul: dest should not be same as source" );
#if 1
   m_Point3F_RMat3F_mulInverse( &a, &m, r );
#else
   r->x = a.x * m.m[0][0] + a.y * m.m[0][1] + a.z * m.m[0][2];
   r->y = a.x * m.m[1][0] + a.y * m.m[1][1] + a.z * m.m[1][2];
   r->z = a.x * m.m[2][0] + a.y * m.m[2][1] + a.z * m.m[2][2];
#endif
   return *r;
}

Point3F& m_mulInverse( const Point3F& a, const TMat3F& m, Point3F* r )
{
	AssertWarn( &a != r, "m_mul: dest should not be same as source" );
#if 1
   m_Point3F_TMat3F_mulInverse( &a, &m, r );
#else
   Point3F b = a;
   b -= m.p;
   m_mulInverse( b, *((RMat3F*)&m), r );
#endif
   return *r;
}


//------------------------------------------------------------------------------

extern "C" {
	void __cdecl m_QuatF_QuatF_mul( const QuatF *, const QuatF *, QuatF * );
}


Point3F& m_mul( const Point3F &p, const QuatF &q, Point3F *r )
{
   QuatF qq;
   QuatF qi = q;
   QuatF qv( p.x, p.y, p.z, 0.0f);

   qi.inverse();   
   m_mul(qi, qv, &qq );
   m_mul(qq, q, &qv );
   r->set(qv.x, qv.y, qv.z);
   return ( *r );
}


Point3F& m_mul( const Point3F &p, const TQuatF &q, Point3F *r )
{
   //rotate a point by a Quaternion
   QuatF a;
   QuatF i = q;
   QuatF v( p.x, p.y, p.z, 0.0f);
   i.inverse();   
   m_mul(i, v, &a );
   m_mul(a, q, &v );
   v.normalize();
   r->set(v.x, v.y, v.z);

   *r += q.p;
   return ( *r );
}


QuatF& m_mul( const QuatF &a, const QuatF &b, QuatF *qr )
{
	AssertWarn( &a != qr && &b != qr, "m_mul: dest should not be same as source" );
   qr->w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
   qr->x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
   qr->y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
   qr->z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
   return ( *qr);
}


//------------------------------------------------------------------------------

Box3F& m_mul(const Box3F &b0, const RMat3F &m, Box3F *b1)
{

   Point3F x,y,z;
   m.getRow(0,&x);
   m.getRow(1,&y);
   m.getRow(2,&z);

   Point3F center,tcenter,radii;
   center  = b0.fMin + b0.fMax;
   center *= 0.5f;
   m_mul(center,m,&tcenter);
   radii   = b0.fMax - b0.fMin;
   radii  *= 0.5f;

   float rx = fabs(x.x) * radii.x + fabs(y.x) * radii.y + fabs(z.x) * radii.z;
   float ry = fabs(x.y) * radii.x + fabs(y.y) * radii.y + fabs(z.y) * radii.z;
   float rz = fabs(x.z) * radii.x + fabs(y.z) * radii.y + fabs(z.z) * radii.z;

   b1->fMin = b1->fMax = tcenter;
   b1->fMin.x -= rx;
   b1->fMin.y -= ry;
   b1->fMin.z -= rz;

   b1->fMax.x += rx;
   b1->fMax.y += ry;
   b1->fMax.z += rz;

   return *b1;

/*
   Point3F p, u, v, w, z;
   
   m_mul(b0.fMin, m, &p);
   m_mul(Point3F(b0.fMax.x - b0.fMin.x, 0.0f, 0.0f), m, &u);
   m_mul(Point3F(0.0f, b0.fMax.y - b0.fMin.y, 0.0f), m, &v);
   m_mul(Point3F(0.0f, 0.0f, b0.fMax.z - b0.fMin.z), m, &w);

	z.x = u.x + v.x + w.x;
	z.y = u.y + v.y + w.y;
	z.z = u.z + v.z + w.z;

   b1->fMin = u;
   b1->fMin.setMin(v);
   b1->fMin.setMin(w);
   b1->fMin.setMin(z);
   b1->fMin += p;

   b1->fMax = u;
   b1->fMax.setMax(v);
   b1->fMax.setMax(w);
   b1->fMax.setMax(z);
   b1->fMax += p;
   
   return *b1;
*/
}

Box3F& m_mul(const Box3F &b0, const TMat3F &m, Box3F *b1)
{
   m_mul(b0, (RMat3F)m, b1);
   b1->fMax += m.p;
   b1->fMin += m.p;
   return *b1;
}

