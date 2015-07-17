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

#ifndef _TPLANE_H_
#define _TPLANE_H_

#include <ml.h>

//---------------------------------------------------------------------------

#define TPlaneDefaultPrecision  float(0.00001)

inline bool isEqual(float a, float b, float tol = TPlaneDefaultPrecision)
{
	return fabs(a - b) <= tol;
}


//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

class TPlaneF: public Point3F
{
public:
	enum Side {
		OnPlane = 	0,
		Inside =		1,
		Outside =	-1,
		Intersect = 2,
		Undefined = 3,
	};

   typedef Poly3F             Poly;
   typedef LineSeg3F          LineSeg;
   typedef float              Value;
   typedef double       		Double;

	static float DistancePrecision;
	static float NormalPrecision;
	Value		d;

	TPlaneF() {}
	TPlaneF( const Point3F& p, const Point3F& n );
	TPlaneF( Value _x, Value _y, Value _z, Value _d);
	TPlaneF( const Point3F& j, const Point3F& k, const Point3F& l );

	// Methods

	void		neg();
	void		normalize();
	void		set( const Point3F& p, const Point3F& n);
	void		set( const Point3F& k, const Point3F& j, const Point3F& l );
	bool		set( int count, Point3F ** vl);

	void		setXY(Value zz);
	void		setYZ(Value xx);
	void		setXZ(Value yy);
	void		shiftX(Value xx);
	void		shiftY(Value yy);
	void		shiftZ(Value zz);

	Value		distance(const Point3F& cp) const;
	Side  	whichSide( const Point3F& cp ) const;
	bool		isFacing( const TPlaneF& plane) const;
	Bool 		intersect( const Point3F& start, const Point3F& end, Value* time ) const;
	Bool		intersect( const LineSeg& line, Point3F* ip ) const;
	Bool		split( const Poly& poly, Poly* front, Poly* back ) const;

	Bool 		isHorizontal() const;
	Bool 		isVertical() const;

	bool 		operator==(const TPlaneF& b) const;
	bool 		operator!=(const TPlaneF& b) const;
};


//---------------------------------------------------------------------------

inline TPlaneF::
   TPlaneF( Value _x, Value _y, Value _z, Value _d )
{
	x = _x; y = _y; z = _z; d = _d;
}

inline TPlaneF::TPlaneF( const Point3F& p, const Point3F& n )
{
	set(p,n);
}

inline TPlaneF::TPlaneF( const Point3F& j, const Point3F& k, const Point3F& l )
{
	set(j,k,l);
}

inline void TPlaneF::neg()
{
	Point3F::neg();
	d = -d;
}	

inline void TPlaneF::setXY( Value zz )
{
	x = y = 0; z = 1; d = -zz;
}

inline void TPlaneF::setYZ( Value xx )
{
	x = 1; z = y = 0; d = -xx;
}

inline void TPlaneF::setXZ( Value yy )
{
	x = z = 0; y = 1; d = -yy;
}

inline void TPlaneF::shiftX( Value xx )
{
	d -= xx * x;
}

inline void TPlaneF::shiftY( Value yy )
{
	d -= yy * y;
}

inline void TPlaneF::shiftZ( Value zz )
{
	d -= zz * z;
}

inline Bool TPlaneF::isHorizontal() const
{
	return (x == 0 && y == 0) ? TRUE : FALSE;
}

inline Bool TPlaneF::isVertical() const
{
    return ((x != 0 || y != 0) && z == 0) ? TRUE : FALSE;
}

inline TPlaneF::Value TPlaneF::distance( const Point3F& cp ) const
{
	return m_dot(*this,cp) + d;
}

inline TPlaneF::Side TPlaneF::whichSide( const Point3F& cp ) const
{
	Value sr = m_dot(*this,cp);
	return ((sr - DistancePrecision) > -d)? Inside:
		((sr + DistancePrecision) < -d)? Outside:
		OnPlane;
}

inline bool	TPlaneF::isFacing( const TPlaneF& plane) const
{
	return m_dot(*this,plane) < -TPlaneF::NormalPrecision;
}	


inline bool TPlaneF::operator==(const TPlaneF& b) const
{
	return (isEqual(m_dot(*this,b),1.0,TPlaneF::NormalPrecision) &&
		isEqual(d,b.d,TPlaneF::DistancePrecision));
}

inline bool TPlaneF::operator!=(const TPlaneF& b) const
{
	return !(*this == b);
}


inline void TPlaneF::normalize()
{
	// Only normalizes the vector component
	Double s = 1.0f / sqrt(Double(x) * x + Double(y) * y + Double(z) * z);
	x *= s;
	y *= s;
	z *= s;
}

//---------------------------------------------------------------------------
// TPlane
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	Calculate the coefficients of the plane passing through 
// a point with the given normal.

inline void TPlaneF::set( const Point3F& p, const Point3F& n )
{
   x = n.x; y = n.y; z = n.z;
   normalize();

	// Calculate the last plane coefficient.

	d = -(p.x * x + p.y * y + p.z * z);
}



#endif
