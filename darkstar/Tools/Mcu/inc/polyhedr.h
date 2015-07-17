#ifndef _POLYHEDR_H_
#define _POLYHEDR_H_

#include <math.h>
#include <stdio.h>   // needed by 3dsftk.h

#include <ml.h>
#include "3ds.h"
#include "status.h"

//-------------------------------------------------------------------------
//
// The structure polyhedron is used to store the geometry of the primitives
// used in this collision detection example.  Since the collision detection
// algorithm only needs to operate on the vertex set of a polyhedron, and
// no rendering is done in this example, the faces and edges of a
// polyhedron are not stored.  Adding faces and edges to the structure for
// rendering purposes should be straight forward and will have no effect on
// the collision detection computations.
//
//-------------------------------------------------------------------------

#define POLYHEDRON_ALLOC_SIZE 128

class Polyhedron
{
   Point3D  centroid;
   Point3D *pVerts; /* 3-D vertices of polyhedron. */
   int	   nVerts;			 /* number of 3-D vertices.  */
   int      nAllocVerts;
public:
   Polyhedron();
   Polyhedron( point3ds *_pVerts, int _nVerts );
   ~Polyhedron();

   void              Grow( int growAmount );
   void              AddVerts( point3ds *_pVerts, int _nVerts );
   void              AddVerts( Point3D *_pVerts, int _nVerts );
   void              AddVerts( Polyhedron * pPoly );
   void              AddVert( const Point3D &Vert );
   void              Scale( RealD scale );
   double            Hp( Point3D &A, int *pIndex );

   int               GetNVerts() const;
   const Point3D &   GetVert( int index ) const;
   const Point3D &   GetCentroid() const;
   RealD             GetRadius() const;
   void              Reset();
   void              Dump();
};

//-------------------------------------------------------------------------
//
// Initialize polyhedron with no vertices
//
//-------------------------------------------------------------------------

inline Polyhedron::Polyhedron() 
{
   nVerts = 0; 
   nAllocVerts = POLYHEDRON_ALLOC_SIZE; 
   pVerts = new Point3D[nAllocVerts]; 
   centroid.set();
   AssertFatal( pVerts != 0, "Out of memory!" );
}

//-------------------------------------------------------------------------
//
// Initialize polyhedron with the given vertices
//
//-------------------------------------------------------------------------

inline Polyhedron::Polyhedron( point3ds *_pVerts, int _nVerts )
{
   nVerts = 0; 
   pVerts = 0;
   nAllocVerts = 0; 
   centroid.set();
   AddVerts( _pVerts, _nVerts );
}

//-------------------------------------------------------------------------
//
// Free memory used by polyhedron
//
//-------------------------------------------------------------------------

inline Polyhedron::~Polyhedron()
{
   delete pVerts;
}

//-------------------------------------------------------------------------
//
// Remove all vertices but don't free memory
//
//-------------------------------------------------------------------------

inline void Polyhedron::Reset()
{
   centroid.set();
   nVerts = 0;
}

//-------------------------------------------------------------------------
//
// Reallocate as necessary to make room for more vertices
//
//-------------------------------------------------------------------------

inline void Polyhedron::Grow( int growAmount )
{
   if( nVerts + growAmount > nAllocVerts )
      {
      nAllocVerts = ((nVerts + growAmount + POLYHEDRON_ALLOC_SIZE - 1) / 
         POLYHEDRON_ALLOC_SIZE) * POLYHEDRON_ALLOC_SIZE;
      Point3D * newpVerts = new Point3D[nAllocVerts];
      AssertFatal( newpVerts, "Out of memory!" );
      memcpy( newpVerts, pVerts, nVerts * sizeof( Point3D ) );
      delete pVerts;
      pVerts = newpVerts;
      }
}

//-------------------------------------------------------------------------
//
// Add the vertices to the polyhedron
//
//-------------------------------------------------------------------------

inline void Polyhedron::AddVerts( Polyhedron * pPoly )
{
   AddVerts( pPoly->pVerts , pPoly->nVerts );
}

//-------------------------------------------------------------------------
//
// Add a single vertex to the polyhedron
//
//-------------------------------------------------------------------------

inline void Polyhedron::AddVert( const Point3D & Vert )
{
   Grow( 1 );
   pVerts[nVerts++] = Vert;
}

//-------------------------------------------------------------------------
//
// Misc access functions
//
//-------------------------------------------------------------------------

inline int Polyhedron::GetNVerts() const
{ 
   return nVerts; 
}
inline const Point3D & Polyhedron::GetVert( int index ) const
{ 
   return pVerts[index]; 
}

inline const Point3D & Polyhedron::GetCentroid() const
{
   return centroid;
}

#endif
