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

#ifndef _TSPLANE_H_
#define _TSPLANE_H_

#include <ml.h>

//---------------------------------------------------------------------------
namespace TS
{

#define TSPlaneDefaultPrecision  float(0.00001)

inline bool isEqual(float a, float b, float tol = TSPlaneDefaultPrecision)
{
	return fabs(a - b) < tol;
}


//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

class TSPlaneF: public Point3F
{
public:
	enum Side {
		OnPlane = 	0,
		Inside =		1,
		Outside =	-1,
		Intersect = 2,
		Undefined = 3,
	};

	static float DistancePrecision;
	static float NormalPrecision;
	float		d;

	TSPlaneF() {}
	TSPlaneF( const Point3F& p, const Point3F& n );
	TSPlaneF( float _x, float _y, float _z, float _d);
	TSPlaneF( const Point3F& j, const Point3F& k, const Point3F& l );

	// Methods

	void		neg();
	void		set( const Point3F& p, const Point3F& n);
	void		set( const Point3F& k, const Point3F& j, const Point3F& l );
	void		set( int count, Point3F ** vl);

	void		setXY(float zz);
	void		setYZ(float xx);
	void		setXZ(float yy);
	void		shiftX(float xx);
	void		shiftY(float yy);
	void		shiftZ(float zz);

	float		distance(const Point3F& cp);
	Side  	whichSide( const Point3F& cp );
	bool		isFacing( const TSPlaneF& plane);
	Bool 		intersect( const Point3F& start, const Point3F& end, float* time );
	Bool		intersect( const LineSeg3F& line, Point3F* ip );
	Bool		split( const Poly3F& poly, Poly3F* front, Poly3F* back );

	Bool 		isHorizontal();
	Bool 		isVertical();

	bool 		operator==(const TSPlaneF& b) const;
	bool 		operator!=(const TSPlaneF& b) const;
};


//-----------------------------------------------------------------------------
inline TSPlaneF::
   TSPlaneF( float _x, float _y, float _z, float _d )
{
	x = _x; y = _y; z = _z; d = _d;
}

inline TSPlaneF::TSPlaneF( const Point3F& p, const Point3F& n )
{
	set(p,n);
}

inline TSPlaneF::TSPlaneF( const Point3F& j, const Point3F& k, const Point3F& l )
{
	set(j,k,l);
}

inline void TSPlaneF::neg()
{
	Point3F::neg();
	d = -d;
}	

inline void TSPlaneF::setXY( float zz )
{
	x = y = 0; z = 1; d = -zz;
}

inline void TSPlaneF::setYZ( float xx )
{
	x = 1; z = y = 0; d = -xx;
}

inline void TSPlaneF::setXZ( float yy )
{
	x = z = 0; y = 1; d = -yy;
}

inline void TSPlaneF::shiftX( float xx )
{
	d -= xx * x;
}

inline void TSPlaneF::shiftY( float yy )
{
	d -= yy * y;
}

inline void TSPlaneF::shiftZ( float zz )
{
	d -= zz * z;
}

inline Bool TSPlaneF::isHorizontal()
{
	return (x == 0 && y == 0) ? TRUE : FALSE;
}

inline Bool TSPlaneF::isVertical()
{
    return ((x != 0 || y != 0) && z == 0) ? TRUE : FALSE;
}

inline float TSPlaneF::distance( const Point3F& cp )
{
	return m_dot(*this,cp) + d;
}

inline TSPlaneF::Side TSPlaneF::whichSide( const Point3F& cp )
{
	float sr = m_dot(*this,cp);
	return ((sr - DistancePrecision) > -d)? Inside:
		((sr + DistancePrecision) < -d)? Outside:
		OnPlane;
}

inline bool	TSPlaneF::isFacing( const TSPlaneF& plane)
{
	return m_dot(*this,plane) < -TSPlaneF::NormalPrecision;
}	


inline bool TSPlaneF::operator==(const TSPlaneF& b) const
{
	return (isEqual(m_dot(*this,b),1.0,TSPlaneF::NormalPrecision) &&
		isEqual(d,b.d,TSPlaneF::DistancePrecision));
}

inline bool TSPlaneF::operator!=(const TSPlaneF& b) const
{
	return !(*this == b);
}


//---------------------------------------------------------------------------
// TSPlane
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	Calculate the coefficients of the plane passing through 
// a point with the given normal.

inline void TSPlaneF::set( const Point3F& p, const Point3F& n )
{
   x = n.x; y = n.y; z = n.z;
   normalize();

	// Calculate the last plane coefficient.

	d = -(p.x * x + p.y * y + p.z * z);
}


};

#endif
