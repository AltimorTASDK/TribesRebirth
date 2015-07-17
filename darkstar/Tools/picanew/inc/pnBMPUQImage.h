//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PNBMPUQIMAGE_H_
#define _PNBMPUQIMAGE_H_

//Includes
#include "pnUnquantizedImage.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class GFXBitmap;

class PNBMPUQImage : public PNUnquantizedImage {

   PALETTEENTRY* m_pFinalPixels;

  public:
   PNBMPUQImage();
   ~PNBMPUQImage();

   const PALETTEENTRY* getFinalPixels() const;

   void registerColors(PNPopularityTable& in_rPopTable,
                       const UInt32       in_weight);
   bool loadFile(const char* in_pFileName);
};


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_PNBMPUQIMAGE_H_
