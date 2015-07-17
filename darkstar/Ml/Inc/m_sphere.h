//================================================================
//   
// $Workfile:   m_sphere.h  $
// $Revision:   1.2  $
// $Version$
//	
// DESCRIPTION:
//    Circle and Sphere Classes.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _M_SPHERE_H_
#define _M_SPHERE_H_


#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//----------------------------------------------------------------------------
// Circle Class

class CircleF
{
public:
	Point2F center;
	float	radius;

	CircleF() {}
	CircleF( const Point2F &c, const float r )
	{
		center = c;
		radius = r;
	}

   CircleF & adjustToEnclose( const Point2F& );
   CircleF & adjustToEnclose( const CircleF& );
};

//----------------------------------------------------------------------------
// Sphere Class

class SphereF
{
public:
	Point3F center;
	float	radius;

	SphereF() {}
   SphereF( const Point3F &c, const float r = 0.0f )
	{
		center = c;
		radius = r;
	}

   SphereF & adjustToEnclose( const Point3F& );
   SphereF & adjustToEnclose( const SphereF& );
   operator const Point3F & () const {  return center;  }
};


inline CircleF & CircleF::adjustToEnclose( const Point2F& p )
{
   float dist = m_dist( p, center );
   if( dist > radius )
   {
      float shift = (dist - radius) * 0.5f;
      RealF t = RealF(shift) / dist;
      radius += shift;
      Point2F p1 = p;
      p1 -= center;
      p1 *= t;
      center += p1;
   }
   return *this;
}

inline CircleF & CircleF::adjustToEnclose( const CircleF& c )
{
   float dist = m_dist( c.center, center );
   if( (dist + c.radius) > radius )
   {
      float shift = (c.radius + dist - radius) * .5f;
      radius += shift;

		// only adjust center if distance is significant:
		// (this avoids unpleasant divide by zero!)

		if( dist > (0.0000001 * radius) )
      {
			Point2F p1 = c.center;
			p1 -= center;
			p1 /= dist;
			p1 *= shift;
			center += p1;
		}
   }
   return *this;
}

inline SphereF & SphereF::adjustToEnclose( const Point3F& p )
{
   float dist = m_dist( p, center );
   if( dist > radius )
   {
      float shift = (dist - radius) * 0.5f;
      RealF t = RealF(shift) / dist;
      radius += shift;
      Point3F p1 = p;
      p1 -= center;
      p1 *= t;
      center += p1;
   }
   return *this;
}

inline SphereF & SphereF::adjustToEnclose( const SphereF& c )
{
   float dist = m_dist( c.center, center );
   if( (dist + c.radius) > radius )
   {
      float shift = (c.radius + dist - radius) * .5f;
      radius += shift;

		// only adjust center if distance is significant:
		// (this avoids unpleasant divide by zero!)

		if( dist > (0.0000001 * radius) )
		{
			Point3F p1 = c.center;
			p1 -= center;
			p1 /= dist;
			p1 *= shift;
			center += p1;
	   }
   }
   return *this;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif

