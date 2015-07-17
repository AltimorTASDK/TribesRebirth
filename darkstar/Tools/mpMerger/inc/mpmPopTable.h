//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _MPMPOPTABLE_H_
#define _MPMPOPTABLE_H_

//Includes
#include <map>
#include <base.h>
#include <g_pal.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class MPMOptions;
struct quantVector;

class PEntryLess {
  public:
   bool operator()(const PALETTEENTRY& in_r1, const PALETTEENTRY& in_r2) const {
      return ((float(in_r1.peRed) * 0.3 + float(in_r1.peGreen) * 0.6 + float(in_r1.peBlue) * 0.1) <
              (float(in_r2.peRed) * 0.3 + float(in_r2.peGreen) * 0.6 + float(in_r2.peBlue) * 0.1));
   }
};
typedef std::map<PALETTEENTRY, float, PEntryLess> EntryWeightMap;


class MPMPopularityTable {
   
   bool m_inAlphaMode;

   int           m_numFixedColors;
   PALETTEENTRY* m_pFixedColors;

   UInt32 m_zeroColor;

   UInt32* m_pTable;
   UInt32  m_tableSize;

   UInt32 m_numUniqueColors;
   UInt32 m_totalWeight;

   ColorF m_shadeColor;
   ColorF m_hazeColor;
   float m_shadePercentage;
   float m_hazePercentage;
   float m_shBiasFactor;
   UInt32 m_shadeLevels;
   UInt32 m_hazeLevels;

   bool   m_colorsQuantized;

   EntryWeightMap* m_pEntryWeightMap;

   void translatePEntryToQVectorRGB(const PALETTEENTRY& in_rPEntry,
                                    quantVector& out_rVector);
   void registerRGBColor(const PALETTEENTRY& in_rEntry,
                         const float         in_weight);

  public:
   explicit MPMPopularityTable(const MPMOptions& in_rOptions);
   ~MPMPopularityTable();

  public:
   void prepFixedColors(const MPMOptions& in_rOptions,
                        const GFXPalette* in_pPal);
   bool quantizeColors(const MPMOptions&  in_rOptions,
                       GFXPalette*        out_pPal);

  public:
   void registerRGBColors(const GFXPalette* in_pPal,
                          const UInt32      in_weight);
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_MPMPOPTABLE_H_
