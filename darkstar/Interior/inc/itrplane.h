//================================================================
//   
// $Workfile:   m_plane.h  $
// $Revision:   1.9  $
// $Version$
//	$Date:   25 Oct 1995 22:43:46  $
// $Log:   R:\darkstar\develop\core\ml\vcs\m_plane.h_v  $
//	
//	   Rev 1.9   25 Oct 1995 22:43:46   CAEDMONI
//	moved backwards compatibility typedefs here from m_back.h
//	
//	   Rev 1.8   24 Oct 1995 17:40:44   CAEDMONI
//	oops! PlaneR should have been PlaneD
//	
//	   Rev 1.7   24 Oct 1995 17:36:02   CAEDMONI
//	to fix vulcan specialization problem: if any specialization is required
//	for a method, only specialized  versions of the method are provided.
//	
//	   Rev 1.6   24 Oct 1995 11:33:06   CAEDMONI
//	another hack for vulcan
//	
//	   Rev 1.5   24 Oct 1995 11:23:46   CAEDMONI
//	changed specialization to work around vulcan bug
//	
//	   Rev 1.4   24 Oct 1995 10:55:12   CAEDMONI
//	made some args const
//	made set() inline to work around vulcan template bug
//	
//	   Rev 1.3   23 Oct 1995 18:11:08   CAEDMONI
//	changed set() to use normalizef() instead of normalize() to avoid overflow.
//	
//	   Rev 1.2   23 Oct 1995 13:23:00   CAEDMONI
//	new templatized version
//	
// DESCRIPTION:
//    Plane classes.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

//---------------------------------------------------------------------------

#ifndef _ITRPLANE_H_
#define _ITRPLANE_H_

#include <m_plist.h>
#include <m_lseg.h>
#include <m_dot.h>

//---------------------------------------------------------------------------

#define TPlaneDefaultPrecision  0.00001

inline float isEqual(float a, float b, float tol = TPlaneDefaultPrecision)
{
	return fabs(a - b) < tol;
}


//---------------------------------------------------------------------------

typedef enum 
{
	MPlaneOn = 		0,
	MPlaneInside =	1,
	MPlaneOutside =	-1,
} MPlaneSide;

//---------------------------------------------------------------------------

template <class _Point>
class TPlaneT: public _Point
{
public:
   typedef _Point             Point;
   typedef PolyT<Point>       Poly;
   typedef LineSeg3T<Point>   LineSeg;
   typedef Point::Value       Value;

	static float DistancePrecision;
	static float NormalPrecision;
	Value		d;

	TPlaneT();
	TPlaneT( const Point& p, const Point& n );
	TPlaneT( Value _x, Value _y, Value _z, Value _d);
	TPlaneT( const Point& j, const Point& k, const Point& l );

	// Methods

	void		set( const Point& p, const Point& n);
	void		set( const Point& k, const Point& j, const Point& l );

	void		setXY(Value zz);
	void		setYZ(Value xx);
	void		setXZ(Value yy);
	void		shiftX(Value xx);
	void		shiftY(Value yy);
	void		shiftZ(Value zz);

	MPlaneSide  whichSide( const Point& cp );
	Bool		intersect( const LineSeg& line, Point* ip );
	Bool		split( const Poly& poly, Poly* front, Poly* back );

	Bool 		isHorizontal();
	Bool 		isVertical();
};


//-----------------------------------------------------------------------------

tamplate <class _Point>
float TPlaneT<_Point>::DistancePrecision = TPlaneDefaultPrecision;
tamplate <class _Point>
float TPlaneT<_Point>::DistancePrecision = TPlaneDefaultPrecision;


//-----------------------------------------------------------------------------

#ifndef M_NO_FLOAT
typedef TPlaneT<Point3F> TPlaneF;
#endif

#ifndef M_NO_DOUBLE
typedef TPlaneT<Point3D> TPlaneD;
#endif

typedef TPlaneT<Point3I> TPlaneI;

#ifdef M_BACK
typedef TPlaneI TPlane;
#endif

//---------------------------------------------------------------------------

template <class _Point>
inline TPlaneT<_Point>::TPlaneT()
{
}

template <class _Point>
inline TPlaneT<_Point>::
   TPlaneT( Value _x, Value _y, Value _z, Value _d )
{
	x = _x; y = _y; z = _z; d = _d;
}

template <class _Point>
inline TPlaneT<_Point>::TPlaneT( const Point& p, const Point& n )
{
	set(p,n);
}

template <class _Point>
inline TPlaneT<_Point>::TPlaneT( const Point& j, const Point& k, const Point& l )
{
	set(j,k,l);
}

template <class _Point>
inline void TPlaneT<_Point>::setXY( Value zz )
{
	x = y = 0; z = 1; d = -zz;
}

template <class _Point>
inline void TPlaneT<_Point>::setYZ( Value xx )
{
	x = 1; z = y = 0; d = -xx;
}

template <class _Point>
inline void TPlaneT<_Point>::setXZ( Value yy )
{
	x = z = 0; y = 1; d = -yy;
}

template <class _Point>
inline void TPlaneT<_Point>::shiftX( Value xx )
{
	d -= xx * x;
}

template <class _Point>
inline void TPlaneT<_Point>::shiftY( Value yy )
{
	d -= yy * y;
}

template <class _Point>
inline void TPlaneT<_Point>::shiftZ( Value zz )
{
	d -= zz * z;
}

template <class _Point>
inline Bool TPlaneT<_Point>::isHorizontal()
{
	return (x == 0 && y == 0) ? TRUE : FALSE;
}

template <class _Point>
inline Bool TPlaneT<_Point>::isVertical()
{
    return ((x != 0 || y != 0) && z == 0) ? TRUE : FALSE;
}

template <class _Point>
inline MPlaneSide TPlaneT<_Point>::whichSide( const Point& cp )
{
	Value sr = m_dot(*this,cp);
	return (sr > -d)? MPlaneInside: (sr < -d)? MPlaneOutside: MPlaneOn;
}

//---------------------------------------------------------------------------
// TPlane
//---------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// specialize for TPlaneI

// The magic number is used to make sure that the plane
// calculations don't overflow.

#define	PLANE_MAGIC_NUMBER		15000


//template <class _Point>
//inline void TPlaneT<_Point>::set( const Point& p, const Point& n )
inline void TPlaneI::set( const Point3I& p, const Point3I& n )
{
	Int32 mag;  // should be Int64?

	// Get magnitude of the normal vector.
	// The len function, squares, then takes the root.  If the
	// pv numbers are greater than the magic number, the len 
	// function may overflow.  Lenf doesn't have that problem
	// but returns an aproximation.

	if (n.x > PLANE_MAGIC_NUMBER || n.x < -PLANE_MAGIC_NUMBER ||
		n.y > PLANE_MAGIC_NUMBER || n.y < -PLANE_MAGIC_NUMBER ||
		n.z > PLANE_MAGIC_NUMBER || n.z < -PLANE_MAGIC_NUMBER)
		mag = n.lenf();
	else
		mag = n.len();

	// Scale the vector to 2048 using 64bit intermediate

//	x = m_muldiv(n.x,2048,mag);
//	y = m_muldiv(n.y,2048,mag);
//	z = m_muldiv(n.z,2048,mag);
// yet another mess caused by vulcan:
   x = m_muldiv((Int32)n.x,(Int32)2048,(Int32)mag);
	y = m_muldiv((Int32)n.y,(Int32)2048,(Int32)mag);
	z = m_muldiv((Int32)n.z,(Int32)2048,(Int32)mag);

	// Calculate the last plane coefficient.

	d = -(p.x * x + p.y * y + p.z * z);
}

//---------------------------------------------------------------------------
//	Calculate the coefficients of the plane passing through 
// a point with the given normal.

//template <class _Point>
//inline void TPlaneT<_Point>::set( const Point& p, const Point& n )
inline void TPlaneF::set( const Point& p, const Point& n )
{
   x = n.x; y = n.y; z = n.z;
   normalize();

	// Calculate the last plane coefficient.

	d = -(p.x * x + p.y * y + p.z * z);
}


//---------------------------------------------------------------------------
//	Calculate the coefficients of the plane passing through 
// a point with the given normal.

//template <class _Point>
//inline void TPlaneT<_Point>::set( const Point& p, const Point& n )
inline void TPlaneD::set( const Point& p, const Point& n )
{
   x = n.x; y = n.y; z = n.z;
   normalize();

	// Calculate the last plane coefficient.

	d = -(p.x * x + p.y * y + p.z * z);
}


//---------------------------------------------------------------------------
//	Calculate the coefficients of the plane passing through 
// three points.  Basically it calculates the normal to the three
// points then calculates a plane through the middle point with that
// normal.

template <class _Point>
void TPlaneT<_Point>::set( const Point& k, const Point& j, const Point& l )
{
	Point	kj,lj,pv;

   kj = k;
   kj -= j;
   lj = l;
   lj -= j;
   m_cross( kj, lj, &pv );

	set(j,pv);
}


//---------------------------------------------------------------------------
// Calculate the intersection point of the line segment
// and the plane. The method returns true if the objects
// intersect and the intersection point in *ip, false if
// they don't.

template <class _Point>
Bool TPlaneT<_Point>::intersect( const LineSeg& line, Point* ip )
{
	Point	dt;
	Value		t,den;

   line.getVector( &dt );

	// If den == 0 then the line and plane area parallel.

	if ((den = x * dt.x + y * dt.y + z * dt.z) == 0)
		return 0;

	t = -(x * line.start.x + y * line.start.y + z * line.start.z + d);

	// Make sure intersection point is inbetween the line
	// endpoints.

	if (den > 0)
	{
		if (t < 0 || t > den)
			return 0;
	}
	else
		if (t > 0 || t < den)
			return 0;
	
	ip->x = line.start.x + m_muldiv(dt.x,t,den);
	ip->y = line.start.y + m_muldiv(dt.y,t,den);
	ip->z = line.start.z + m_muldiv(dt.z,t,den);

	return 1;
}


//-----------------------------------------------------------------------------
// This function will split convex or concave G_3d_polygons along a G_3d_plane.
// Concave G_3d_polygons can end up with some degenerate cases.
//
// Example of a degenerate concave polygon case.  The plane is the vertical
// line draw with '!'  then rest is the polygon.  The polygon should be
// split into three parts.
//
//	           !
//            !
//        +---!-------+
//        |   !       |
//        +---!--+    |
//            !  |    |
//        +---!--+    |
//        |   !       |
//		    +---!-------+
//            !
//            !
//
// The polygon gets split into two polygons.  The right polygon is OK. but
// the left one has some problems.
//
//        +---+   +-------+
//        |   |   |       |
//		    +---+	+--+    |
//            |      |    |
//        +---+   +--+    |
//        |   |   |       |
//        +---+   +-------+
//
// There would be 8 vertices on the left polygon as follows:
//
//        1---2
//        |   |
//        8---7
//            |
//        5---6
//        |   |
//        4---3
//
// The G_3d_segment 6-7 would overlap 2-3.
//

template <class _Point>
Bool TPlaneT<_Point>::split( const Poly& poly, Poly* front, Poly* back )
{
	LineSeg line;
	Point	ip,*pcp,*fcp,*bcp;
	Int32	ci,csign,psign,bcount,fcount;
	Int32	back_degen,front_degen;

	// Make sure the given poly is not degenerate.

	if (poly.numPoints < 3)
	{
		front->numPoints = 0;
	 	back->numPoints = 0;
		return 0;
	}

	// Assume both back & front are degenerate polygons.

	back_degen = front_degen = 1;

	// Perform the splitting.  The polygon is closed
	// by starting with the last point.

	line.start = poly.points[poly.numPoints - 1];
	psign = whichSide(line.start);

	fcp = front->points;
	bcp = back->points;
	pcp = poly.points;
	
	bcount = fcount = 0;

	for (ci = poly.numPoints; ci > 0; --ci)
	{
		// This assumes that whichSide() continues to return
		// -1 for outside, 0 on the plane and 1 for inside.

		line.end = *pcp++;
		csign = whichSide(line.end);

		switch(psign * 3 + csign)
		{
			case -4:		// P-, C-
			case -1:		// P., C-
				bcount++; *bcp++ = line.end;
				back_degen = 0;
				break;

			case 4:		// P+, C+
			case 1:		// P., C+
				fcount++; *fcp++ = line.end;
				front_degen = 0;
				break;

			case -3:		// P-, C.
			case 0:		// P., C.
			case 3:		// P+, C.
				bcount++; *bcp++ = line.end;
				fcount++; *fcp++ = line.end;
				break;

			case -2:		// P-, C+
				if (intersect(line,&ip))
				{
					bcount++; *bcp++ = ip;
					fcount++; *fcp++ = ip;
					fcount++; *fcp++ = line.end;
					front_degen = 0;
				}
				break;

			case 2:		// P+, C-
				if (intersect(line,&ip))
				{
					bcount++; *bcp++ = ip;
					fcount++; *fcp++ = ip;
					bcount++; *bcp++ = line.end;
					back_degen = 0;
				}
				break;
		}

		psign = csign;
		line.start = line.end;
	}

	// Check for degeneracy.
	// This check should be a little more complete.

	if (fcount < 3)
	{
		front_degen = 1;
		fcount = 0;
	}

	if (bcount < 3)
	{
		back_degen = 1;
		bcount = 0;
	}

	// If both front and back are degeneratem then I assume that
	// it is on the plane and default to the front half space.

	if (front_degen && back_degen)
	{
		pcp = poly.points;
		fcp = front->points;

		fcount = ci = poly.numPoints;

		for (; ci > 0; --ci)
			*fcp++ = *pcp++;
	}

	front->numPoints = fcount;
	back->numPoints = bcount;

	return fcount && bcount;
}


#endif
