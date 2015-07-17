//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _M_RANDOM_H_
#define _M_RANDOM_H_

#include <base.h>
#include <types.h>
#include "m_base.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif


const unsigned int PRIME = 33655;

class DLLAPI Random
{
private:
   unsigned int buffer[250];
   unsigned int index;
   unsigned int seed;
   unsigned int quickRandSeed;
   float normalStore;
   bool  normalReady;

	UInt32 quickRand();
	
   enum Constants
   {
      BITS	      = 32,
      MSB         = 0x80000000L,
      MAX_UINT    = 0xffffffffL,
      HALF_RANGE  = 0x40000000L,
      STEP        = 7,
   };

public:
   Random( unsigned int seed = PRIME );
   void setSeed( unsigned int seed );
   unsigned int getSeed();

   UInt32 getInt();                       // 0   to MAX_UINT
   UInt32 getInt(UInt32 max);             // 0   to max
   Int32  getInt(int min, int max);       // min to max
   float  getFloat();                     // 0.0 to 1.0
   float  getFloat(float max);            // 0.0 to max
   float  getFloat(float min, float max); // min to max

   //--------------------------------------
   float getNormal();
   float getNormal(float mean, float dev);
   float getNormal(float mean, float dev, float min, float max );
};   


inline unsigned int Random::getSeed()
{
   return ( seed );
}

inline float Random::getFloat()
{
   return ((float)getInt() / (float)((unsigned int)MAX_UINT) );
}

inline UInt32 Random::getInt(UInt32 max)
{
   return m_muludiv(getInt(), max, MAX_UINT);   
}

inline Int32 Random::getInt(int min, int max)
{
   register int range = max - min;
   return m_muludiv(getInt(), range, MAX_UINT) + min;
}

//--------------------------------------
inline float Random::getFloat(float max)
{
   return ( getFloat() * max );   
}

inline float Random::getFloat(float min, float max)
{
   register float range = max-min;
   return ( (getFloat()*range) + min);
}

//--------------------------------------
inline float Random::getNormal(float mean, float dev)
{
   return ((getNormal()*dev) +mean);
}

// This generator produces normal random numbers
// between min and max.  Numbers are evenly distributed on either
// side of the_mean value.  i.e, there is a 50% chance the value
// will fall on either side of the mean.  The dev value is not a 
// standard deviation but lets you express how much the values 
// deviate from the mean.  Values of 1.0 produce a rather flat
// distribution of frequencies.  Values near 0.0 produce numbers
// that are closely centered around the mean;
inline float Random::getNormal(float mean, float dev, float min, float max )
{
   float val = getNormal(mean, dev);
   if ( val < 0.0 )
      return ( (mean-min)*val + mean );
   else
      return ( (max-mean)*val + mean );
}



#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_M_RANDOM_H_
