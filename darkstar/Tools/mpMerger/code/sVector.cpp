//-===================================================================
//
//        FILENAME: sVector.cpp
//
//  INITIAL AUTHOR: Dave Moore
//           NOTES: The matrix multiplication, dot product stuff, etc.,
//                   in these routines are rather hacky.  Maybe firm
//                   and pretty them up if time?
//
//  AUTOMATICALLY UPDATED SOURCESAFE INFORMATION
//  --------------------------------------------
//
//         $Author: Dmoore $
//           $Date: 1/26/98 10:37p $
//       $Revision: 6 $
//        $Archive: /Niqolas/code/sVector.cpp $
//
//  
//  
//-===================================================================
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include <base.h>

#include "sVector.h"


//------------------------------------------------------------------------------
//--------------------------------------
// Internal Module Data and Funcs.
//--------------------------------------
//
typedef struct _sortVector {
   const quantVector *pQVector;
   float sortKey;
} sortVector;

typedef struct _quantTableEntry {
   int          numVectors;
   float       totalWeight;
   float       standardDev;

   sortVector  *pVectors;

   // Just 'cause I'm a paranoid kinda guy...
   _quantTableEntry() : numVectors(0), pVectors(NULL),
      totalWeight(0.0), standardDev(-1.0) { }
} quantTableEntry;


//------------------------------------------------------------------------------
//-------------------------------------------
// local function Pre-decls. & static data...
//-------------------------------------------
//
extern "C" {
   float __cdecl
   asmGetStandardDev(const quantVector* pCenterOfMass,
                     const sortVector*  pVectors,
                     Int32              numVectors);
   void __cdecl
   asmCalcCenterOfMass(const quantVector* pCenterOfMass,
                       const sortVector*  pVectors,
                             Int32        numVectors,
                             float       totalWeight);
   void __cdecl
   asmCalcCovarMatrix(const float*     pCenterOfMass,
                      float**          pCaovarMatrix,
                      const UInt32      numDimension,
                      const sortVector* pVectors,
                      Int32             numVectors,
                      float            totalWeight);
   Int32 __cdecl
   asmFindClosestVectorNonEx(const quantVector* in_pTarget,
                             const quantVector* in_pSourceVectors,
                             const UInt32       in_numSourceVectors,
                             const UInt32       in_numDim);
}

namespace {

float getStandardDeviation(const quantTableEntry* in_pQuantEntry);
void   sortQuantEntry(quantTableEntry*  io_pQuantEntry);
void   splitQuantEntry(quantTableEntry* io_pQuantFirst,
                       quantTableEntry* out_pQuantSecond);
void   calculateCenterOfMass(const quantTableEntry* in_pQuantEntry,
                             quantVector*           out_pOutputVector);
void   calcCovarMatrix(const quantTableEntry* in_pQuantEntry,
                       float**               out_pCovarMatrix);
void   calcPrincipleEigenVector(const float** in_pMatrix,
                                float*        out_pEigenVector,
                                const int      in_dimension);
void   formDotProductKey(quantTableEntry* io_pQuantEntry,
                         const float*    in_pEigenVector);
void   createQuantTableEntry(quantTableEntry* io_pQuantEntry,
                             sortVector*      io_pSortVectors,
                             const int        in_numVectors);

//--------------------------------------
const int sg_maxNumIter = 100;
//--------------------------------------

//-=========================================================
//   Description: std qsort required compare function, sorts
//                 an array of sortVectors on their sortKeys.
//
//         Notes: 
//-=========================================================
int __cdecl
comparVector(const void *in_pVector1, const void *in_pVector2)
{
   sortVector *pVector1 = (sortVector *)in_pVector1;
   sortVector *pVector2 = (sortVector *)in_pVector2;
   
   if (pVector1->sortKey < pVector2->sortKey) {
      return -1;
   } else if (pVector1->sortKey == pVector2->sortKey) {
      return 0;
   } else {
      return 1;
   }
}

}; // namespace {}


//------------------------------------------------------------------------------
//--------------------------------------
// User callable functions
//--------------------------------------

//-=========================================================
//     Arguments: in_pVectors          - source vectors to be
//                                       quantized
//                in_numInputVectors   - number of input Vectors
//                out_pOutputVectors   - array for returned vectors
//                io_pNumOutputVectors - On input, contains
//                                  the quantization target,
//                                  i.e., the number of desired
//                                  "best" vectors.  On output,
//                                  contains the number that
//                                  were actually created.
//
//       Returns: VQ_SUCCESS or VQ_FAILURE
//
//   Description: Uses PCA to quantize the inputs to the
//                 "best" set of representative vectors.
//
//         Notes: Note that the CALLING function is responsible
//                 for allocating the memory assigned to the
//                 elements of the output vector array.  This
//                 function neither allocates nor free's any
//                 memory for that purpose.
//-=========================================================
int
quantizeVectors(const quantVector* in_pVectors,
                const Int32        in_numInputVectors,
                quantVector*       out_pOutputVectors,
                Int32&             io_rNumOutputVectors)
{
   // Sanity checks
   //
   AssertFatal(in_pVectors != NULL, "NULL vector input");
   AssertFatal(in_numInputVectors > 0, "Bad numVectors");
   AssertFatal(out_pOutputVectors != NULL, "output Array NULL");
   AssertFatal(io_rNumOutputVectors > 0, "Error, bad quant request");
   
   if (in_numInputVectors <= io_rNumOutputVectors) {
      // Well, in this case we have an easy time of it!
      //  Just copy the input vectors to the output
      //  array and be done with it!  (return rather.)
      //
      for (int i = 0; i < in_numInputVectors; i++) {
         out_pOutputVectors[i].numDim = in_pVectors[i].numDim;
         memcpy(out_pOutputVectors[i].pElem, in_pVectors[i].pElem,
                sizeof(float) * in_pVectors[i].numDim);
         out_pOutputVectors[i].weight = in_pVectors[i].weight;
      }
      io_rNumOutputVectors = in_numInputVectors;
      
      return VQ_SUCCESS;
   }

   // Ah well, on to the real work.  First, create the
   //  quantization table, with the appropriate number
   //  of entries, and insert the entire input set into
   //  the first entry.
   //
   quantTableEntry *pQuantEntries  = new quantTableEntry[io_rNumOutputVectors];
   int              currEndOfTable = 0;
   
   //  Create the sortVector array that we will use to sort the
   // input quantVectors
   //
   sortVector *pSortVectors = new sortVector[in_numInputVectors];
   for (int v = 0; v < in_numInputVectors; v++) {
      pSortVectors[v].pQVector = &in_pVectors[v];
   }

   // Create the base quantEntry
   //
   createQuantTableEntry(&pQuantEntries[0], pSortVectors,
                         in_numInputVectors);
   
   // Ok, now the meat of the process.  While there are fewer
   //  than the desired number of output entries, find the
   //  entry that maximizes (STD_DEV * numVectors), and
   //  split it into two sub-partitions. 
   // Note that we are guaranteed to always have a partition with
   //  > 1 entries at all times during this loop, so there is never
   //  a chance of trying to split a singular partition.  However,
   //  the danger exists that we could wind up with partitions that
   //  contain multilple copies of the same vector, so we check for
   //  zero STD_DEV, and abort at that point...
   //
   while (++currEndOfTable < io_rNumOutputVectors) {

      int maxEntry     = -1;
      float maxStdDev = -1.0;
   
      // Find "worst" table entry
      //
      for (int i = 0; i < currEndOfTable; i++) {
         float temp;
         if (pQuantEntries[i].standardDev < 0) {
            temp = getStandardDeviation(&pQuantEntries[i]);
            temp *= pQuantEntries[i].totalWeight;
            pQuantEntries[i].standardDev = temp;
         } else {
            temp = pQuantEntries[i].standardDev;
         }
         
         if (temp > maxStdDev) {
            maxStdDev = temp;
            maxEntry = i;
         }
      }
      
      // Check to make sure that we haven't homogenized our entries
      //
      const float minStdDev = 1e-10;
      if (maxStdDev <= minStdDev) {
         // Ok, break out of the while loop, we're done.
         //
         break;
      }
      
      sortQuantEntry(&pQuantEntries[maxEntry]);
      splitQuantEntry(&pQuantEntries[maxEntry],
                      &pQuantEntries[currEndOfTable]);
   }
   
   // Ok.  At this point, we have a number of partitioned vectors,
   //  in nice neat buckets, (currEndOfTable) in number.  Write
   //  the number of output vectors in io_rNumOutputVectors,
   //  extract the center of masses from the buckets, and put these
   //  into the output array.
   //
   io_rNumOutputVectors = currEndOfTable;
   
   for (int i = 0; i < io_rNumOutputVectors; i++) {
      calculateCenterOfMass(&pQuantEntries[i], &out_pOutputVectors[i]);
      out_pOutputVectors[i].weight = pQuantEntries[i].totalWeight;
   }
   
   // Yay!  Done and Successful!  Clean-up section...
   //
   delete [] pSortVectors;
   delete [] pQuantEntries;

   return VQ_SUCCESS;
}

//-=========================================================
//     Arguments: in_pTargetVector    - vector to be matched
//                in_pSourceVectors   - set of vectors to be
//                                       searched
//                in_numSourceVectors - rather obvious
//                out_pClosestIndex   - also rather obvious
//
//       Returns: VQ_SUCCESS or VQ_FAILURE
//
//   Description: Searches the input set, and finds the closest
//                 Euclidean match to the target vector.
//
//         Notes: 
//-=========================================================
int
findClosestVector(const quantVector& in_pTargetVector,
                  const quantVector* in_pSourceVectors,
                  const Int32        in_numSourceVectors,
                  Int32&             out_rClosestIndex,
                  const bool*        in_pExcludedVectors)
{
   // Sanity checks
   //
   AssertFatal(in_pSourceVectors != NULL, "Source vectors NULL");
   AssertFatal(in_numSourceVectors > 0, "Bad number of source vectors");
   AssertFatal(in_pTargetVector.numDim == in_pSourceVectors[0].numDim,
               "Dimension mismatch");
   
   // If we don't have any excludes, we can do this the easy way...
   //
   if (in_pExcludedVectors == NULL) {
      Int32 minIndex = 
         asmFindClosestVectorNonEx(&in_pTargetVector,
                                   in_pSourceVectors,
                                   in_numSourceVectors,
                                   in_pTargetVector.numDim);

      if (minIndex == -1)
         return VQ_FAILURE;
      else {
         out_rClosestIndex = minIndex;
         return VQ_SUCCESS;
      }
   }

   // Otherwise, gotta tough it out...
   //
   bool checkExcludes = false;
   if (in_pExcludedVectors != NULL)
      checkExcludes = true;
   
   int    minIndex = -1;
   float minDist  = 1e200;
   
   for (int i = 0; i < in_numSourceVectors; i++) {
      // note: uses short-circuit
      if (checkExcludes == true &&
          in_pExcludedVectors[i] == true) {
         continue;
      }

      float distance = 0;

      const quantVector &targetVector  = in_pTargetVector;
      const quantVector *sourceVectors = &in_pSourceVectors[i];
      for (int j = 0; j < in_pTargetVector.numDim; j++) {
         distance +=((targetVector.pElem[j] - sourceVectors->pElem[j]) *
                     (targetVector.pElem[j] - sourceVectors->pElem[j]));
      }
      
      if (distance < minDist) {
         minDist = distance;
         minIndex = i;
      }
   }
   
   if (minIndex == -1)
      return VQ_FAILURE;
   else {
      out_rClosestIndex = minIndex;

      return VQ_SUCCESS;
   }
}


namespace {

//------------------------------------------------------------------------------
//--------------------------------------
// Internal Support Functions
//--------------------------------------

//-=========================================================
//     Arguments: pQuantEntry - Entry to be checked
//
//       Returns: Standard deviation of the vectors distance
//                 from the Center of Mass
//
//   Description: See Returns field.
//
//         Notes: The definition of StdDev is ((x - |x|)^2)/N.
//                 Since we use the StdDev * N, technically,
//                 the final division in this routine could
//                 be left out, however, to avoid possible
//                 confusion, we use the traditional definition.
//-=========================================================
float
getStandardDeviation(const quantTableEntry *in_pQuantEntry)
{
   // Sanity checks...
   //
   AssertFatal(in_pQuantEntry != NULL, "NULL input");
   AssertFatal(in_pQuantEntry->numVectors > 0, "Bad number of Vectors");
   AssertFatal(in_pQuantEntry->pVectors != NULL, "Vector Pointer NULL");
   
   // Data
   //
   quantVector  centerOfMass;

   // Check the trivial case first...
   //
   if (in_pQuantEntry->numVectors == 1)
      return 0.0;

   // Ok, not trivial, calculate the CoM
   // We make some assumptions, like all vectors have the
   //  same dimension as the first.  Better be true...
   //
   centerOfMass.numDim = in_pQuantEntry->pVectors[0].pQVector->numDim;
   centerOfMass.pElem  = new float[centerOfMass.numDim];
   
   calculateCenterOfMass(in_pQuantEntry, &centerOfMass);
   float standardDev = ::asmGetStandardDev(&centerOfMass,
                                            in_pQuantEntry->pVectors,
                                            in_pQuantEntry->numVectors);
   standardDev /= in_pQuantEntry->totalWeight;

   // Clean-up section...
   //
   delete [] centerOfMass.pElem;
   centerOfMass.pElem = NULL;
   
   return standardDev;
}

//-=========================================================
//     Arguments: pQuantEntry - input list of vectors to be
//                 sorted
//
//       Returns: -
//
//   Description: The workhorse.  Calculates the Covariance
//                 matrix, its principle eigenvector, then
//                 sorts the vectors based on the value of
//                 the dot-product with the eigenvector.
//
//         Notes: For information on this algorithm, PCA, see
//                 articles by X. Wu in ACM Trans. on Graphics
//                 and in Graphics Gems III.
//                Maybe switch to (faster) binsort on vectors
//                 as suggested by X. Wu?   DMMNOTE
//-=========================================================
void
sortQuantEntry(quantTableEntry *io_pQuantEntry)
{
   // Sanity checks
   //
   AssertFatal(io_pQuantEntry != NULL, "NULL table");
   AssertFatal(io_pQuantEntry->pVectors != NULL, "NULL vector ptr");
   // We should never see a singular entry here...
   AssertFatal(io_pQuantEntry->numVectors > 1, "Bad numVectors");

   if (io_pQuantEntry->numVectors == 2) {
      // If there are only 2 vectors, it is possible that the
      //  covariance matrix will be singular, which is bad.  Since they'll
      //  be split down the middle no matter what, there is no need to
      //  sort...
      //
      return;
   }

   // Well, first order is to set up a proper matrix, and
   // a vector for the eigenvalue.
   //
   int      dimension   = io_pQuantEntry->pVectors[0].pQVector->numDim;
   int i;

   float*  eigenVector = new float  [dimension];
   float** covarMatrix = new float* [dimension];
   for (i = 0; i < dimension; i++) {
      covarMatrix[i] = new float[dimension];
   }

   // Now extract the covariance data from the vector set,
   //  and find the matrix's principle eigenvector.  Eigenvector
   //  returned is normalized in length.
   //
   calcCovarMatrix(io_pQuantEntry, covarMatrix);
   calcPrincipleEigenVector((const float **)covarMatrix, eigenVector, dimension);
   // This is slightly unusual, requesting const rather than tossing it... :)

   // Now form the dot product with all the vectors as a
   //  sort key, and call qsort to do our dirty work.
   // Note: for speed, replace qsort with a bin sort.
   // Note: Dot product is normalized to length of vector,
   //        forming a projection along the eigenvector,
   //        rather than the traditional meaning.
   //
   formDotProductKey(io_pQuantEntry, eigenVector);
   qsort(io_pQuantEntry->pVectors,
         io_pQuantEntry->numVectors,
         sizeof(sortVector),
         comparVector);

   // Clean-up section
   //
   delete [] eigenVector;

   for (i = 0; i < dimension; i++) {
      delete [] covarMatrix[i];
      covarMatrix[i] = NULL;
   }
   delete [] covarMatrix;
}

//-=========================================================
//     Arguments: io_pQuantFirst   - Source of the list to be
//                                    split, receipient of one
//                                    half the vectors
//                out_pQuantSecond - Receipient of the other
//                                    half
//
//       Returns: -
//
//   Description: Simply places half the vectors in one
//                 entry, and half in the other.
//
//         Notes: 
//-=========================================================
void
splitQuantEntry(quantTableEntry *io_pQuantFirst,
                quantTableEntry *out_pQuantSecond)
{
   // Sanity Checks
   //
   AssertFatal(io_pQuantFirst != NULL, "NULL table1");
   AssertFatal(io_pQuantFirst->pVectors != NULL, "NULL vector array");
   AssertFatal(out_pQuantSecond != NULL, "NULL table2");
   // Can't split a singular entry
   //
   AssertFatal(io_pQuantFirst->numVectors > 1, "Error, table has only one entry");
   
   int    splitPoint  = 0;
   float weightSoFar = 0.0;

   // ALWAYS at least one is assigned to the first quantTableEntry
   //
   do {
      weightSoFar += io_pQuantFirst->pVectors[splitPoint].pQVector->weight;
      splitPoint++;
   } while (weightSoFar < (io_pQuantFirst->totalWeight/2.0));

   // Also, one is ALWAYS assigned to the second entry...
   //
   if (splitPoint == io_pQuantFirst->numVectors) {
      splitPoint = io_pQuantFirst->numVectors - 1;
   }
   
   //  Assign the sets to the quantEntries
   //
   out_pQuantSecond->numVectors = io_pQuantFirst->numVectors - splitPoint;
   io_pQuantFirst->numVectors   = splitPoint;
   out_pQuantSecond->pVectors   = (io_pQuantFirst->pVectors +
                                   io_pQuantFirst->numVectors);

   //  This looks somewhat silly, but doing this after the above is the
   // simplest way to avoid overwriting intermediary values...
   //
   createQuantTableEntry(io_pQuantFirst, io_pQuantFirst->pVectors,
                         io_pQuantFirst->numVectors);
   createQuantTableEntry(out_pQuantSecond, out_pQuantSecond->pVectors,
                         out_pQuantSecond->numVectors);
}

//-=========================================================
//     Arguments: in_pQuantEntry    - Table to be analyzed
//                out_pOutputVector - obvious
// 
//       Returns: -
//
//   Description: Simply calculates the center of mass of a
//                 group of vectors, and returns it in the
//                 output vector.
//
//         Notes: Assumes that the memory for the output
//                 vector was allocated by the calling
//                 function, as well as the dimension
//-=========================================================
void
calculateCenterOfMass(const quantTableEntry* in_pQuantEntry,
                      quantVector*           out_pOutputVector)
{
   // Sanity checks
   //
   AssertFatal(in_pQuantEntry != NULL, "NULL table");
   AssertFatal(in_pQuantEntry->pVectors != NULL, "NULL vector ptr");
   AssertFatal(out_pOutputVector != NULL, "NULL output vetor");
   AssertFatal(out_pOutputVector->pElem != NULL, "NULL output elements");
   // Just to be sure...
   AssertFatal((in_pQuantEntry->pVectors[0].pQVector->numDim ==
                out_pOutputVector->numDim),
               "Error, dimension mismatch");
   AssertFatal(in_pQuantEntry->numVectors > 0, "Error, no vectors");
   
   // Clear out any garbage in the vector...
   for (int i = 0; i < out_pOutputVector->numDim; i++) {
      out_pOutputVector->pElem[i] = 0.0;
   }
   
   asmCalcCenterOfMass(out_pOutputVector,
                       in_pQuantEntry->pVectors,
                       in_pQuantEntry->numVectors,
                       in_pQuantEntry->totalWeight);
}

//-=========================================================
//     Arguments: io_pQuantEntry  - table of entries to be dotted
//                                   with input eigenvector
//                in_pEigenVector - obvious
//
//       Returns: -
//
//   Description: Forms the normalized dot product (projected
//                 length) along the given vector and assigns
//                 it to the sortKey field of the vectors
//
//         Notes: in_pEigenVector must be normalized...
//-=========================================================
void
formDotProductKey(      quantTableEntry *io_pQuantEntry,
                  const float          *in_pEigenVector)
{
   // Sanity checks
   //
   AssertFatal(io_pQuantEntry != NULL, "NULL table");
   AssertFatal(io_pQuantEntry->pVectors != NULL, "NULL vector ptr");
   AssertFatal(io_pQuantEntry->numVectors > 0, "No vectors!");
   AssertFatal(in_pEigenVector != NULL, "NULL eigenVector ptr");
   
   int dimension = io_pQuantEntry->pVectors[0].pQVector->numDim;
   
   for (int i = 0; i < io_pQuantEntry->numVectors; i++) {
      float dotProd = 0.0;
      
      for (int j = 0; j < dimension; j++) {
         dotProd += (io_pQuantEntry->pVectors[i].pQVector->pElem[j] *
                     in_pEigenVector[j]);
      }
      
      io_pQuantEntry->pVectors[i].sortKey = dotProd;
   }
}

//-=========================================================
// Function Name: calcPrincipleEigenVector
//
//     Arguments: in_pCovarMatrix  - matrix to extract eigen-
//                                    vector from
//                out_pEigenVector - obv.
//                in_dimension     - obv.
//
//       Returns: -
//
//   Description: Uses the "Power Method" to extract the
//                 eigenvector with the largest associated
//                 eigenvalue.
//
//         Notes: Maybe use a less iffy method?  DMMNOTE
//                Need some unobtrusive way to handle error
//                 case.  Move to Dynamix Assert Libs?
//-=========================================================

void
calcPrincipleEigenVector(const float **in_pMatrix,
                               float  *out_pEigenVector,
                         const int      in_dimension)
{
   // Sanity checks
   //
   AssertFatal(in_pMatrix != NULL, "NULL matrix!");
   AssertFatal(out_pEigenVector != NULL, "NULL eigenVector");
   AssertFatal(in_dimension > 1, "bad dimension");
   
   float *tempVector1  = new float[in_dimension];
   float *tempVector2  = new float[in_dimension];
   
   // Set up the entry vector
   //
   int i;
   float initValue =100.0;
   for (i = 0; i < in_dimension; i++) {
      tempVector1[i] = initValue;
      initValue -= 1.0f;
   }

   // Iterate, hopefully converging to the vector...
   //
   for (i = 0; i < sg_maxNumIter; i++) {
      // mult.
      //
      int j;
      for (j = 0; j < in_dimension; j++) {
         float temp = 0.0;
         for (int k = 0; k < in_dimension; k++) {
            temp += in_pMatrix[j][k] * tempVector1[k];
         }
         tempVector2[j] = temp;
      }

      // Find max (absolute) entry and divide...
      //
      float maxValue = 0.0;

      for (j = 0; j < in_dimension; j++) {
         if (fabs(tempVector2[j]) > fabs(maxValue)) {
            maxValue = tempVector2[j];
         }
      }
      for (j = 0; j < in_dimension; j++) {
         tempVector1[j] = tempVector2[j] / maxValue;
      }
   }
   
   // Find the length of the final value
   //
   float length = 0;
   for (i = 0; i < in_dimension; i++) {
      length += tempVector1[i] * tempVector1[i];
   }
   length = sqrt(length);
   
   // We should never wind up with a zero vector using
   //  this method unless there is an error in the matrix,
   //  i.e. it is singular...
   //
   AssertFatal(length != 0.0, "Length = 0!  Bad vector.");
   
   // Normalize the vector, and assign the values to the output,
   //  and that's all she wrote!
   //  
   for (i = 0; i < in_dimension; i++) {
      out_pEigenVector[i] = tempVector1[i] / length;
   }
   
   // clean-up section
   //
   delete [] tempVector1;
   delete [] tempVector2;
}

//-=========================================================
//     Arguments: in_pQuantEntry   - set of vectors to extract
//                                    the covariance data from
//                out_pCovarMatrix - obv.
//
//       Returns: -
//
//   Description: Calculates the StdDev in all directions, then
//                 constructs the covariance matrix for PCA.
//
//         Notes: Standard Deviation finding correct?
//-=========================================================
void
calcCovarMatrix(const quantTableEntry  *in_pQuantEntry,
                      float          **out_pCovarMatrix)
{
   // Sanity checks
   //
   AssertFatal(in_pQuantEntry != NULL, "NULL table");
   AssertFatal(in_pQuantEntry->pVectors != NULL, "NULL vector ptr");
   AssertFatal(in_pQuantEntry->numVectors > 0, "bad numvectors");
   AssertFatal(out_pCovarMatrix != NULL, "NULL matrix");
   
   int dimension = in_pQuantEntry->pVectors[0].pQVector->numDim;
   
   quantVector  centerOfMass;
   float      *sigma  = new float[dimension];

   centerOfMass.numDim = dimension;
   centerOfMass.pElem  = new float[dimension];
   
   int i;
   for (i = 0; i < dimension; i++) {
      centerOfMass.pElem[i] = sigma[i] = 0.0;
   }
   
   // Calculate the center of mass, and then the matrix
   //
   calculateCenterOfMass(in_pQuantEntry, &centerOfMass);

   asmCalcCovarMatrix(centerOfMass.pElem,
                      out_pCovarMatrix,
                      dimension,
                      in_pQuantEntry->pVectors,
                      in_pQuantEntry->numVectors,
                      in_pQuantEntry->totalWeight);

   // asmCalcCovarMatrix calculates the bottom left half of the matrix.  In other words,
   //  if the shape of the matrix is
   //    X X X
   //    X X X
   //    X X X
   //  then asm... calcs the x's in
   //    X 0 0
   //    X X 0
   //    X X X
   //  Since the Covar matrix is symmetric, we can copy the appropriate values to their
   //   spots...
   //
   for (i = dimension - 1; i > 0; i--) {
      for (int j = 0; j < i; j++) {
         out_pCovarMatrix[i][j] = out_pCovarMatrix[j][i];
      }
   }

   // clean-up
   //
   delete [] centerOfMass.pElem;
   delete [] sigma;
   centerOfMass.pElem = sigma = NULL;
}

//-=========================================================
//     Arguments: io_pQuantEntry   - obv.
//                io_pSortVectors  - vectors to be pointed to
//                in_numVectors    - obv.
//
//       Returns: -
//
//   Description: Simply makes sure that the quantTableEntry
//                 is properly constructed, and sets up the
//                 totalweight variable.
//-=========================================================
void
createQuantTableEntry(      quantTableEntry *io_pQuantEntry,
                            sortVector      *io_pSortVectors,
                      const int              in_numVectors)
{
   // Sanity Checks
   //
   AssertFatal(io_pQuantEntry != NULL, "Error, bad quantEntry pointer");
   AssertFatal(io_pSortVectors != NULL, "Error, no sortvectors");
   AssertFatal(in_numVectors >= 1, "Error, bad number of vectors");   

   io_pQuantEntry->pVectors    = io_pSortVectors;
   io_pQuantEntry->numVectors  = in_numVectors;
   io_pQuantEntry->totalWeight = 0.0;
   io_pQuantEntry->standardDev = -1.0;
   
   for (int i = 0; i < in_numVectors; i++) {
      io_pQuantEntry->totalWeight += io_pSortVectors[i].pQVector->weight;
   }
}

}; // namespace {}
