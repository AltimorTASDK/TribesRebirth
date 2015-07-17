//
// esfEvDcl.h
//

#ifndef _ESFEVDCL_H_
#define _ESFEVDCL_H_

//--------------------------------------------------------------------------- 

// [AppIdRange,EsIdRange) reserved for common stuff
// [EsIdRange,FearIdRange) reserved for Earthsiege
// [FearIdRange,???) reserved for Fear
// similar for type ranges

#define EsIdRange               (AppIdRange + 100)
#define FearIdRange             (EsIdRange  + 100)
#define EsTypeRange             (AppTypeRange + 16)
#define FearTypeRange           (AppTypeRange  + 16)

//--------------------------------------------------------------------------- 

#define SimDamageEventType                SimRangeNum(AppTypeRange, 1)
#define SimMarkerNetEventType		         SimRangeNum(AppTypeRange, 2)
#define SimTriggerPhysicalNetEventType    SimRangeNum(AppTypeRange, 3)

// DON'T GO BEYOND SimRangeNum(AppIdRange, 16) !!

//--------------------------------------------------------------------------- 

#define SimTriggerLogId				SimRangeNum(AppIdRange, 1)
#define SimDropPointSetId		   SimRangeNum(AppIdRange,	2)  

#endif