//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PNPOPTABLE_H_
#define _PNPOPTABLE_H_

//Includes
#include <base.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class PNOptions;
class GFXPalette;
struct quantVector;

class PNPopularityTable {
   
   bool m_inAlphaMode;

   int           m_numFixedColors;
   PALETTEENTRY* m_pFixedColors;

   int m_rBits;
   int m_gBits;
   int m_bBits;
   int m_aBits;

   UInt32 m_zeroColor;

   UInt32* m_pTable;
   UInt32  m_tableSize;

   UInt32 m_numUniqueColors;
   UInt32 m_totalWeight;

   bool   m_colorsQuantized;

   void translateIndexToQVectorRGB(const UInt32 in_index,
                                   quantVector& out_rVector);
   void translateIndexToQVectorRGBA(const UInt32 in_index,
                                    quantVector& out_rVector);
   void translatePEntryToQVectorRGB(const PALETTEENTRY& in_rPEntry,
                                    quantVector& out_rVector);
   void translatePEntryToQVectorRGBA(const PALETTEENTRY& in_rPEntry,
                                     quantVector& out_rVector);
   void registerRGBColor(const PALETTEENTRY& in_rEntry,
                         const UInt32 in_weight);
   void registerRGBAColor(const PALETTEENTRY& in_rEntry,
                          const UInt32 in_weight);
  public:
   explicit PNPopularityTable(const PNOptions& in_rOptions);
   ~PNPopularityTable();

  public:
   void prepFixedColors(const PNOptions&  in_rOptions,
                        const GFXPalette* in_pPal);
   bool quantizeColors(const PNOptions&  in_rOptions,
                       GFXPalette*       out_pPal);

  public:
   void registerRGBColors(const PALETTEENTRY* in_pPEntries,
                          const UInt32        in_numEntries,
                          const UInt32        in_weight);
   void registerRGBAColors(const PALETTEENTRY* in_pPEntries,
                           const UInt32        in_numEntries,
                           const UInt32        in_weight);
};


inline void
PNPopularityTable::registerRGBColor(const PALETTEENTRY& in_rEntry,
                                    const UInt32 in_weight)
{
   UInt32 index =
      ((in_rEntry.peRed   >> (8 - m_rBits)) << (m_gBits + m_bBits)) |
      ((in_rEntry.peGreen >> (8 - m_gBits)) << (m_bBits))           |
      ((in_rEntry.peBlue  >> (8 - m_bBits)) << (0));
   AssertFatal(index < m_tableSize, "Oops");

   // Make sure this isn't a zero color
   //
   if (index == m_zeroColor)
      return;

   if (m_pTable[index] == 0)
      m_numUniqueColors++;

   m_pTable[index] += in_weight;
   m_totalWeight   += in_weight;
}

inline void
PNPopularityTable::registerRGBAColor(const PALETTEENTRY& in_rEntry,
                                     const UInt32 in_weight)
{
   UInt32 index;
   
   if (in_rEntry.peFlags == 0) {
      // Want all 0 alpha colors to map to 0 alpha black...
      //
      index = 0;
   } else {
      index = 
         ((in_rEntry.peFlags >> (8 - m_aBits)) << (m_rBits + m_gBits + m_bBits)) |
         ((in_rEntry.peRed   >> (8 - m_rBits)) << (m_gBits + m_bBits))           |
         ((in_rEntry.peGreen >> (8 - m_gBits)) << (m_bBits))                     |
         ((in_rEntry.peBlue  >> (8 - m_bBits)) << (0));
   }

   if (m_pTable[index] == 0)
      m_numUniqueColors++;

   m_pTable[index] += in_weight;
   m_totalWeight   += in_weight;
}


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_PNPOPTABLE_H_
