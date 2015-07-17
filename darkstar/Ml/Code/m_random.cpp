//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <stdlib.h>

#include "m_random.h"


//------------------------------------------------------------------------------
Random::Random( unsigned int _seed )
{
   normalReady = false;
   setSeed( _seed );  
}


//  Quick random number generator from Num. Recipes (2nd, p284)
// because not all compilers support 32 bit random functions
// (MSVC)
//
UInt32 Random::quickRand()
{
	quickRandSeed = (UInt32(1664525) * quickRandSeed + UInt32(1013904223));
	return quickRandSeed;
}


//------------------------------------------------------------------------------
void Random::setSeed( unsigned int _seed )
{
   index = 0;
   seed = _seed;
	quickRandSeed = _seed;
   srand( seed );  
   int j;                 

   for (j=0; j<250 ; j++)       //initial buffer setup
      buffer[j] = quickRand();         

   for (j=0; j<250 ; j++)       //set some of the MSBs to 1
      if (quickRand() > HALF_RANGE)
           buffer[j] |= MSB;

   unsigned int msb  = MSB;         //turn on diagonal bit
   unsigned int mask = MAX_UINT;    //turn off leftmost bits
   int k;

   for (j=0; j<BITS; j++)
   {
      k = STEP * j + 3;          //select element to operate on
      buffer[k] &= mask;         //turn off bits left of the diagonal
      buffer[k] |= msb;          //turn on the diagonal bit
      mask >>= 1;
      msb  >>= 1;
   }
}


//------------------------------------------------------------------------------
UInt32 Random::getInt()
{
   register int j;
   register unsigned int new_rand;

   if (index >= 147) j = index-147;
   else              j = index+103;

   new_rand      = buffer[index] ^ buffer[j];
   buffer[index] = new_rand;

   if (index >= 249) index = 0;
   else              index++;

   return (new_rand);
}


//------------------------------------------------------------------------------
//Generates a Normal (or Gaussian) Deviate with mean=0 and standard devieation = 1
//algorithm taken from Numerical Recipies 2nd Edition p288.
float Random::getNormal()
{
   if (normalReady)      
   {
      normalReady = false;
      return (normalStore);
   }
   else
   {
      float v1, v2, w;
      normalReady = true;
      do
      {
         v1 = (2.0f * getFloat()) - 1.0f;
         v2 = (2.0f * getFloat()) - 1.0f;
         w = v1*v1 + v2*v2;
      }while (w >= 1.0 || w == 0.0);

		// Calls sqrt instead of m_sqrt because of linking
		// problems with with gfx dll.
      //w = float(sqrt( -2.0f * m_log2(w) / w ));
      w = float(sqrt( -2.0f * (log(w)/log(2)) / w ));
      normalStore = v2 * w;
      return ( v1 * w);
   }
}

