//================================================================
//   
// $Workfile:   g_timer.h  $
// $Version$
// $Revision:   1.1  $
//   
// DESCRIPTION:
//   
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _G_TIMER_H_
#define _G_TIMER_H_

extern "C"
{
   void __cdecl gfx_push_time();
   Int32 __cdecl gfx_pop_time();
};
extern "C" Int32 timerVal;

#endif

