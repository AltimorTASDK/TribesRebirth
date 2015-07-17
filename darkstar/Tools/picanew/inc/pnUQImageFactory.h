//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PNUQIMAGEFACTORY_H_
#define _PNUQIMAGEFACTORY_H_

//Includes
#include <base.h>
#include <pnUnquantizedImage.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class PNUQImageFactory {
   static PNUQImageFactory sm_theUQImageFactory;

   // Class is singleton
   //
   PNUQImageFactory();
  public:
   ~PNUQImageFactory();

   static PNUQImageFactory& getInstance();

   PNUnquantizedImage* createUQImage(const char* in_pFileName);
};


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_PNUQIMAGEFACTORY_H_
