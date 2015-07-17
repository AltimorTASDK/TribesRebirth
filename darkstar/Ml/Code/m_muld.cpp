//================================================================
//   
//	$Workfile:   m_muld.cpp  $
//	$Revision:   1.0  $
// $Version: 1.6.2 $
//	$Date:   23 Oct 1995 13:25:16  $
// $Log:   R:\darkstar\develop\core\ml\vcs\m_muld.cpv  $
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#include "m_mul.h"


RMat2D& m_mul( const RMat2D & a, const RMat2D & b, RMat2D * r ) 
{ 
	AssertWarn( &a != r && &b != r, "m_mul: dest should not be same as source" );
   r->m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0];
   r->m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1];
   r->m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0];
   r->m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1];
   return *r;
}

TMat2D& m_mul( const TMat2D & a, const TMat2D & b, TMat2D* r )
{ 
	AssertWarn( &a != r && &b != r, "m_mul: dest should not be same as source" );
   r->m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0];
   r->m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1];
   r->m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0];
   r->m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1];
   r->p.x = a.p.x * b.m[0][0] + a.p.y * b.m[1][0] + b.p.x;
   r->p.y = a.p.x * b.m[0][1] + a.p.y * b.m[1][1] + b.p.y;
   return *r;
}

TMat2D& m_mul( const RMat2D & a, const TMat2D & b, TMat2D* r )
{ 
	AssertWarn( &b != r, "m_mul: dest should not be same as source" );
   r->m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0];
   r->m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1];
   r->m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0];
   r->m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1];
   r->p.x = b.p.x;
   r->p.y = b.p.y;
   return *r;
}

TMat2D& m_mul( const TMat2D & a, const RMat2D & b, TMat2D* r )
{ 
	AssertWarn( &a != r, "m_mul: dest should not be same as source" );
   r->m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0];
   r->m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1];
   r->m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0];
   r->m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1];
   r->p.x = a.p.x * b.m[0][0] + a.p.y * b.m[1][0];
   r->p.y = a.p.x * b.m[0][1] + a.p.y * b.m[1][1];
   return *r;
}

RMat3D& m_mul( const RMat3D & a, const RMat3D & b, RMat3D* r ) 
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
   return *r;
}

TMat3D& m_mul( const TMat3D & a, const TMat3D & b, TMat3D* r )
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
   r->p.x = a.p.x * b.m[0][0] + a.p.y * b.m[1][0] + a.p.z * b.m[2][0] + b.p.x;
   r->p.y = a.p.x * b.m[0][1] + a.p.y * b.m[1][1] + a.p.z * b.m[2][1] + b.p.y;
   r->p.z = a.p.x * b.m[0][2] + a.p.y * b.m[1][2] + a.p.z * b.m[2][2] + b.p.z;
   return *r;
}

TMat3D& m_mul( const RMat3D & a, const TMat3D & b, TMat3D* r )
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
   return *r;
}

TMat3D& m_mul( const TMat3D & a, const RMat3D & b, TMat3D* r )
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
   return *r;
}

// - Point/Vector * Matrix

Point2D& m_mul( const Point2D& a, const RMat2D& m, Point2D* r )
{
	AssertWarn( &a != r, "m_mul: dest should not be same as source" );
   r->x = a.x * m.m[0][0] + a.y * m.m[1][0];
   r->y = a.x * m.m[0][1] + a.y * m.m[1][1];
   return *r;
}

Point2D& m_mul( const Point2D& a, const TMat2D& m, Point2D* r )
{
	AssertWarn( &a != r, "m_mul: dest should not be same as source" );
   r->x = a.x * m.m[0][0] + a.y * m.m[1][0] + m.p.x;
   r->y = a.x * m.m[0][1] + a.y * m.m[1][1] + m.p.y;
   return *r;
}

Point3D& m_mul( const Point3D& a, const RMat3D& m, Point3D* r )
{
	AssertWarn( &a != r, "m_mul: dest should not be same as source" );
   r->x = a.x * m.m[0][0] + a.y * m.m[1][0] + a.z * m.m[2][0];
   r->y = a.x * m.m[0][1] + a.y * m.m[1][1] + a.z * m.m[2][1];
   r->z = a.x * m.m[0][2] + a.y * m.m[1][2] + a.z * m.m[2][2];
   return *r;
}

Point3D& m_mul( const Point3D& a, const TMat3D& m, Point3D* r )
{
	AssertWarn( &a != r, "m_mul: dest should not be same as source" );
   r->x = a.x * m.m[0][0] + a.y * m.m[1][0] + a.z * m.m[2][0] + m.p.x;
   r->y = a.x * m.m[0][1] + a.y * m.m[1][1] + a.z * m.m[2][1] + m.p.y;
   r->z = a.x * m.m[0][2] + a.y * m.m[1][2] + a.z * m.m[2][2] + m.p.z;
   return *r;
}
