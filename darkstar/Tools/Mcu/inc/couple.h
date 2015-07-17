#ifndef _COUPLE_H_
#define _COUPLE_H_

#include "polyhedr.h"

#define FUZZ	       (0.00001)
#define FUZZ2	       (0.00003)
#define EQZ(x)	       ( fabs((x)) < FUZZ ? 1 : 0 )

//-------------------------------------------------------------------------
//
// The class couple is used to store the information required to
// repeatedly test a pair of polyhedra for collision.  This information
// includes : a reference to each polyhedron,  a flag indicating if there
// is a cached prospective proper separating plane, two points for
// constructing the proper separating plane, and possibly a cached set
// of points from each polyhedron for speeding up the distance algorithm.
//
//-------------------------------------------------------------------------

class Couple
{
   Polyhedron* ppolyhedron1;	/* First polyhedron of collision test. */
   Polyhedron* ppolyhedron2;	/* Second polyhedron of collision test. */
   int	      vert_indx[4][2]; /* cached points for distance algorithm. */
   int	      n;		/* number of cached points, if any. */
   Point3D     pln_pnt1;	/* 1st point used to form separating plane. */
   Point3D     pln_pnt2;	/* 2nd point used to form separating plane. */
   RealD       lambda[4];

   RealD   Hs( Point3D &A, Point3D *pContact, int *pIndex1, int *pIndex2 );
   RealD   dist3d();

   RealD       fFuzz;
   inline      Bool  EqZ(RealD x)	    { return fabs((x)) < fFuzz ? 
                                             TRUE : FALSE; }
public:
   Couple( Polyhedron * pp1, Polyhedron * pp2 );

   void        Reset();
   Bool        Collision();
   void        GetPlane(Point3F *pNormal, RealF *pCoeff ) const;
};

inline Couple::Couple( Polyhedron * pp1, Polyhedron * pp2 )
{
   ppolyhedron1 = pp1;
   ppolyhedron2 = pp2;
   n  = 0;
}

inline void Couple::Reset()
{
   n  = 0;
}

inline void Couple::GetPlane(Point3F *pNormal, RealF *pCoeff ) const
{
   Point3F mid = pln_pnt2;
   mid += pln_pnt1;
   mid /= 2.0f;
   
   *pNormal = pln_pnt1;
   *pNormal -= pln_pnt2;
   pNormal->normalize();
   *pCoeff = m_dot(mid, *pNormal);
}

#endif
