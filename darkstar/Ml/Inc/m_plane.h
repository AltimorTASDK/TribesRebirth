//================================================================
//   
// $Workfile:   m_plane.h  $
// $Revision:   1.9  $
// $Version$
//	
// DESCRIPTION:
//    Plane classes.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

//---------------------------------------------------------------------------

#ifndef _M_PLANE_H_
#define _M_PLANE_H_


#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//---------------------------------------------------------------------------

typedef enum 
{
	MPlaneOn = 		0,
	MPlaneInside =	1,
	MPlaneOutside =	-1,
} MPlaneSide;

//---------------------------------------------------------------------------

class PlaneF: public Point3F
{
public:
	float		d;

	PlaneF();
	PlaneF( const Point3F& p, const Point3F& n );
	PlaneF( float _x, float _y, float _z, float _d);
	PlaneF( const Point3F& j, const Point3F& k, const Point3F& l );

	// Methods

	void		set( const Point3F& p, const Point3F& n);
	void		set( const Point3F& k, const Point3F& j, const Point3F& l );

	void		setXY(float zz);
	void		setYZ(float xx);
	void		setXZ(float yy);
	void		shiftX(float xx);
	void		shiftY(float yy);
	void		shiftZ(float zz);

	MPlaneSide  whichSide( const Point3F& cp );
	DLLAPI Bool		intersect( const LineSeg3F& line, Point3F* ip );
	DLLAPI Bool		split( const Poly3F& poly, Poly3F* front, Poly3F* back );

	Bool 		isHorizontal();
	Bool 		isVertical();
};

//---------------------------------------------------------------------------

inline PlaneF::PlaneF()
{
}

inline PlaneF::
   PlaneF( float _x, float _y, float _z, float _d )
{
	x = _x; y = _y; z = _z; d = _d;
}

inline PlaneF::PlaneF( const Point3F& p, const Point3F& n )
{
	set(p,n);
}

inline PlaneF::PlaneF( const Point3F& j, const Point3F& k, const Point3F& l )
{
	set(j,k,l);
}

inline void PlaneF::setXY( float zz )
{
	x = y = 0; z = 1; d = -zz;
}

inline void PlaneF::setYZ( float xx )
{
	x = 1; z = y = 0; d = -xx;
}

inline void PlaneF::setXZ( float yy )
{
	x = z = 0; y = 1; d = -yy;
}

inline void PlaneF::shiftX( float xx )
{
	d -= xx * x;
}

inline void PlaneF::shiftY( float yy )
{
	d -= yy * y;
}

inline void PlaneF::shiftZ( float zz )
{
	d -= zz * z;
}

inline Bool PlaneF::isHorizontal()
{
	return (x == 0 && y == 0) ? TRUE : FALSE;
}

inline Bool PlaneF::isVertical()
{
    return ((x != 0 || y != 0) && z == 0) ? TRUE : FALSE;
}

inline MPlaneSide PlaneF::whichSide( const Point3F& cp )
{
	float sr = m_dot(*this,cp);
	return (sr > -d)? MPlaneInside: (sr < -d)? MPlaneOutside: MPlaneOn;
}

//---------------------------------------------------------------------------
//	Calculate the coefficients of the plane passing through 
// a point with the given normal.

////inline void PlaneF::set( const Point3F& p, const Point3F& n )
inline void PlaneF::set( const Point3F& p, const Point3F& n )
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

inline void PlaneF::set( const Point3F& k, const Point3F& j, const Point3F& l )
{
	Point3F	kj,lj,pv;

   kj = k;
   kj -= j;
   lj = l;
   lj -= j;
   m_cross( kj, lj, &pv );

	set(j,pv);
}



#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


#endif
