//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _MPMMERGER_H_
#define _MPMMERGER_H_

//Includes
#include <base.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class GFXPalette;
class MPMOptions;
class RGBBVColorMatcher;

class MPMMerger {
   ColorF m_shadeColor;
   float  m_shadePercentage;
   ColorF m_hazeColor;
   float  m_hazePercentage;

   UInt32 m_numShadeLevels;
   UInt32 m_numHazeLevels;

   void createCommonTables(RGBBVColorMatcher&        in_rRGBMatcher,
                           GFXPalette::MultiPalette& out_rMPalette);
   void calcShadeHazeTable(RGBBVColorMatcher&        in_rRGBMatcher,
                           UInt8*                    out_pTable,
                           GFXPalette::MultiPalette& out_rMPalette,
                           const UInt32              in_shade,
                           const UInt32              in_haze);
   void calcHazeTable(RGBBVColorMatcher&        in_rRGBMatcher,
                      UInt8*                    out_pTable,
                      GFXPalette::MultiPalette& out_rMPalette,
                      const int                 in_haze);
   void createAlphaTable(RGBBVColorMatcher&        in_rRGBMatcher,
                         GFXPalette::MultiPalette& out_rMPalette,
                         const GFXPalette*         in_pMasterPalette);
   void createAdditiveTable(RGBBVColorMatcher&        in_rRGBMatcher,
                            GFXPalette::MultiPalette& out_rMPalette,
                            const GFXPalette*         in_pMasterPalette);
   void createSubtractiveTable(RGBBVColorMatcher&        in_rRGBMatcher,
                               GFXPalette::MultiPalette& out_rMPalette,
                               const GFXPalette*         in_pMasterPalette);
   void createShadeHazeTable(RGBBVColorMatcher&        in_rRGBMatcher,
                             GFXPalette::MultiPalette& out_rMPalette,
                             const GFXPalette*         in_pMasterPalette);
  public:
   MPMMerger(const MPMOptions& in_rOptions);

   void insertPalette(GFXPalette*       out_pMasterPalette,
                      const GFXPalette* in_pMergePalette);

   void createRemapTables(GFXPalette* out_pMasterPalette);
};


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_MPMMERGER_H_
