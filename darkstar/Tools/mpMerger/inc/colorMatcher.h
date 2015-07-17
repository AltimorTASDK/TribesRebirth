//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _COLORMATCHER_H_
#define _COLORMATCHER_H_

//Includes
#include <g_pal.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class ColorMatcher {
  protected:
   GFXPalette const* m_pMatchPalette;
   UInt32            m_matchStart;
   UInt32            m_matchEnd;
   UInt32 m_chosenWeights[256];

  public:
   ColorMatcher();
   virtual ~ColorMatcher();

   void setMatchPalette(const GFXPalette*  in_pPalette,
                        const UInt32 in_matchStart,
                        const UInt32 in_matchEnd);
   virtual void setZeroColor(const PALETTEENTRY& in_rZeroColor);

   virtual UInt8 matchColor(const PALETTEENTRY& in_rMatch) = 0;

   const UInt32* getChosenFreqs() const { return m_chosenWeights; }
};


class RGBColorMatcher : public ColorMatcher {
   UInt8*   m_pMatchAlreadyFound;
   UInt8*   m_pMatchArray;

   UInt32   m_zeroColor;
   bool     m_zeroing;

  public:
   RGBColorMatcher();
   ~RGBColorMatcher();

   void setZeroColor(const PALETTEENTRY& in_rZeroColor);

   UInt8 matchColor(const PALETTEENTRY& in_rMatch);
};

//-------------------------------------- Same as RGB, but uses palettes BitVector...
//
class RGBBVColorMatcher : public ColorMatcher {
   UInt8*   m_pMatchAlreadyFound;
   UInt8*   m_pMatchArray;

  public:
   RGBBVColorMatcher();
   ~RGBBVColorMatcher();

   UInt8 matchColor(const PALETTEENTRY& in_rMatch);
};

class AlphaColorMatcher : public ColorMatcher {
  public:
    AlphaColorMatcher();
   ~AlphaColorMatcher();

   UInt8 matchColor(const PALETTEENTRY& in_rMatch);
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_COLORMATCHER_H_
