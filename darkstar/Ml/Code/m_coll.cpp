#include "ml.h"
#include "m_coll.h"

//-------------------------------------- MSVC Does not have proper floating fabs
//
#ifdef _MSC_VER
namespace {

float
fabs(float in_abs)
{
   return float(::fabs(double(in_abs)));
}

};
#endif


//------------------------------------------------------------------------------------
// First a couple utility functions:
//------------------------------------------------------------------------------------

// the following is used in the point in triangle routine
// these three dot products are the dot product of the vector from
// each vertex to the point and the normal to one of the edges:
// vector (V1,P) . normal to vector (V1,V2)
// vector (V2,P) . normal to vector (V2,V3)
// vector (V3,P) . normal to vector (V3,V1)
// P is inside triangle if the dot products all have the same sign
inline void doDot(float v1x, float v1y, float v2x, float v2y, float v3x, float v3y,
                            float px, float py, float & dot1, float & dot2, float & dot3)
{
   dot1 = (v1x-px) * (v1y-v2y) +
          (v1y-py) * (v2x-v1x);

   dot2 = (v2x-px) * (v2y-v3y) +
          (v2y-py) * (v3x-v2x);

   dot3 = (v3x-px) * (v3y-v1y) +
          (v3y-py) * (v1x-v3x);
}

// this version of doDot used by m_pointInPoly
inline float doDot(float v1x, float v1y, float v2x, float v2y, float px, float py)
{
   return (v1x-px) * (v1y-v2y) +
          (v1y-py) * (v2x-v1x);
}

// used by m_lineAABox
// assumes tdenom>=0
inline bool checkSide( float tnum, float tdenom, 
                               float a1, float a2,
                               float b1, float b2, 
                               float min1, float min2,
                               float max1, float max2,
                               float & hitTime )
{
	if ( tnum * (b1-a1) < tdenom * (max1-a1) &&
		  tnum * (b1-a1) > tdenom * (min1-a1) &&
		  tnum * (b2-a2) < tdenom * (max2-a2) &&
		  tnum * (b2-a2) > tdenom * (min2-a2) )
	{
		hitTime=tnum/tdenom;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------------
// m_pointInTriangle returns true if point is inside triangle defined by verts on plane w/ normal "normal"
// projects to 2 dimensions to do the job (drops coord with largest abs value in normal)
//------------------------------------------------------------------------------------
bool m_pointInTriangle(const Point3F & point, const Point3F & normal,
                               const Point3F &vert1,
                               const Point3F &vert2,
                               const Point3F &vert3)
{
	float dot1,dot2,dot3;

	// we can ignore one of the dimensions because all points are on the same plane...
	if (fabs(normal.y)>fabs(normal.x)&&fabs(normal.y)>fabs(normal.z))
		// drop y coord
		doDot(vert1.x,vert1.z,vert2.x,vert2.z,vert3.x,vert3.z,point.x,point.z,dot1,dot2,dot3);
	else if (fabs(normal.x)>fabs(normal.y)&&fabs(normal.x)>fabs(normal.z))
		// drop x coord
		doDot(vert1.y,vert1.z,vert2.y,vert2.z,vert3.y,vert3.z,point.y,point.z,dot1,dot2,dot3);
	else
		// drop z coord
		doDot(vert1.x,vert1.y,vert2.x,vert2.y,vert3.x,vert3.y,point.x,point.y,dot1,dot2,dot3);

	return (dot1*dot2>=0 && dot2*dot3>=0);
}

//------------------------------------------------------------------------------------
// m_pointInPoly returns true if point is inside poly defined by verts on plane w/ 
// normal "normal" -- based on m_pointInTriangle.
//------------------------------------------------------------------------------------
bool m_pointInPoly(const Point3F & point, 
                           const Point3F & normal,
                           const Point3F * verts,
                           int n)
{
	float thisDot, lastDot=0;
	int i;

	// we can ignore one of the dimensions because all points are on the same plane...
	if (fabs(normal.y)>fabs(normal.x)&&fabs(normal.y)>fabs(normal.z))
	{
		// drop y coord
		thisDot = doDot(verts[n-1].x,verts[n-1].z,verts[0].x,verts[0].z,point.x,point.z);
		if (thisDot*lastDot<0)
			return false;
		lastDot = thisDot;

		for (i=0;i<n-1;i++)
		{
			thisDot = doDot(verts[i].x,verts[i].z,verts[i+1].x,verts[i+1].z,point.x,point.z);
			if (thisDot*lastDot<0)
				return false; // different sign, point outside one of the edges
			lastDot = thisDot;
		}
	}
	else if (fabs(normal.x)>fabs(normal.y)&&fabs(normal.x)>fabs(normal.z))
	{
		// drop x coord
		thisDot = doDot(verts[n-1].y,verts[n-1].z,verts[0].y,verts[0].z,point.y,point.z);
		if (thisDot*lastDot<0)
			return false;
		lastDot = thisDot;

		for (i=0;i<n-1;i++)
		{
			thisDot = doDot(verts[i].y,verts[i].z,verts[i+1].y,verts[i+1].z,point.y,point.z);
			if (thisDot*lastDot<0)
				return false; // different sign, point outside one of the edges
			lastDot = thisDot;
		}
	}
	else
	{
		// drop z coord
		thisDot = doDot(verts[n-1].x,verts[n-1].y,verts[0].x,verts[0].y,point.x,point.y);
		if (thisDot*lastDot<0)
			return false;
		lastDot = thisDot;

		for (i=0;i<n-1;i++)
		{
			thisDot = doDot(verts[i].x,verts[i].y,verts[i+1].x,verts[i+1].y,point.x,point.y);
			if (thisDot*lastDot<0)
				return false; // different sign, point outside one of the edges
			lastDot = thisDot;
		}
	}

	return true;
}


//------------------------------------------------------------------------------------
// m_lineAABox returns true if line (a,b) hits axis aligned box (AABox) defined by minCorner and maxCorner
// hitTime is set to the "time" along line of intersection
// time goes from 0.0f at a to 1.0f at b
// if a is inside of box returns true w/ time 0.0f
//------------------------------------------------------------------------------------
bool m_lineAABox(const Point3F & a, const Point3F & b,
                         const Point3F & minCorner, const Point3F & maxCorner, 
                         float & hitTime)
{
	bool aoutLeft;
	bool aoutRight;
	bool aoutBack;
	bool aoutFront;
	bool aoutTop;
	bool aoutBottom;

	aoutLeft = a.x<minCorner.x;
	if (aoutLeft)
	{
		if ( b.x>=minCorner.x &&
		     checkSide( (minCorner.x-a.x), (b.x-a.x), a.y, a.z, b.y, b.z, 
		                minCorner.y, minCorner.z, maxCorner.y, maxCorner.z, hitTime )
		   )
			return true;
		aoutRight = false;
	}
	else
	{
		aoutRight = a.x>maxCorner.x;
		if ( aoutRight && b.x<=maxCorner.x &&
		     checkSide( (a.x-maxCorner.x), (a.x-b.x), a.y, a.z, b.y, b.z,
		                minCorner.y, minCorner.z, maxCorner.y, maxCorner.z, hitTime )
		   )
			return true;
	}
	
	aoutBack	= a.y>maxCorner.y;
	if (aoutBack)
	{
		if ( b.y<=maxCorner.y &&
		     checkSide( (a.y-maxCorner.y), (a.y-b.y), a.x, a.z, b.x, b.z,
		                minCorner.x, minCorner.z, maxCorner.x, maxCorner.z, hitTime )
		   )
		    return true;
		aoutFront=false;
	}
	else
	{
		aoutFront = a.y<minCorner.y;
		if ( aoutFront && b.y>=minCorner.y &&
		     checkSide( (minCorner.y-a.y), (b.y-a.y), a.x, a.z, b.x, b.z,
		                minCorner.x, minCorner.z, maxCorner.x, maxCorner.z, hitTime )
		   )
			return true;
	}

	aoutTop = a.z>maxCorner.z;
	if (aoutTop)
	{
		if ( b.z<=maxCorner.z &&
		      checkSide( (a.z-maxCorner.z), (a.z-b.z), a.x, a.y, b.x, b.y,
		                 minCorner.x, minCorner.y, maxCorner.x, maxCorner.y, hitTime )
		   )
			return true;	
		aoutBottom=false;
	}
	else
	{
		aoutBottom = a.z<minCorner.z;
		if ( aoutBottom && b.z>=minCorner.z &&
		     checkSide( (minCorner.z-a.z), (b.z-a.z), a.x, a.y, b.x, b.y,
		                minCorner.x, minCorner.y, maxCorner.x, maxCorner.y, hitTime )
		   )
			return true;
	}
	
	// check to see if a is inside
	if (!(aoutLeft | aoutRight | aoutBack | aoutFront | aoutTop | aoutBottom))
	{
		hitTime=0.0f;
		return true;
	}
	
	return false;
}

//------------------------------------------------------------------------------------
// m_tubeAABox returns true if tube hits axis aligned box (AABox) defined by minCorner and maxCorner
// hitTime is set to the "time" along line of intersection
// time goes from 0.0f at a to 1.0f at b
// if a is inside of box returns true w/ time 0.0f
// m_tubeAABox uses m_lineAABox by first adjusting box width depending on radius of tube
//------------------------------------------------------------------------------------
bool m_tubeAABox(const Point3F & a, const Point3F & b, float radius,
                       const Point3F & minCorner, const Point3F & maxCorner, 
                       float & hitTime)
{

	Point3F adjMin, adjMax;
	adjMin.x = minCorner.x - radius;
	adjMin.y = minCorner.y - radius;
	adjMin.z = minCorner.z - radius;

	adjMax.x = maxCorner.x + radius;
	adjMax.y = maxCorner.y + radius;
	adjMax.z = maxCorner.z + radius;

	return m_lineAABox(a,b,adjMin,adjMax,hitTime);
}

//------------------------------------------------------------------------------------
// m_sphereAABox returns true if sphere collides with axis aligned box
// on exit minDist2 will hold square of min distance from sphere center to box
//------------------------------------------------------------------------------------
bool m_sphereAABox(const Point3F & center, float radius2, 
                           const Point3F & minCorner, const Point3F & maxCorner,
						   float & minDist2)
{
	minDist2 = 0.0f; 

	if (center.x<minCorner.x)
		minDist2 += (center.x-minCorner.x)*(center.x-minCorner.x);
	else if (center.x>maxCorner.x)
		minDist2 += (center.x-maxCorner.x)*(center.x-maxCorner.x);

	if (center.y<minCorner.y)
		minDist2 += (center.y-minCorner.y)*(center.y-minCorner.y);
	else if (center.y>maxCorner.y)
		minDist2 += (center.y-maxCorner.y)*(center.y-maxCorner.y);

	if (center.z<minCorner.z)
		minDist2 += (center.z-minCorner.z)*(center.z-minCorner.z);
	else if (center.z>maxCorner.z)
		minDist2 += (center.z-maxCorner.z)*(center.z-maxCorner.z);

	// collide iff closest point w/in radius
	return minDist2 < radius2;
}

//------------------------------------------------------------------------------------
// m_OBoxOBox returns true if "oriented" boxes collide (oriented as opposed to axis-aligned)
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
			 ..... ./.......
*/
// notice that aOverlap.x is not as easy to interpret...
// it will be width of box A plus width of outer box B (bbox of box B in A's space)
// minus the horizontal distance between A and B's center.  Note that the corresponding formula
// in the vertical direction produces the distance marked off as aOverlap.y in the figure
// Finally, the same process determines bOverlap, except carried out in B's space
//------------------------------------------------------------------------------------

bool m_OBoxOBox(const Point3F & aRadii, const Point3F & bRadii, const TMat3F & trans,
                        Point3F & aOverlap, Point3F & bOverlap)
{
	const Point3F & T = trans.p;

	float absTrans00,absTrans01,absTrans02;
	float absTrans10,absTrans11,absTrans12;
	float absTrans20,absTrans21,absTrans22;
	
	absTrans00 = fabs(trans.m[0][0]);
	absTrans10 = fabs(trans.m[1][0]);
	absTrans20 = fabs(trans.m[2][0]);

	aOverlap.x = aRadii.x + bRadii.x * absTrans00 + bRadii.y * absTrans10 + bRadii.z * absTrans20 - fabs(T.x);
	if (aOverlap.x<0.0f)
		return false;

	absTrans01 = float(fabs(trans.m[0][1]));
	absTrans11 = float(fabs(trans.m[1][1]));
	absTrans21 = float(fabs(trans.m[2][1]));
	aOverlap.y = aRadii.y + bRadii.x * absTrans01 +	bRadii.y * absTrans11 +	bRadii.z * absTrans21 - fabs(T.y);
	if (aOverlap.y<0.0f)
		return false;
	
	absTrans02 = fabs(trans.m[0][2]);
	absTrans12 = fabs(trans.m[1][2]);
	absTrans22 = fabs(trans.m[2][2]);
	aOverlap.z = aRadii.z + bRadii.x * absTrans02 + bRadii.y * absTrans12 +	bRadii.z * absTrans22 - fabs(T.z);
	if (aOverlap.z<0.0f)
		return false;

	bOverlap.x = aRadii.x*absTrans00 + aRadii.y*absTrans01+ aRadii.z*absTrans02 + bRadii.x -
	             fabs(T.x*trans.m[0][0] + T.y*trans.m[0][1] + T.z*trans.m[0][2]);
	if (bOverlap.x<0.0f)
		return false;

	bOverlap.y = aRadii.x*absTrans10 + aRadii.y*absTrans11+ aRadii.z*absTrans12 + bRadii.y -
	             fabs(T.x*trans.m[1][0] + T.y*trans.m[1][1] + T.z*trans.m[1][2]);
	if (bOverlap.y<0.0f)
		return false;		
	
	bOverlap.z = aRadii.x*absTrans20 + aRadii.y*absTrans21 + aRadii.z*absTrans22 + bRadii.z -
	             fabs(T.x*trans.m[2][0] + T.y*trans.m[2][1] + T.z*trans.m[2][2]);
	if (bOverlap.z<0.0f)
		return false;		
	
	if (fabs(T.z*trans.m[0][1] - T.y*trans.m[0][2]) > 
				aRadii.y * absTrans02 + aRadii.z * absTrans01 +
				bRadii.y * absTrans20 + bRadii.z * absTrans10)
		return false;
	
	if (fabs(T.z*trans.m[1][1] - T.y*trans.m[1][2]) > 
				aRadii.y * absTrans12 + aRadii.z * absTrans11 +
				bRadii.x * absTrans20 + bRadii.z * absTrans00)
		return false;
	
	if (fabs(T.z*trans.m[2][1] - T.y*trans.m[2][2]) > 
				aRadii.y * absTrans22 + aRadii.z * absTrans21 +
				bRadii.x * absTrans10 + bRadii.y * absTrans00)
		return false;
	
	if (fabs(T.x*trans.m[0][2] - T.z*trans.m[0][0]) > 
				aRadii.x * absTrans02 + aRadii.z * absTrans00 +
				bRadii.y * absTrans21 + bRadii.z * absTrans11)
		return false;
	
	if (fabs(T.x*trans.m[1][2] - T.z*trans.m[1][0]) > 
				aRadii.x * absTrans12 + aRadii.z * absTrans10 +
				bRadii.x * absTrans21 + bRadii.z * absTrans01)
		return false;
	
	if (fabs(T.x*trans.m[2][2] - T.z*trans.m[2][0]) > 
				aRadii.x * absTrans22 + aRadii.z * absTrans20 +
				bRadii.x * absTrans11 + bRadii.y * absTrans01)
		return false;
	
	if (fabs(T.y*trans.m[0][0] - T.x*trans.m[0][1]) > 
				aRadii.x * absTrans01 + aRadii.y * absTrans00 +
				bRadii.y * absTrans22 + bRadii.z * absTrans12)
		return false;
	
	if (fabs(T.y*trans.m[1][0] - T.x*trans.m[1][1]) > 
				aRadii.x * absTrans11 + aRadii.y * absTrans10 +
				bRadii.x * absTrans22 + bRadii.z * absTrans02)
		return false;
	
	if (fabs(T.y*trans.m[2][0] - T.x*trans.m[2][1]) > 
				aRadii.x * absTrans21 + aRadii.y * absTrans20 +
				bRadii.x * absTrans12 + bRadii.y * absTrans02)
		return false;
	
	return true;
}

//------------------------------------------------------------------------------------
// like m_OBoxOBox but assumes aRadii.z = 0, saves about 10% of the *,+,-'s
//------------------------------------------------------------------------------------
bool m_rectOBox(const Point3F & aRadii, const Point3F & bRadii, const TMat3F & trans,
                        Point3F & aOverlap, Point3F & bOverlap)
{
	const Point3F & T = trans.p;

	float absTrans00,absTrans01,absTrans02;
	float absTrans10,absTrans11,absTrans12;
	float absTrans20,absTrans21,absTrans22;
	
	absTrans00 = fabs(trans.m[0][0]);
	absTrans10 = fabs(trans.m[1][0]);
	absTrans20 = fabs(trans.m[2][0]);

	aOverlap.x = aRadii.x + bRadii.x * absTrans00 + bRadii.y * absTrans10 + bRadii.z * absTrans20 - fabs(T.x);
	if (aOverlap.x<0.0f)
		return false;

	absTrans01 = fabs(trans.m[0][1]);
	absTrans11 = fabs(trans.m[1][1]);
	absTrans21 = fabs(trans.m[2][1]);
	aOverlap.y = aRadii.y + bRadii.x * absTrans01 +	bRadii.y * absTrans11 +	bRadii.z * absTrans21 - fabs(T.y);
	if (aOverlap.y<0.0f)
		return false;
	
	absTrans02 = fabs(trans.m[0][2]);
	absTrans12 = fabs(trans.m[1][2]);
	absTrans22 = fabs(trans.m[2][2]);
	aOverlap.z = bRadii.x * absTrans02 + bRadii.y * absTrans12 + bRadii.z * absTrans22 - fabs(T.z);
	if (aOverlap.z<0.0f)
		return false;

	bOverlap.x = aRadii.x*absTrans00 + aRadii.y*absTrans01+ bRadii.x -
	             fabs(T.x*trans.m[0][0] + T.y*trans.m[0][1] + T.z*trans.m[0][2]);
	if (bOverlap.x<0.0f)
		return false;

	bOverlap.y = aRadii.x*absTrans10 + aRadii.y*absTrans11+ bRadii.y -
	             fabs(T.x*trans.m[1][0] + T.y*trans.m[1][1] + T.z*trans.m[1][2]);
	if (bOverlap.y<0.0f)
		return false;		
	
	bOverlap.z = aRadii.x*absTrans20 + aRadii.y*absTrans21 + bRadii.z -
	             fabs(T.x*trans.m[2][0] + T.y*trans.m[2][1] + T.z*trans.m[2][2]);
	if (bOverlap.z<0.0f)
		return false;		
	
	if (fabs(T.z*trans.m[0][1] - T.y*trans.m[0][2]) > 
				aRadii.y * absTrans02 + 
				bRadii.y * absTrans20 + bRadii.z * absTrans10)
		return false;
	
	if (fabs(T.z*trans.m[1][1] - T.y*trans.m[1][2]) > 
				aRadii.y * absTrans12 + 
				bRadii.x * absTrans20 + bRadii.z * absTrans00)
		return false;
	
	if (fabs(T.z*trans.m[2][1] - T.y*trans.m[2][2]) > 
				aRadii.y * absTrans22 + 
				bRadii.x * absTrans10 + bRadii.y * absTrans00)
		return false;
	
	if (fabs(T.x*trans.m[0][2] - T.z*trans.m[0][0]) > 
				aRadii.x * absTrans02 + 
				bRadii.y * absTrans21 + bRadii.z * absTrans11)
		return false;
	
	if (fabs(T.x*trans.m[1][2] - T.z*trans.m[1][0]) > 
				aRadii.x * absTrans12 + 
				bRadii.x * absTrans21 + bRadii.z * absTrans01)
		return false;
	
	if (fabs(T.x*trans.m[2][2] - T.z*trans.m[2][0]) > 
				aRadii.x * absTrans22 + 
				bRadii.x * absTrans11 + bRadii.y * absTrans01)
		return false;
	
	if (fabs(T.y*trans.m[0][0] - T.x*trans.m[0][1]) > 
				aRadii.x * absTrans01 + aRadii.y * absTrans00 +
				bRadii.y * absTrans22 + bRadii.z * absTrans12)
		return false;
	
	if (fabs(T.y*trans.m[1][0] - T.x*trans.m[1][1]) > 
				aRadii.x * absTrans11 + aRadii.y * absTrans10 +
				bRadii.x * absTrans22 + bRadii.z * absTrans02)
		return false;
	
	if (fabs(T.y*trans.m[2][0] - T.x*trans.m[2][1]) > 
				aRadii.x * absTrans21 + aRadii.y * absTrans20 +
				bRadii.x * absTrans12 + bRadii.y * absTrans02)
		return false;
	
	return true;
}

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
                         float & overlap)
{
	const Point3F & T = trans.p;
	float planeDist = m_dot(n,p);
	float negBoxDist = m_dot(n,T);
	if (planeDist+negBoxDist<0.0f) // back-face
		return false;

	overlap = bRadii.x * fabs(trans.m[0][0] * n.x + trans.m[0][1] * n.y + trans.m[0][2] * n.z) +
	          bRadii.y * fabs(trans.m[1][0] * n.x + trans.m[1][1] * n.y + trans.m[1][2] * n.z) +
	          bRadii.z * fabs(trans.m[2][0] * n.x + trans.m[2][1] * n.y + trans.m[2][2] * n.z) -
	          planeDist+negBoxDist;

	return overlap>0.0f;
}
/*
bool m_overlapBPA(const Point3F & radii,const TMat3F & trans,
                          const Point3F * v,int n,Point3F & axis)
{
	// project box center onto axis (this is negative of actual value)
	float negBC = m_dot(trans.p,axis);

	// project all the poly points onto the axis, subtracting box center, and keeping only
	// one with min abs value
	float min = fabs(m_dot(v[0],axis)+negBC);
	int i;
	for (i=1;i<n;i++)
	{
		float tmpF = fabs(m_dot(v[i],axis)+negBC);
		if (tmpF<min)
			min=tmpF;
	}

	// now project box radii onto axis
	float rProj = 
	          radii.x * fabs(trans.m[0][0] * axis.x + trans.m[0][1] * axis.y + trans.m[0][2] * axis.z) +
	          radii.y * fabs(trans.m[1][0] * axis.x + trans.m[1][1] * axis.y + trans.m[1][2] * axis.z) +
	          radii.z * fabs(trans.m[2][0] * axis.x + trans.m[2][1] * axis.y + trans.m[2][2] * axis.z);

	return min<rProj;
}

bool m_polyOBox(const Point3F & radii, const TMat3F & trans,
               			const Point3F & normal, const Point3F * v, Point3F * e, int n,
                        float & overlap)
{
	// does poly plane intersect box
	if (!m_planeOBox(radii,trans,normal,v[0],overlap))
		return false;

	// check the box axes
	Point3F ba[3];
	int i;
	for (i=0;i<3;i++)
	{
		ba[i].x = trans.m[i][0];
		ba[i].y = trans.m[i][1];
		ba[i].z = trans.m[i][2];
		if (!m_overlapBPA(radii,trans,v,n,ba[i]))
			return false;
	}

	// check the poly axes
	int j;
	for (j=0;j<n;j++)
	{
		int k = i % n;
		e[j].x = v[k].x - v[j].x;
		e[j].y = v[k].y - v[j].y;
		e[j].z = v[k].z - v[j].z;
		Point3F en;
		m_cross(e[j],normal);
		
		if (!m_overlapBPA(radii,trans,v,n,en))
			return false;
	}

	// check axes orthoganol to one box axis and one poly axis
	for (i=0;i<3;i++)
		for (j=0;j<n;j++)
		{
			Point3F checkAxis;
			m_cross(ba[i],e[j],&checkAxis);
			if (!m_overlapBPA(radii,trans,v,n,checkAxis))
				return false;
		}

	return true;
}
*/
// warning: the following is a work in progress...not testeed yet
bool m_polyOBox(const Point3F & radii, const TMat3F & trans, const TMat3F & invTrans,
               			const Point3F & normal, const Point3F * v, Point3F * tv, int n,
                        float & overlap)
{
	// does poly plane intersect box
	if (!m_planeOBox(radii,trans,normal,v[0],overlap))
		return false;

	int i;
	for (i=0;i<n;i++)
		m_mul(v[i],invTrans,&tv[i]);

	float min;

	// x axis test
	min = fabs(v[0].x);
	for (i=1;i<n;i++)
	{
		float tmpF = fabs(v[i].x);
		if (tmpF<min) min=tmpF;
	}
	if (min>radii.x)
		return false;

	// y axis test
	min = fabs(v[0].y);
	for (i=1;i<n;i++)
	{
		float tmpF = fabs(v[i].y);
		if (tmpF<min) min=tmpF;
	}
	if (min>radii.y)
		return false;

	// z axis test
	min = fabs(v[0].z);
	for (i=1;i<n;i++)
	{
		float tmpF = fabs(v[i].z);
		if (tmpF<min) min=tmpF;
	}
	if (min>radii.z)
		return false;

	for (i=0;i<n;i++)
	{
		Point3F e;
		int k = (i+1) % n;
		e.x = tv[k].x-tv[i].x;
		e.y = tv[k].y-tv[i].y;
		e.z = tv[k].z-tv[i].z;
		Point3F abse;
		abse.x = fabs(e.x);
		abse.y = fabs(e.y);
		abse.z = fabs(e.z);

		if (fabs(tv[i].y*e.z-tv[i].z*e.y) > radii.y * abse.z + radii.z * abse.y)
			return false;

		if (fabs(tv[i].x*e.z-tv[i].z*e.x) > radii.x * abse.z + radii.z * abse.x)
			return false;

		if (fabs(tv[i].x*e.y-tv[i].y*e.x) > radii.x * abse.y + radii.y * abse.x)
			return false;

		Point3F ne;
		m_cross(e,normal,&ne);
		if (fabs(m_dot(tv[i],ne)) > radii.x * fabs(ne.x) + radii.y * fabs(ne.y) + radii.z * fabs(ne.z))
			return false;
	}
	
	// now test the poly's normal crossed w/ each box axis as the projection axis
	Point3F absN;
	absN.x = fabs(normal.x);
	absN.y = fabs(normal.y);
	absN.z = fabs(normal.z);
	min = fabs(tv[0].y*normal.z-tv[0].z*normal.y);
	for (i=1;i<n;i++)
	{
		float tmpF = fabs(tv[i].y*normal.z-tv[i].z*normal.y);
		if (tmpF<min) min=tmpF;
	}
	if (min > radii.y * absN.z + radii.z * absN.y)
		return false;

	min = fabs(tv[0].x*normal.z-tv[0].z*normal.x);
	for (i=1;i<n;i++)
	{
		float tmpF =fabs(tv[i].x*normal.z-tv[i].z*normal.x);
		if (tmpF<min) min=tmpF;
	}
	if (min > radii.x * absN.z + radii.z * absN.x)
		return false;

	min = fabs(tv[0].x*normal.y-tv[0].y*normal.x);
	for (i=1;i<n;i++)
	{
		float tmpF = fabs(tv[0].x*normal.y-tv[0].y*normal.x);
		if (tmpF<min) min=tmpF;
	}
	if (min > radii.x * absN.y + radii.y * absN.x)
		return false;

	return true;
}

//------------------------------------------------------------------------------------
// like m_OBoxOBox but assumes aRadii.z = 0, aRadii.x = 0, saves about 20% of the *,+,-'s
//------------------------------------------------------------------------------------
bool m_lineOBox(const Point3F & aRadii, const Point3F & bRadii, const TMat3F & trans,
                        Point3F & aOverlap, Point3F & bOverlap)
{
	const Point3F & T = trans.p;

	float absTrans00,absTrans01,absTrans02;
	float absTrans10,absTrans11,absTrans12;
	float absTrans20,absTrans21,absTrans22;
	
	absTrans00 = fabs(trans.m[0][0]);
	absTrans10 = fabs(trans.m[1][0]);
	absTrans20 = fabs(trans.m[2][0]);

	aOverlap.x = bRadii.x * absTrans00 + bRadii.y * absTrans10 + bRadii.z * absTrans20 - fabs(T.x);
	if (aOverlap.x<0.0f)
		return false;

	absTrans01 = fabs(trans.m[0][1]);
	absTrans11 = fabs(trans.m[1][1]);
	absTrans21 = fabs(trans.m[2][1]);
	aOverlap.y = aRadii.y + bRadii.x * absTrans01 +	bRadii.y * absTrans11 +	bRadii.z * absTrans21 - fabs(T.y);
	if (aOverlap.y<0.0f)
		return false;
	
	absTrans02 = fabs(trans.m[0][2]);
	absTrans12 = fabs(trans.m[1][2]);
	absTrans22 = fabs(trans.m[2][2]);
	aOverlap.z = bRadii.x * absTrans02 + bRadii.y * absTrans12 + bRadii.z * absTrans22 - fabs(T.z);
	if (aOverlap.z<0.0f)
		return false;

	bOverlap.x = aRadii.y*absTrans01+ bRadii.x -
	             fabs(T.x*trans.m[0][0] + T.y*trans.m[0][1] + T.z*trans.m[0][2]);
	if (bOverlap.x<0.0f)
		return false;

	bOverlap.y = aRadii.y*absTrans11+ bRadii.y -
	             fabs(T.x*trans.m[1][0] + T.y*trans.m[1][1] + T.z*trans.m[1][2]);
	if (bOverlap.y<0.0f)
		return false;		
	
	bOverlap.z = aRadii.y*absTrans21 + bRadii.z -
	             fabs(T.x*trans.m[2][0] + T.y*trans.m[2][1] + T.z*trans.m[2][2]);
	if (bOverlap.z<0.0f)
		return false;		
	
	if (fabs(T.z*trans.m[0][1] - T.y*trans.m[0][2]) > 
				aRadii.y * absTrans02 + 
				bRadii.y * absTrans20 + bRadii.z * absTrans10)
		return false;
	
	if (fabs(T.z*trans.m[1][1] - T.y*trans.m[1][2]) > 
				aRadii.y * absTrans12 + 
				bRadii.x * absTrans20 + bRadii.z * absTrans00)
		return false;
	
	if (fabs(T.z*trans.m[2][1] - T.y*trans.m[2][2]) > 
				aRadii.y * absTrans22 + 
				bRadii.x * absTrans10 + bRadii.y * absTrans00)
		return false;
	
	if (fabs(T.x*trans.m[0][2] - T.z*trans.m[0][0]) >
				bRadii.y * absTrans21 + bRadii.z * absTrans11)
		return false;
	
	if (fabs(T.x*trans.m[1][2] - T.z*trans.m[1][0]) > 
				bRadii.x * absTrans21 + bRadii.z * absTrans01)
		return false;
	
	if (fabs(T.x*trans.m[2][2] - T.z*trans.m[2][0]) > 
				bRadii.x * absTrans11 + bRadii.y * absTrans01)
		return false;
	
	if (fabs(T.y*trans.m[0][0] - T.x*trans.m[0][1]) > 
				aRadii.y * absTrans00 +
				bRadii.y * absTrans22 + bRadii.z * absTrans12)
		return false;
	
	if (fabs(T.y*trans.m[1][0] - T.x*trans.m[1][1]) > 
				aRadii.y * absTrans10 +
				bRadii.x * absTrans22 + bRadii.z * absTrans02)
		return false;
	
	if (fabs(T.y*trans.m[2][0] - T.x*trans.m[2][1]) > 
				aRadii.y * absTrans20 +
				bRadii.x * absTrans12 + bRadii.y * absTrans02)
		return false;
	
	return true;
}
