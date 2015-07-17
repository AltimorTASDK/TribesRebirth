#include "tplane.h"


float TPlaneF::DistancePrecision = TPlaneDefaultPrecision;
float TPlaneF::NormalPrecision = TPlaneDefaultPrecision;

//---------------------------------------------------------------------------
//	Calculate the coefficients of the plane passing through 
// three points.  Basically it calculates the normal to the three
// points then calculates a plane through the middle point with that
// normal.

void TPlaneF::set( const Point3F& k, const Point3F& j, const Point3F& l )
{
	Point3F	kj,lj,pv;

   kj = k;
   kj -= j;
   lj = l;
   lj -= j;
   m_cross( kj, lj, &pv );

	set(j,pv);
}


//---------------------------------------------------------------------------
// Build plane equation from list of points.
//
bool TPlaneF::set( int count, Point3F ** vl)
{
	Double normalx,normaly,normalz;
	normalx = normaly = normalz = Double(0);
	const Point3F *p0 = vl[count - 1];
	const Point3F *p1 = vl[0];

	for (int i = 0; i < count; i++) {
		normalx -= Double(p0->y - p1->y) * Double(p0->z + p1->z);
		normaly -= Double(p0->z - p1->z) * Double(p0->x + p1->x);
		normalz -= Double(p0->x - p1->x) * Double(p0->y + p1->y);
		p0 = p1;
		p1 = vl[i+1];
	}

	// Normalize
	Double den = normalx * normalx + normaly * normaly + normalz * normalz;
	if (den != .0f) {
		Double s = 1.0f / sqrt(den);
		x = normalx *= s;
		y = normaly *= s;
		z = normalz *= s;
		d = -(vl[0]->x * normalx + vl[0]->y * normaly + vl[0]->z * normalz);
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------
// Calculate the intersection point of the line segment
// and the plane. The method returns true if the objects
// intersect and the intersection time in *value, false if
// they don't. The time value is 0-1

Bool TPlaneF::intersect( const Point3F& start, const Point3F& end, Value* value ) const
{
	Point3F dt = end;
	dt -= start;

	// If den == 0 then the line and plane area parallel.
	Value den;
	if ((den = x * dt.x + y * dt.y + z * dt.z) == 0)
		return 0;
	Value t = -(x * start.x + y * start.y + z * start.z + d);

	// Make sure intersection point is in between the line
	// endpoints.
	if (den > 0) {
		if (t < 0 || t > den)
			return false;
	}
	else
		if (t > 0 || t < den)
			return false;
	// Should correct this for fixed point math...
	*value = t / den;
	return true;
}


//---------------------------------------------------------------------------
// Calculate the intersection point of the line segment
// and the plane. The method returns true if the objects
// intersect and the intersection point in *ip, false if
// they don't.

Bool TPlaneF::intersect( const LineSeg& line, Point3F* ip ) const
{
	Point3F	dt;
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

Bool TPlaneF::split( const Poly& poly, Poly* front, Poly* back ) const
{
	LineSeg line;
	Point3F	ip,*pcp,*fcp,*bcp;
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

