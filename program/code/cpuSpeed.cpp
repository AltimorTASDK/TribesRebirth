//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include "cpuSpeed.h"
#include "console.h"
#include "g_timer.h"

extern "C" int __cdecl exec_cpuid();

#pragma option -x

#define INTEL   0
#define AMD     1
#define CYRIX   2
#define UNKNOWN 3

#ifdef _MSC_VER
#define try       _try
#define except    _except
#endif

void getCPUMetrics(float *_speed, int *_Hz, int *_pref, int* _cpuType)
{
   struct CPUSpeeds
   {
      int   Hz;
      int   maxHz;
      int   pref;
   }cpuSpeeds[]=
   {
      { 60, 66, 133},
      { 90, 94, 133},
      {100,110, 133},
      {120,125, 133},
      {133,140, 133},
      {166,172, 166},
      {200,210, 200},
      {233,240, 233},
      {266,280, 266},
      {300,310, 300},
      {333,340, 333},
      {350,355, 333},
      {400,410, 333},
      {450,460, 333},
      {500,510, 333},
      {533,550, 333},
      {566,570, 333},
      {600,610, 333},
      {0, 0, 0},
   };  

   float speed;
   try {
      gfx_push_time();
      DWORD clicks = GetTickCount();
      while ( GetTickCount() < clicks+500 );
      clicks = GetTickCount()-clicks;
      DWORD time = gfx_pop_time();

      speed =  ((float)time/(float)clicks)/1000;
      if ( _speed ) *_speed = speed;
   }
   except(EXCEPTION_EXECUTE_HANDLER)
   {
      speed = -1;
      if ( _speed ) *_speed = speed;
   }

   int cpuType;
   try {
      cpuType = exec_cpuid();
   } except (EXCEPTION_EXECUTE_HANDLER) {
      cpuType = UNKNOWN;
   }
   if (_cpuType) *_cpuType = cpuType;
   
   if (speed < 0)
   {
      if ( _Hz )   *_Hz   = -1;
      if ( _pref ) *_pref = -1;
   }
   else
   {
      int cpu=0;
      while ( cpuSpeeds[cpu].Hz && speed>cpuSpeeds[cpu].maxHz) 
         cpu++;
      if ( _Hz )   *_Hz   = cpuSpeeds[cpu].Hz;
      if ( _pref ) *_pref = cpuSpeeds[cpu].pref;
   }
}  

void
setCPUConsoleVars()
{
   float speed;
   int   hz;
   int   pref;
   int   cpuType;

   static const char* s_vendorStrings[] = {
      "Intel",
      "AMD",
      "Cyrix",
      "Unknown"
   };

   getCPUMetrics(&speed, &hz, &pref, &cpuType);
   const char* pName = s_vendorStrings[cpuType];

   Console->setIntVariable("$CPU::estimatedSpeed",     hz);
   Console->setIntVariable("$CPU::recommendPrefSpeed", pref);
   Console->setVariable("$CPU::Vendor", pName);
}
