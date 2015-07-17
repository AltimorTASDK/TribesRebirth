//================================================================
//   
// $Workfile:   m_coll.h  $
// $Revision:   1.0  $
// $Version$
//	
// DESCRIPTION:
//    Geometry routines useful when checking for collisions.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

//---------------------------------------------------------------------------

#ifndef _M_COLL_H_
#define _M_COLL_H_

#include <base.h>
#include "m_dot.h"
#include "m_mat3.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//------------------------------------------------------------------------------------
// returns normal to plane containing v1,v2,v3 facing "outward" assuming v1,v2,v3 clock-wise order
//------------------------------------------------------------------------------------
inline void m_normal(const Point3F & v1, const Point3F &v2, const Point3F &v3, Point3F &normal)
{
	// build the normal
	Point3F v13,v23;
	v13.x = v1.x-v3.x;
	v13.y = v1.y-v3.y;
	v13.z = v1.z-v3.z;
	v23.x = v2.x-v3.x;
	v23.y = v2.y-v3.y;
	v23.z = v2.z-v3.z;
	m_cross(v23,v13,&normal);
}

//------------------------------------------------------------------------------------
// m_pointInTriangle returns true if point is inside triangle defined by verts on plane w/ normal "normal"
//------------------------------------------------------------------------------------
bool m_pointInTriangle(const Point3F & point, const Point3F & normal,
                               const Point3F &vert1,
                               const Point3F &vert2,
                               const Point3F &vert3);

//------------------------------------------------------------------------------------
// m_pointInPoly -- same as point in triangle, above
//------------------------------------------------------------------------------------
bool m_pointInPoly(const Point3F & point, const Point3F & normal,
                           const Point3F * verts, int n);

//------------------------------------------------------------------------------------
// m_lineAABox returns true if line (a,b) hits axis aligned box (AABox) defined by minCorner and maxCorner
// hitTime is set to the "time" along line of intersection
// time goes from 0.0f at a to 1.0f at b
// if a is inside of box returns true w/ time 0.0f
//------------------------------------------------------------------------------------
bool m_lineAABox(const Point3F & a, const Point3F & b,
                         const Point3F & minCorner, const Point3F & maxCorner, 
                         float & hitTime);

//------------------------------------------------------------------------------------
// m_tubeAABox returns true if tube hits axis aligned box (AABox) defined by minCorner and maxCorner
// hitTime is set to the "time" along line of intersection
// time goes from 0.0f at a to 1.0f at b
// if a is inside of box returns true w/ time 0.0f
// m_tubeAABox uses m_lineAABox by first adjusting box width depending on radius of tube
//------------------------------------------------------------------------------------
bool m_tubeAABox(const Point3F & a, const Point3F & b, float radius,
                       const Point3F & minCorner, const Point3F & maxCorner, 
                       float & hitTime);

//------------------------------------------------------------------------------------
// m_sphereAABox returns true if sphere collides with axis aligned box
// on exit minDist2 will hold square of min distance from sphere center to box
//------------------------------------------------------------------------------------
bool m_sphereAABox(const Point3F & center, float radius2, 
                           const Point3F & minCorner, const Point3F & maxCorner,
						   float & minDist2);

//------------------------------------------------------------------------------------
// m_OBoxOBox returns true if boxes collide
// aRadii and bRadii define center to face distances in each boxes own space
// trans is the transform that takes box B into box A space:
//   x axis to x axis, y axis to y axis, z axis to z axis, center to center
// if there is a collision, aOverlap and bOverlap holds the amount of overlap
// between the boxes along each axis of a (b)
// the meaning of these vectors is depicted below:

/*					
               ----------
			   |        |
               |        |
               |        |
               |   A    |
               |        |
               |        |
               |        |
             ..|.. .....|...   -
             . |   /^\  |  .   | aOverlap.y
             . _________   .   -
             .  /  		\  .
             ./           \.
             .\    B     / .
             . \        /  .
             .   \    /	   .
             .....\./.......
*/
// notice that aOverlap.x is not as easy to interpret...
// it will be width of box A plus width of outer box B (bbox of box b in A's space)
// minus the horizontal distance between A and B.  Note that the corresponding formula
// in the vertical direction produces the distance marked off as aOverlap.y in the figure
// Finally, the same process determines bOverlap, except carried out in B's space
//------------------------------------------------------------------------------------
bool m_OBoxOBox(const Point3F & aRadii, const Point3F & bRadii, const TMat3F & trans,
                        Point3F & aOverlap, Point3F & bOverlap);

//------------------------------------------------------------------------------------
// like m_OBoxOBox but assumes aRadii.z = 0, saves about 10% of the *,+,-'s
//------------------------------------------------------------------------------------
bool m_rectOBox(const Point3F & aRadii, const Point3F & bRadii, const TMat3F & trans,
                        Point3F & aOverlap, Point3F & bOverlap);

//------------------------------------------------------------------------------------
// m_planeOBox is based on the same principle as other OBox routines but works a bit
// different.  The transform takes the plane into some space (not nec. one in which
// the plane is axis aligned, but the box center must be at the origin in this space)
// and the normal is in this same space.  p is any point on the plane.
// This test then projects the box radii onto the normal and see if they reach
// the plane...
// overlap will hold the distance into the box that the plane reaches (times the length
// of the normal)
//------------------------------------------------------------------------------------
bool m_planeOBox(const Point3F & bRadii, const TMat3F & trans,
                         const Point3F & n, const Point3F & p,
                         float & overlap);

//------------------------------------------------------------------------------------
// m_polyOBox
//------------------------------------------------------------------------------------
bool m_polyOBox(const Point3F & radii, const TMat3F & trans, const TMat3F & invTrans,
               			const Point3F & normal, const Point3F * v, Point3F * e, int n,
                        float & overlap);

//------------------------------------------------------------------------------------
// like m_OBoxOBox but assumes aRadii.z = 0, aRadii.x = 0, saves about 20% of the *,+,-'s
//------------------------------------------------------------------------------------
bool m_lineOBox(const Point3F & aRadii, const Point3F & bRadii, const TMat3F & trans,
                        Point3F & aOverlap, Point3F & bOverlap);

#endif








