//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _TIMER_H_
#define _TIMER_H_

//Includes
#include "base.h"
#include <windows.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif



//------------------------------------------------------------------------------
class Timer
{
private:
   bool useQPC;         // use query performance counter?

   LONGLONG qpcStart;   // query performance counter
   double   qpcFreq;

   ULONG    gtcStart;   // get tick count

public:
   Timer();
   void   reset();
   double getElapsed() const;
};


//------------------------------------------------------------------------------
class DeltaTimer
{
private:
   float elapsed;
   float period;
   bool  repeat;
   Int32 tripped;

public:
   DeltaTimer();

   void set(float period, bool repeat, float startScale = 0.0f);
   void reset();
   bool update(float dt);
   float getElapsed() const;
   operator bool() {int tmp = tripped; tripped >>= 1; return (tmp&1);}   
};




#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_TIMER_H_

