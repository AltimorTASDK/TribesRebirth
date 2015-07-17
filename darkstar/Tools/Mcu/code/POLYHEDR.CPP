/*
 * Based on C code from the article
 * "Fast Collision Detection of Moving Convex Polyhedra"
 * by Rich Rabbitz, rrabbitz%pgn138fs@serling.motown.ge.com
 * in "Graphics Gems IV", Academic Press, 1994
 */

#include "stdafx.h"
#include "polyhedr.h"

//-------------------------------------------------------------------------
//
// Add the vertices to the polyhedron
//
//-------------------------------------------------------------------------
#define FUZZ 0.05

void Polyhedron::AddVerts( point3ds *_pVerts, int _nVerts )
{
   Grow( _nVerts );
   for (int i = 0; i < _nVerts; i++)
      {
      for( int j = 0; j < nVerts; j++ )
         if( fabs(pVerts[j].x - _pVerts[i].x) < FUZZ &&
             fabs(pVerts[j].y - _pVerts[i].y) < FUZZ &&
             fabs(pVerts[j].z - _pVerts[i].z) < FUZZ )
            break;
      if( !(j < nVerts ) )
         {
         pVerts[nVerts].x = _pVerts[i].x;
         pVerts[nVerts].y = _pVerts[i].y;
         pVerts[nVerts].z = _pVerts[i].z;
         nVerts++;
         }
      }
}

void Polyhedron::AddVerts( Point3D *_pVerts, int _nVerts )
{
   Grow( _nVerts );
   for (int i = 0; i < _nVerts; i++)
      {
      for( int j = 0; j < nVerts; j++ )
         if( fabs(pVerts[j].x - _pVerts[i].x) < FUZZ &&
             fabs(pVerts[j].y - _pVerts[i].y) < FUZZ &&
             fabs(pVerts[j].z - _pVerts[i].z) < FUZZ )
            break;
      if( !(j < nVerts ) )
         {
         pVerts[nVerts] = _pVerts[i];
         nVerts++;
         }
      }
}

//-------------------------------------------------------------------------
//
// Scale the polyhedron about its centroid by the given scale factor
//
//-------------------------------------------------------------------------

void Polyhedron::Scale( RealD scale )
{
   // first, find centroid:
   centroid.set();
   for( int i = 0; i < nVerts; i++ )
      centroid += pVerts[i];
   centroid /= nVerts;

   // second, scale all points about centroid:
   for( i = 0; i < nVerts; i++ )
      {
      pVerts[i] -= centroid;
      pVerts[i] *= scale;
      pVerts[i] += centroid;
      }
}

//-------------------------------------------------------------------------

RealD Polyhedron::GetRadius() const
{
   if( !nVerts )
      return 0.0;

   Point3D minVal, maxVal;
   minVal = pVerts[0];
   maxVal = pVerts[0];
   for( int i = 1; i < nVerts; i++ )
      {
      minVal.setMin( pVerts[i] );
      maxVal.setMax( pVerts[i] );
      }
   maxVal -= minVal;
   return maxVal.len() / 2.0;
}

//-------------------------------------------------------------------------
//
// Function to evaluate the support and contact functions at A for a given
// polytope. See equations (6) & (7) in the Graphics Gem.
//
// On Entry:
//	   A        - vector at which support and contact functions will be 
//                  evaluated.
//	   pIndex   - pointer to an int.
//
// On Exit:
//	   pIndex   - index into P of contact point of P w.r.t. A.
//
// Function Return :
//	   the result of the evaluation of eq. (6) for A.
//
//-------------------------------------------------------------------------

RealD Polyhedron::Hp( Point3D &A, int *pIndex )
{
   Point3D *pVert = pVerts;
   RealD max_val = m_dot( *pVert++, A );
   *pIndex = 0;

   for( int i = 1; i < nVerts; i++ )
      {
      RealD val = m_dot( *pVert++, A );
      if (val > max_val)
         {
	      *pIndex = i;
	      max_val = val;
         }
      }
   return max_val;
}

//-------------------------------------------------------------------------
// Dump the contents of the polyhedron to stdout
//-------------------------------------------------------------------------

void Polyhedron::Dump()
{
   StatusPrintf( "Polyhedron: %d vertices, %d allocated vertices", 
      nVerts, nAllocVerts );
   StatusPrintf( "Centroid: %8.8lf, %8.8lf, %8.8lf",
      centroid.x, centroid.y, centroid.z );
   for( int i = 0; i < nVerts; i++ )
      StatusPrintf( "%8.8lf, %8.8lf, %8.8lf", 
         pVerts[i].x, pVerts[i].y, pVerts[i].z );
}

