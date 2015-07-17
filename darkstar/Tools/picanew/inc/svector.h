//-===================================================================
//        FILENAME: sVector.h
//
//  INITIAL AUTHOR: Dave Moore
//           NOTES: 
//
//  AUTOMATICALLY UPDATED SOURCESAFE INFORMATION
//  --------------------------------------------
//
//         $Author: Dmoore $
//           $Date: 1/23/98 1:13a $
//       $Revision: 4 $
//        $Archive: /Niqolas/inc/svector.h $
//
//  
//  
//-===================================================================

#ifndef __SVECTOR_H__
#define __SVECTOR_H__

#include <types.h>

// Simple error codes for determining if the VQ
//  routines go south...
//
#define VQ_SUCCESS   0
#define VQ_FAILURE   1

// Just a simple, generic vector struct.  This is all that
//  external functions should need to see.  Matrices and
//  such else that we'll be using are entirely internal
//  to the sVector.cpp file.  Likewise with functions listed
//  after...
//
struct quantVector {
   Int32   numDim;
   float *pElem;
   float  weight;
};
 
// Attempts to quantize to the (io_pNumOutputVectors) "best"
//  vectors given the input data set.  For a more
//  detailed description of the inputs, see the function
//  headers.  Returns one of the values #define'd above.
//
int
quantizeVectors(const quantVector*  in_pVectors,
                const Int32         in_numInputVectors,
                quantVector*        out_pOutputVectors,
                Int32&              io_pNumOutputVectors);

// Finds the closest vector in a set to the input target
//  vector.  Returns the value in the out_pClosestIndex
//  field.  Returns one of the error codes from above.
//
int
findClosestVector(const quantVector& in_pTargetVector,
                  const quantVector* in_pSourceVectors,
                  const Int32        in_numSourceVectors,
                  Int32&             out_pClosestIndex,
                  const Bool*        in_pExcludedVectors = NULL);

#endif // __SVECTOR_H__



