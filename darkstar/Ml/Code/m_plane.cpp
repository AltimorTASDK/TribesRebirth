//---------------------------------------------------------------------------

//	The coordinates used to generate the plane and the coodinates
//	that are tested against it must quite small to avoid overflow errors.

//	$Workfile:   m_plane.cpp  $
//	$Revision:   1.4  $
//	$Version$
//	$Date:   24 Oct 1995 10:55:32  $
//	$Log:   R:\darkstar\develop\core\ml\vcs\m_plane.cpv  $
//
//	   Rev 1.4   24 Oct 1995 10:55:32   CAEDMONI
//	made some args const
//	made set() inline to work around vulcan template bug
//
//	   Rev 1.3   23 Oct 1995 13:25:16   CAEDMONI
//	new templatized version
//
//	   Rev 1.2   12 Jul 1995 10:24:18   CAEDMONI
//	changed bool to Bool to agree with header file
//
//	   Rev 1.1   06 Jun 1995 11:19:50   CAEDMONI
//	changed to use m_muldiv (by TimG)
//
//	   Rev 1.0   07 Mar 1995 13:16:44   NANCYH
//	Initial revision.
//

//---------------------------------------------------------------------------

#include "ml.h"

//---------------------------------------------------------------------------
// Calculate the intersection point of the line segment
// and the plane. The method returns true if the objects
// intersect and the intersection point in *ip, false if
// they don't.

DLLAPI Bool PlaneF::intersect( const LineSeg3F& line, Point3F* ip )
{
	Point3F	dt;
	float		t,den;

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

DLLAPI Bool PlaneF::split( const Poly3F& poly, Poly3F* front, Poly3F* back )
{
	LineSeg3F line;
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
