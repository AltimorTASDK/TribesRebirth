//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "colorMatcher.h"

//------------------------------------------------------------------------------
//-------------------------------------- BASE CLASS
//
ColorMatcher::ColorMatcher()
 : m_pMatchPalette(NULL),
   m_matchStart(0),
   m_matchEnd(0)
{
   memset(m_chosenWeights, 0, sizeof(UInt32) * 256);
}

ColorMatcher::~ColorMatcher()
{
   m_pMatchPalette = NULL;
}

void
ColorMatcher::setMatchPalette(const GFXPalette*  in_pPalette,
                              const UInt32       in_matchStart,
                              const UInt32       in_matchEnd)
{
   AssertFatal(m_pMatchPalette == NULL, "Match palette already set...");
   AssertFatal(in_matchStart >= 0 && in_matchStart < 256, "Invalid match start");
   AssertFatal(in_matchEnd   >= in_matchStart && in_matchStart < 256, "Invalid match end");

   m_pMatchPalette = in_pPalette;
   m_matchStart    = in_matchStart;
   m_matchEnd      = in_matchEnd;
}

void
ColorMatcher::setZeroColor(const PALETTEENTRY& /*in_rZeroColor*/)
{
   //
}

//------------------------------------------------------------------------------
//-------------------------------------- RGB SPECIALIZATION
//
RGBColorMatcher::RGBColorMatcher()
{
   m_pMatchAlreadyFound = new UInt8[1 << 24];
   m_pMatchArray        = new UInt8[1 << 24];

   memset(m_pMatchAlreadyFound, 0, sizeof(UInt8) * (1 << 24));

   m_zeroColor = 0;
   m_zeroing   = false;
}

RGBColorMatcher::~RGBColorMatcher()
{
   delete [] m_pMatchAlreadyFound;
   delete [] m_pMatchArray;
   m_pMatchAlreadyFound = NULL;
   m_pMatchArray        = NULL;
}

void
RGBColorMatcher::setZeroColor(const PALETTEENTRY& in_rZeroColor)
{
   m_zeroing   = true;
   m_zeroColor = UInt32(in_rZeroColor.peRed   << 16) |
                 UInt32(in_rZeroColor.peGreen << 8)  |
                 UInt32(in_rZeroColor.peBlue  << 0);
}

UInt8
RGBColorMatcher::matchColor(const PALETTEENTRY& in_rMatch)
{
   UInt32 lookUpVal = UInt32(in_rMatch.peRed   << 16) |
                      UInt32(in_rMatch.peGreen << 8)  |
                      UInt32(in_rMatch.peBlue  << 0);

   // Zerocolor is always 0
   if (lookUpVal == m_zeroColor && m_zeroing == true)
      return 0;

   // Full black always maps to PEntry 0, unless there is a zeroColor that isn't black
   //
   if (lookUpVal == 0 && (lookUpVal == m_zeroColor || m_zeroing == false))
      return 0;

   if (m_pMatchAlreadyFound[lookUpVal] != 0) {
      return m_pMatchArray[lookUpVal];
   }

   UInt8 newMatch = 0;
   UInt32 minDist = 0x7fffffff;

   for (UInt32 i = m_matchStart; i <= m_matchEnd; i++) {
      // Colors that aren't the zero color aren't allowed to match 0
      if (i == 0 && (lookUpVal != m_zeroColor && m_zeroing == true))
         continue;

      UInt32 dist = ((int(in_rMatch.peRed)   - int(m_pMatchPalette->palette[0].color[i].peRed))   *
                     (int(in_rMatch.peRed)   - int(m_pMatchPalette->palette[0].color[i].peRed)))   +
                    ((int(in_rMatch.peGreen) - int(m_pMatchPalette->palette[0].color[i].peGreen)) *
                     (int(in_rMatch.peGreen) - int(m_pMatchPalette->palette[0].color[i].peGreen))) +
                    ((int(in_rMatch.peBlue)  - int(m_pMatchPalette->palette[0].color[i].peBlue))  *
                     (int(in_rMatch.peBlue)  - int(m_pMatchPalette->palette[0].color[i].peBlue)));
      if (dist < minDist) {
         minDist  = dist;
         newMatch = i;
      }
   }

   m_pMatchArray[lookUpVal]        = newMatch;
   m_pMatchAlreadyFound[lookUpVal] = 1;

   m_chosenWeights[newMatch] += 1;

   return newMatch;
}

//------------------------------------------------------------------------------
//-------------------------------------- RGB BITVECTOR SPECIALIZATION
//
RGBBVColorMatcher::RGBBVColorMatcher()
{
   m_pMatchAlreadyFound = new UInt8[1 << 24];
   m_pMatchArray        = new UInt8[1 << 24];

   memset(m_pMatchAlreadyFound, 0, sizeof(UInt8) * (1 << 24));
}

RGBBVColorMatcher::~RGBBVColorMatcher()
{
   delete [] m_pMatchAlreadyFound;
   delete [] m_pMatchArray;
   m_pMatchAlreadyFound = NULL;
   m_pMatchArray        = NULL;
}

UInt8
RGBBVColorMatcher::matchColor(const PALETTEENTRY& in_rMatch)
{
   UInt32 lookUpVal = UInt32(in_rMatch.peRed   << 16) |
                      UInt32(in_rMatch.peGreen << 8)  |
                      UInt32(in_rMatch.peBlue  << 0);

   // Full black always maps to PEntry 0
   if (lookUpVal == 0)
      return 0;

   if (m_pMatchAlreadyFound[lookUpVal] != 0) {
      return m_pMatchArray[lookUpVal];
   }

   Int16 newMatch = -1;
   UInt32 minDist = 0x7fffffff;
   for (UInt32 i = 0; i <= 256; i++) {
      if (m_pMatchPalette->isAllowed(i) == true) {
         UInt32 dist = ((int(in_rMatch.peRed)   - int(m_pMatchPalette->palette[0].color[i].peRed))   *
                        (int(in_rMatch.peRed)   - int(m_pMatchPalette->palette[0].color[i].peRed)))   +
                       ((int(in_rMatch.peGreen) - int(m_pMatchPalette->palette[0].color[i].peGreen)) *
                        (int(in_rMatch.peGreen) - int(m_pMatchPalette->palette[0].color[i].peGreen))) +
                       ((int(in_rMatch.peBlue)  - int(m_pMatchPalette->palette[0].color[i].peBlue))  *
                        (int(in_rMatch.peBlue)  - int(m_pMatchPalette->palette[0].color[i].peBlue)));
         if (dist < minDist) {
            minDist  = dist;
            newMatch = i;
         }
      }
   }
   AssertFatal(newMatch != -1, "No allowed matches?");

   m_pMatchArray[lookUpVal]        = newMatch;
   m_pMatchAlreadyFound[lookUpVal] = 1;

   m_chosenWeights[newMatch] += 1;

   return newMatch;
}

//------------------------------------------------------------------------------
//-------------------------------------- Alpha SPECIALIZATION
//
AlphaColorMatcher::AlphaColorMatcher()
{
   //
}

AlphaColorMatcher::~AlphaColorMatcher()
{
   //
}

UInt8
AlphaColorMatcher::matchColor(const PALETTEENTRY& in_rMatch)
{
   UInt8 newMatch = 0;
   UInt32 minDist = 0x7fffffff;

   // Alpha zero always matches 0
   //
   if (in_rMatch.peFlags == 0)
      return 0;

   for (UInt32 i = m_matchStart; i <= m_matchEnd; i++) {
      UInt32 dist = ((int(in_rMatch.peRed)   - int(m_pMatchPalette->palette[0].color[i].peRed))   *
                     (int(in_rMatch.peRed)   - int(m_pMatchPalette->palette[0].color[i].peRed)))   +
                    ((int(in_rMatch.peGreen) - int(m_pMatchPalette->palette[0].color[i].peGreen)) *
                     (int(in_rMatch.peGreen) - int(m_pMatchPalette->palette[0].color[i].peGreen))) +
                    ((int(in_rMatch.peBlue)  - int(m_pMatchPalette->palette[0].color[i].peBlue))  *
                     (int(in_rMatch.peBlue)  - int(m_pMatchPalette->palette[0].color[i].peBlue)))  +
                    ((int(in_rMatch.peFlags) - int(m_pMatchPalette->palette[0].color[i].peFlags)) *
                     (int(in_rMatch.peFlags) - int(m_pMatchPalette->palette[0].color[i].peFlags)));

      if (dist < minDist) {
         minDist  = dist;
         newMatch = i;
      }
   }

   m_chosenWeights[newMatch] += 1;

   return newMatch;
}

