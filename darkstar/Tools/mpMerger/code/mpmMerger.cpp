//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <g_pal.h>
#include "mpmOptions.h"
#include "mpmMerger.h"
#include "colorMatcher.h"
#include "mpmPseudoTrace.h"

namespace {

int
calcShift(int x)
{
   int result = -1;
   do {
      x >>= 1;
      result++;
   }while(x);
   return (result);
}   

} // namespace {}


MPMMerger::MPMMerger(const MPMOptions& in_rOptions)
 : m_shadeColor(in_rOptions.getShadeColor()),
   m_shadePercentage(in_rOptions.getShadePercentage()),
   m_hazeColor(in_rOptions.getHazeColor()),
   m_hazePercentage(in_rOptions.getHazePercentage()),
   m_numShadeLevels(in_rOptions.getNumShadeLevels()),
   m_numHazeLevels(in_rOptions.getNumHazeLevels())
{
   //
}


void
MPMMerger::insertPalette(GFXPalette*       out_pMasterPalette,
                         const GFXPalette* in_pMergePalette)
{
   AssertFatal(in_pMergePalette->palette[0].paletteIndex != 0xffffffff,
               "Invalid palette key, -1");
   for (int i = 1; i < out_pMasterPalette->numPalettes; i++) {
      AssertFatal((in_pMergePalette->palette[0].paletteIndex !=
                   out_pMasterPalette->palette[i].paletteIndex),
                  avar("Error: two palettes share palette key: 0x%x",
                       in_pMergePalette->palette[0].paletteIndex));
   }

   GFXPalette::MultiPalette& rMPalette =
      out_pMasterPalette->palette[out_pMasterPalette->numPalettes];
   out_pMasterPalette->numPalettes++;

   memcpy(rMPalette.color, in_pMergePalette->palette[0].color, sizeof(PALETTEENTRY) * 256);
   rMPalette.paletteIndex = in_pMergePalette->palette[0].paletteIndex;
   
   switch (in_pMergePalette->palette[0].paletteType) {
     case GFXPalette::ColorQuantPaletteType:
      rMPalette.paletteType = GFXPalette::ShadeHazePaletteType;
      break;
     case GFXPalette::AlphaQuantPaletteType:
      rMPalette.paletteType = GFXPalette::TranslucentPaletteType;
      break;
     case GFXPalette::AdditiveQuantPaletteType:
      rMPalette.paletteType = GFXPalette::AdditivePaletteType;
      break;
     case GFXPalette::SubtractiveQuantPaletteType:
      rMPalette.paletteType = GFXPalette::SubtractivePaletteType;
      break;

     default:
      AssertFatal(0, avar("Unexpected palette type: %d",
                          in_pMergePalette->palette[0].paletteType));
   }
}


void
MPMMerger::createCommonTables(RGBBVColorMatcher&        in_rRGBMatcher,
                              GFXPalette::MultiPalette& out_rMPalette)
{
   // First the identity map...
   //
   for (int i = 0; i < 256; i++) {
      out_rMPalette.identityMap[i] = in_rRGBMatcher.matchColor(out_rMPalette.color[i]);
   }

   // Now the component maps...
   //
   for (int i = 0; i < 256; i++) {
      out_rMPalette.indexToRMap[i] = float(out_rMPalette.color[i].peRed)   / 255.0f;
      out_rMPalette.indexToGMap[i] = float(out_rMPalette.color[i].peGreen) / 255.0f;
      out_rMPalette.indexToBMap[i] = float(out_rMPalette.color[i].peBlue)  / 255.0f;

      if (out_rMPalette.paletteType == GFXPalette::TranslucentPaletteType)
         out_rMPalette.indexToAMap[i] = float(out_rMPalette.color[i].peFlags) / 255.0f;
      else
         out_rMPalette.indexToAMap[i] = (i == 0) ? 0.0f : 1.0f;
   }
}

void
MPMMerger::createShadeHazeTable(RGBBVColorMatcher&        in_rRGBMatcher,
                                GFXPalette::MultiPalette& out_rMPalette,
                                const GFXPalette*         in_pMasterPalette)
{

   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(1);
   tracer << "ShadeHaze: ";

   // Shade/Haze tables...
   //
   for (UInt32 s = 0; s < m_numShadeLevels; s++) {
      for (UInt32 h = 0; h < m_numHazeLevels; h++) {
         UInt8* currTable =
            out_rMPalette.shadeMap + (256 * ((h * m_numShadeLevels) + s));

         calcShadeHazeTable(in_rRGBMatcher, currTable, out_rMPalette, s, h);
      }

      tracer << ".";
      tracer.flush();
   }

   // Haze table...
   //
   for (int h = 0; h < in_pMasterPalette->shadeLevels; h++) {
      UInt8* currTable =
         out_rMPalette.hazeMap + (256 * h);

      calcHazeTable(in_rRGBMatcher, currTable, out_rMPalette, h);
      tracer << ".";
      tracer.flush();
   }

   tracer << "\n";
   tracer.popVerbosityLevel();
}

void
MPMMerger::createAlphaTable(RGBBVColorMatcher&        in_rRGBMatcher,
                            GFXPalette::MultiPalette& out_rMPalette,
                            const GFXPalette*         in_pMasterPalette)
{
   UInt8* currDest = out_rMPalette.transMap;
   
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(1);
   tracer << "Alpha:     ";

   // First row is special case, always equal to dest...
   //
   for (int i = 0; i < 256; i++) {
      currDest[i] = i;
   }
   currDest += 256;
   
   for (int i = 1; i < 256; i++) {
      PALETTEENTRY& srcColor = out_rMPalette.color[i];
      float alphaVal   = float(srcColor.peFlags) / 255.0f;
      float oneMinusAV = 1 - alphaVal;

      // Premultiply srcColor...
      //
      ColorF srcColorF;
      srcColorF.red   = float(srcColor.peRed)   * alphaVal;
      srcColorF.green = float(srcColor.peGreen) * alphaVal;
      srcColorF.blue  = float(srcColor.peBlue)  * alphaVal;

      for (int j = 0; j < 256; j++) {
         const PALETTEENTRY& dstColor = in_pMasterPalette->palette[0].color[j];

         ColorF dstColorF;
         dstColorF.red   = float(dstColor.peRed)   * oneMinusAV;
         dstColorF.green = float(dstColor.peGreen) * oneMinusAV;
         dstColorF.blue  = float(dstColor.peBlue)  * oneMinusAV;

         PALETTEENTRY finalColor;
         finalColor.peRed   = UInt8(srcColorF.red   + dstColorF.red);
         finalColor.peGreen = UInt8(srcColorF.green + dstColorF.green);
         finalColor.peBlue  = UInt8(srcColorF.blue  + dstColorF.blue);

         currDest[j] = in_rRGBMatcher.matchColor(finalColor);
      }

      currDest += 256;

      if ((i % 4) == 0) {
         tracer << ".";
         tracer.flush();
      }
   }

   tracer << "\n";
   tracer.popVerbosityLevel();
}

void
MPMMerger::createAdditiveTable(RGBBVColorMatcher&        in_rRGBMatcher,
                               GFXPalette::MultiPalette& out_rMPalette,
                               const GFXPalette*         in_pMasterPalette)
{
   UInt8* currDest = out_rMPalette.transMap;
   
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(1);
   tracer << "Additive:  ";

   // First row is special case, always equal to dest...
   //
   for (int i = 0; i < 256; i++) {
      currDest[i] = i;
   }
   currDest += 256;
   
   for (int i = 1; i < 256; i++) {
      PALETTEENTRY& srcColor = out_rMPalette.color[i];

      ColorF srcColorF;
      srcColorF.red   = float(srcColor.peRed) ;
      srcColorF.green = float(srcColor.peGreen);
      srcColorF.blue  = float(srcColor.peBlue);

      for (int j = 0; j < 256; j++) {
         const PALETTEENTRY& dstColor = in_pMasterPalette->palette[0].color[j];

         ColorF dstColorF;
         dstColorF.red   = float(dstColor.peRed)   + srcColorF.red;
         dstColorF.green = float(dstColor.peGreen) + srcColorF.green;
         dstColorF.blue  = float(dstColor.peBlue)  + srcColorF.blue;

         if (dstColorF.red > 255.0f)
            dstColorF.red = 255.0f;
         if (dstColorF.green > 255.0f)
            dstColorF.green = 255.0f;
         if (dstColorF.blue > 255.0f)
            dstColorF.blue = 255.0f;

         PALETTEENTRY finalColor;
         finalColor.peRed   = UInt8(dstColorF.red);
         finalColor.peGreen = UInt8(dstColorF.green);
         finalColor.peBlue  = UInt8(dstColorF.blue);

         currDest[j] = in_rRGBMatcher.matchColor(finalColor);
      }

      currDest += 256;

      if ((i % 4) == 0) {
         tracer << ".";
         tracer.flush();
      }
   }

   tracer << "\n";
   tracer.popVerbosityLevel();
}

void
MPMMerger::createSubtractiveTable(RGBBVColorMatcher&        in_rRGBMatcher,
                                  GFXPalette::MultiPalette& out_rMPalette,
                                  const GFXPalette*         in_pMasterPalette)
{
   UInt8* currDest = out_rMPalette.transMap;
   
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(1);
   tracer << "Subtractive: ";

   // First row is special case, always equal to dest...
   //
   for (int i = 0; i < 256; i++) {
      currDest[i] = i;
   }
   currDest += 256;
   
   for (int i = 1; i < 256; i++) {
      PALETTEENTRY& srcColor = out_rMPalette.color[i];

      ColorF srcColorF;
      srcColorF.red   = (255.0f - float(srcColor.peRed))   / 255.0f;
      srcColorF.green = (255.0f - float(srcColor.peGreen)) / 255.0f;
      srcColorF.blue  = (255.0f - float(srcColor.peBlue))  / 255.0f;

      for (int j = 0; j < 256; j++) {
         const PALETTEENTRY& dstColor = in_pMasterPalette->palette[0].color[j];

         ColorF dstColorF;
         dstColorF.red   = float(dstColor.peRed)   * srcColorF.red;
         dstColorF.green = float(dstColor.peGreen) * srcColorF.green;
         dstColorF.blue  = float(dstColor.peBlue)  * srcColorF.blue;

         if (dstColorF.red < 0.0f)
            dstColorF.red = 0.0f;
         if (dstColorF.green < 0.0f)
            dstColorF.green = 0.0f;
         if (dstColorF.blue < 0.0f)
            dstColorF.blue = 0.0f;

         PALETTEENTRY finalColor;
         finalColor.peRed   = UInt8(dstColorF.red);
         finalColor.peGreen = UInt8(dstColorF.green);
         finalColor.peBlue  = UInt8(dstColorF.blue);

         currDest[j] = in_rRGBMatcher.matchColor(finalColor);
      }

      currDest += 256;

      if ((i % 4) == 0) {
         tracer << ".";
         tracer.flush();
      }
   }

   tracer << "\n";
   tracer.popVerbosityLevel();
}

void
MPMMerger::createRemapTables(GFXPalette* out_pMasterPalette)
{
   // Calculate the shadeShift, and enter the number of shade/haze levels...
   //
   out_pMasterPalette->shadeLevels = m_numShadeLevels;
   out_pMasterPalette->hazeLevels  = m_numHazeLevels;
   out_pMasterPalette->shadeShift  = calcShift(m_numShadeLevels);

   DWORD remapTableSize = 0;
   for(Int32 i = 0; i < out_pMasterPalette->numPalettes; i++) {
      if(out_pMasterPalette->palette[i].paletteType == GFXPalette::ShadeHazePaletteType) {
         remapTableSize += 256 * out_pMasterPalette->shadeLevels * (out_pMasterPalette->hazeLevels + 1);

         remapTableSize += 256 * sizeof(BYTE);
         remapTableSize += 4 * (256 * sizeof(float));
      } else if(out_pMasterPalette->palette[i].paletteType == GFXPalette::TranslucentPaletteType ||
                out_pMasterPalette->palette[i].paletteType == GFXPalette::AdditivePaletteType    ||
                out_pMasterPalette->palette[i].paletteType == GFXPalette::SubtractivePaletteType) {
         remapTableSize += 65536;

         remapTableSize += 256 * sizeof(BYTE);
         remapTableSize += 4 * (256 * sizeof(float));
      }
   }
   
   out_pMasterPalette->remapBase  = new UInt8[remapTableSize];
   out_pMasterPalette->remapTable = out_pMasterPalette->remapBase;

   // now build the remap table
   if(remapTableSize != 0) {
      UInt32 padOffset = out_pMasterPalette->shadeLevels * 256;
      out_pMasterPalette->remapBase = new UInt8[ remapTableSize + padOffset ];

      UInt8* remap = (UInt8*) (( (UInt32) out_pMasterPalette->remapBase + padOffset) & ~(padOffset - 1));
      out_pMasterPalette->remapTable = remap;

      for(Int32 i = 0; i < out_pMasterPalette->numPalettes; i++) {
         GFXPalette::MultiPalette& rMPalette = out_pMasterPalette->palette[i];

         if(rMPalette.paletteType == GFXPalette::ShadeHazePaletteType) {
            rMPalette.shadeMap = remap;
            rMPalette.hazeMap  = remap + (256 * out_pMasterPalette->shadeLevels *
                                          out_pMasterPalette->hazeLevels);
            remap += 256 * out_pMasterPalette->shadeLevels * (out_pMasterPalette->hazeLevels + 1);
         } else if(out_pMasterPalette->palette[i].paletteType == GFXPalette::TranslucentPaletteType ||
                   out_pMasterPalette->palette[i].paletteType == GFXPalette::AdditivePaletteType    ||
                   out_pMasterPalette->palette[i].paletteType == GFXPalette::SubtractivePaletteType) {
            out_pMasterPalette->palette[i].transMap = remap;
            remap += 65536;
         }
      }
      for(Int32 i = 0; i < out_pMasterPalette->numPalettes; i++) {
         GFXPalette::MultiPalette& rMPalette = out_pMasterPalette->palette[i];

         if(rMPalette.paletteType == GFXPalette::ShadeHazePaletteType   ||
            rMPalette.paletteType == GFXPalette::TranslucentPaletteType ||
            rMPalette.paletteType == GFXPalette::AdditivePaletteType    ||
            rMPalette.paletteType == GFXPalette::SubtractivePaletteType) {
            rMPalette.identityMap = remap;
            remap += 256;
            rMPalette.indexToRMap = (float*)remap;
            remap += 256 * sizeof(float);
            rMPalette.indexToGMap = (float*)remap;
            remap += 256 * sizeof(float);
            rMPalette.indexToBMap = (float*)remap;
            remap += 256 * sizeof(float);
            rMPalette.indexToAMap = (float*)remap;
            remap += 256 * sizeof(float);
         }
      }
   }

   // Set up a caching matcher...
   //
   RGBBVColorMatcher rgbMatcher;
   rgbMatcher.setMatchPalette(out_pMasterPalette,
                              out_pMasterPalette->m_usedRangeStart,
                              out_pMasterPalette->m_usedRangeEnd);

   for (Int32 i = 0; i < out_pMasterPalette->numPalettes; i++) {
      GFXPalette::MultiPalette& rMPalette = out_pMasterPalette->palette[i];

      if(rMPalette.paletteType == GFXPalette::ShadeHazePaletteType) {
         createShadeHazeTable(rgbMatcher, rMPalette, out_pMasterPalette);
         createCommonTables(rgbMatcher, rMPalette);
      } else if (out_pMasterPalette->palette[i].paletteType == GFXPalette::TranslucentPaletteType) {
         createAlphaTable(rgbMatcher, rMPalette, out_pMasterPalette);
         createCommonTables(rgbMatcher, rMPalette);
      } else if (out_pMasterPalette->palette[i].paletteType == GFXPalette::AdditivePaletteType) {
         createAdditiveTable(rgbMatcher, rMPalette, out_pMasterPalette);
         createCommonTables(rgbMatcher, rMPalette);
      } else if (out_pMasterPalette->palette[i].paletteType == GFXPalette::SubtractivePaletteType) {
         createSubtractiveTable(rgbMatcher, rMPalette, out_pMasterPalette);
         createCommonTables(rgbMatcher, rMPalette);
      }

      // Special case for master palette...
      if (i == 0) {
         AssertFatal(out_pMasterPalette->palette[0].identityMap != NULL, "Should never have null id map");
         for (int j = 0; j < 256; j++) {
            out_pMasterPalette->palette[0].identityMap[j] = j;
         }
      }
   }
}

void
MPMMerger::calcShadeHazeTable(RGBBVColorMatcher&        in_rRGBMatcher,
                              UInt8*                    out_pTable,
                              GFXPalette::MultiPalette& out_rMPalette,
                              const UInt32              in_shade,
                              const UInt32              in_haze)
{
   for (int i = 0; i < 256; i++) {
      ColorF currentColor;

      currentColor.red   = float(out_rMPalette.color[i].peRed)   / 255.0f;
      currentColor.green = float(out_rMPalette.color[i].peGreen) / 255.0f;
      currentColor.blue  = float(out_rMPalette.color[i].peBlue)  / 255.0f;

      float percentage = m_shadePercentage;
      if (m_numShadeLevels > 1)
         percentage *= float(in_shade) / float(m_numShadeLevels - 1);

      percentage = 1.0f - percentage;
      currentColor.interpolate(ColorF(currentColor), m_shadeColor, percentage);

      if (in_haze != 0) {
         float percentage = m_hazePercentage;
         if (m_numHazeLevels > 1)
            percentage *= float(in_haze) / float(m_numHazeLevels - 1);

         currentColor.interpolate(ColorF(currentColor), m_hazeColor, percentage);
      }

      PALETTEENTRY lookUp;
      lookUp.peRed   = UInt8((currentColor.red   * 255.0f) + 0.5f);
      lookUp.peGreen = UInt8((currentColor.green * 255.0f) + 0.5f);
      lookUp.peBlue  = UInt8((currentColor.blue  * 255.0f) + 0.5f);

      out_pTable[i] = in_rRGBMatcher.matchColor(lookUp);
   }
}


void
MPMMerger::calcHazeTable(RGBBVColorMatcher&        in_rRGBMatcher,
                         UInt8*                    out_pTable,
                         GFXPalette::MultiPalette& out_rMPalette,
                         const int                 in_haze)
{
   for (int i = 0; i < 256; i++) {
      ColorF currentColor;

      currentColor.red   = float(out_rMPalette.color[i].peRed)   / 255.0f;
      currentColor.green = float(out_rMPalette.color[i].peGreen) / 255.0f;
      currentColor.blue  = float(out_rMPalette.color[i].peBlue)  / 255.0f;

      float percentage = m_hazePercentage;
      if (m_numShadeLevels > 1)
         percentage *= float(in_haze) / float(m_numShadeLevels - 1);

      currentColor.interpolate(ColorF(currentColor), m_hazeColor, percentage);

      PALETTEENTRY lookUp;
      lookUp.peRed   = UInt8((currentColor.red   * 255.0f) + 0.5f);
      lookUp.peGreen = UInt8((currentColor.green * 255.0f) + 0.5f);
      lookUp.peBlue  = UInt8((currentColor.blue  * 255.0f) + 0.5f);

      out_pTable[i] = in_rRGBMatcher.matchColor(lookUp);
   }
}

