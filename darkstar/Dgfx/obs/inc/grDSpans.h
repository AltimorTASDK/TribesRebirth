//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GRDSPANS_H_
#define _GRDSPANS_H_

//Includes
#include <base.h>
#include "rDSpans.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

namespace Rendition {

typedef struct _spansetupC {
   UInt32 spanTag;

   _spansetupC() : spanTag(V_FIFO_DSPAN_C) { }
} GR_DSpanSetupC;

typedef struct _spansetupS {
   UInt32 spanTag;

   Int32 a0, dadx, dady;
   Int32 r0, drdx, drdy;
   Int32 g0, dgdx, dgdy;
   Int32 b0, dbdx, dbdy;

   _spansetupS() : spanTag(V_FIFO_DSPAN_S) { }
} GR_DSpanSetupS;

typedef struct _spansetupH {
   UInt32 spanTag;

   Int32 h0, dhdx, dhdy;
   
   _spansetupH() : spanTag(V_FIFO_DSPAN_H) { }
} GR_DSpanSetupH;

typedef struct _spansetupUV {
   UInt32 spanTag;

   Int32 u0, dudx, dudy;
   Int32 v0, dvdx, dvdy;

   _spansetupUV() : spanTag(V_FIFO_DSPAN_UV) { }
} GR_DSpanSetupUV;

typedef struct _spansetupUVS {
   UInt32 spanTag;

   Int32 a0, dadx, dady;
   Int32 r0, drdx, drdy;
   Int32 g0, dgdx, dgdy;
   Int32 b0, dbdx, dbdy;
   
   Int32 u0, dudx, dudy;
   Int32 v0, dvdx, dvdy;

   _spansetupUVS() : spanTag(V_FIFO_DSPAN_UVS) { }
} GR_DSpanSetupUVS;

typedef struct _spansetupUVH {
   UInt32 spanTag;

   Int32 u0, dudx, dudy;
   Int32 v0, dvdx, dvdy;
   Int32 h0, dhdx, dhdy;

   _spansetupUVH() : spanTag(V_FIFO_DSPAN_UVH) { }
} GR_DSpanSetupUVH;

typedef struct _spansetupUVQ {
   UInt32 spanTag;

   Int32 uQ0, duQdx, duQdy;
   Int32 vQ0, dvQdx, dvQdy;
   Int32 Q0,  dQdx, dQdy;

   _spansetupUVQ() : spanTag(V_FIFO_DSPAN_UVQ) { }
} GR_DSpanSetupUVQ;

typedef struct _spansetupUVQS {
   UInt32 spanTag;

   Int32 a0, dadx, dady;
   Int32 r0, drdx, drdy;
   Int32 g0, dgdx, dgdy;
   Int32 b0, dbdx, dbdy;
   
   Int32 uQ0, duQdx, duQdy;
   Int32 vQ0, dvQdx, dvQdy;
   Int32 Q0,  dQdx, dQdy;

   _spansetupUVQS() : spanTag(V_FIFO_DSPAN_UVQS) { }
} GR_DSpanSetupUVQS;

typedef struct _spansetupUVQH {
   UInt32 spanTag;
   
   Int32 uQ0, duQdx, duQdy;
   Int32 vQ0, dvQdx, dvQdy;
   Int32 Q0,  dQdx, dQdy;

   Int32 h0, dhdx, dhdy;

   _spansetupUVQH() : spanTag(V_FIFO_DSPAN_UVQH) { }
} GR_DSpanSetupUVQH;

typedef struct {
   UInt32 xy;   // 16.0 x << 16 | 16.0 y
   UInt32 cnt;  // count
} GR_DSpan;

static const UInt32 GR_POLYMAXSPANS = 2048;

}; // namespace Rendition

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GRDSPANS_H_
