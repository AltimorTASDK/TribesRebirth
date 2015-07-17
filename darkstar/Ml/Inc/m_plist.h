//================================================================
//   
// $Workfile:   m_plist.h  $
// $Revision:   1.2  $
// $Version$
//	
// DESCRIPTION:
//    Point List and Poly functions.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _M_PLIST_H_
#define _M_PLIST_H_

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//---------------------------------------------------------------------------

class Poly3F
{
public:
   Int32 numPoints;
   Point3F *points;
};

class Poly3I
{
public:
   Int32 numPoints;
   Point3I *points;
};

//---------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif


