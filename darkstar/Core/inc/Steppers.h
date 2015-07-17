//----------------------------------------------------------------------------
//
//    Steppers.h
//
// DistSquaredHelper<type of k>:
// A class to incrementally step through (x-0)^2, (x-k)^2, (x-2k)^2, etc.  
//
//----------------------------------------------------------------------------

#ifndef _STEPPERS_H_
#define _STEPPERS_H_
#include <types.h>
#ifdef __BORLANDC__
#pragma option -w-inl
#endif



// This uses fact that difference between two terms is -2kx plus k^2 times Nth
//    odd number.  T can be either float or an int type - but must be large 
//    enough to hold multiples of k^2 and should be signed.  
template<class T>
class DistSquaredHelper
{
      float    current, k2x;
      T        k_sqrd_odd, k_sqrd_twice;

   public:
      DistSquaredHelper ( float x, T k )    {   init ( x, k );   }
      DistSquaredHelper ()    {}
      
      operator float ()    { return current; }
      
      float init ( float x, T k )  {
         k2x =  k * 2.0 * x;
         k_sqrd_odd = k * k;
         k_sqrd_twice = k_sqrd_odd + k_sqrd_odd;
         return ( current = x * x );
      }

      float  next ( void )  {
         current += k_sqrd_odd;        
         k_sqrd_odd += k_sqrd_twice;   
         return ( current -= k2x );    
      }
};




// bounces back and forth between min and max in supplied number of 
// steps, changes on get(). debugging aid.  
class BounceBetween 
{
   private:
      float start, middle, end, cur, add;
      float sign;
   public:
      BounceBetween ( float min = 0.0, float max = 1.0, int steps = 24 )
      {
         sign = 1.0;
         if ( max < min ){
            max = (sign = -1.0) * max;
            min = -1.0 * min;
         }
         if ( steps < 3 )  steps = 3;
         add = ((middle = max) - (start = min)) / steps;
         cur = end = middle + (max - min);
      }
      float get ( void )
      {
         if ( (cur += add) >= end )
            return (cur = start) * sign;
         else if ( cur > middle )
            return (end - cur + start) * sign;
         return cur * sign;
      }
      float operator() (void)  {  return this->get();  }
};


// Bounce a couple of elements of type T having subtract and multiply and such
template <class T>
class BetterBouncer : public BounceBetween 
{
      T  Start, End;
      
   public:
   
      BetterBouncer ( T start, T end, int steps = 24 ) :
         BounceBetween ( 0.0, 1.0, steps )
            { 
               Start = start;
               End = end;
            }
      
      T operator() (void)
            {
               T  temp = End;
               temp -= Start;
               temp *= get();
               return ( temp += Start );
            }
};



#ifdef __BORLANDC__
#pragma option -winl.
#endif
#endif
