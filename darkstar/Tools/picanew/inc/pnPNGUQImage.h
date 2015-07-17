//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PNPNGUQIMAGE_H_
#define _PNPNGUQIMAGE_H_

//Includes
#include "pnUnquantizedImage.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class PNPNGUQImage : public PNUnquantizedImage {

   PALETTEENTRY* m_pFinalPixels;
   bool          m_hasAlpha;

  public:
   PNPNGUQImage();
   ~PNPNGUQImage();

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

#endif //_PNPNGUQIMAGE_H_
