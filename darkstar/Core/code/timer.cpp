//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "Timer.h"


//------------------------------------------------------------------------------
Timer::Timer()
{
   LONGLONG freq64;

   // Get ticks-per-second of the performance counter.
   if (QueryPerformanceFrequency((LARGE_INTEGER*)&freq64))
   {  
      useQPC  = true;
      qpcFreq = static_cast<double>(freq64);
      qpcFreq = 1.0/qpcFreq;
   }
   else
   {  // fall back and use GetTick count :(
      useQPC  = false;
      AssertWarn(0,"Hardware does not support query performance counter.");
   }

   reset();   
}   


//------------------------------------------------------------------------------
void Timer::reset()
{
   // reset the counter.
   if (useQPC)
      QueryPerformanceCounter((LARGE_INTEGER*)&qpcStart);
   else
      gtcStart = GetTickCount();
}   
   
   
//------------------------------------------------------------------------------
// return seconds elapsed since last reset()
double Timer::getElapsed() const
{
   if (useQPC)    // use QuerryPerformanceCounter
   {
      LONGLONG end, delta64;
      QueryPerformanceCounter((LARGE_INTEGER*)&end);
      delta64 = end-qpcStart;
      return ((double)delta64*qpcFreq);
   }
   else           // fallback on GetTickCount
   {
      ULONG delta;
      delta = GetTickCount() - gtcStart;   
      return ((double)delta*(1.0/1000.0));   
   }
}   
   



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DeltaTimer::DeltaTimer()
{
   elapsed = 0;
   repeat  = false;
   tripped = 0;
}   

//------------------------------------------------------------------------------
void DeltaTimer::set(float p, bool r, float startScale)
{
   // the startScale member allows the timer to be started sometime within
   // the first period, instead of immediately.  This is useful for staggering
   // timers (e.g. by random(0.0, 1.0)) better ai behavior.

   period = p;
   repeat = r;      
   tripped = 0;
   elapsed = period*startScale;
}   


//------------------------------------------------------------------------------
void DeltaTimer::reset()
{
   elapsed = 0.0f;   
   tripped = 0;
}   

//------------------------------------------------------------------------------
bool DeltaTimer::update(float dt)
{
   elapsed += dt;

   if ( elapsed < period )
      return false;

   tripped = (tripped<<1)|1;

   elapsed -= period;

   if (!repeat)
      return (false);
   else
      return (tripped);
}   

//------------------------------------------------------------------------------
float DeltaTimer::getElapsed() const
{
   return (elapsed);
}   
