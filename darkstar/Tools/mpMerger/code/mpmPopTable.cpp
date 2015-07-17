//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <algorithm>
#include <function>

#include <g_pal.h>
#include "sVector.h"
#include "mpmPopTable.h"
#include "mpmPseudoTrace.h"
#include "mpmOptions.h"
#include "mpmState.h"

namespace {

ColorF
computeShadedColor(const PALETTEENTRY& in_rPEntry,
                   const ColorF&       in_rShadeColor,
                   const UInt32        in_level,
                   const UInt32        in_numLevels,
                   const float         in_finalPercentage)
{
   ColorF newColor;
   newColor.red   = float(in_rPEntry.peRed)   / 255.0f;
   newColor.green = float(in_rPEntry.peGreen) / 255.0f;
   newColor.blue  = float(in_rPEntry.peBlue)  / 255.0f;
   float distanceAlong = (float(in_level) / float(in_numLevels - 1)) * in_finalPercentage;
   AssertFatal(distanceAlong <= 1.0f && distanceAlong >= 0.0f, "Huh?");
   
   newColor.interpolate(in_rShadeColor, newColor, distanceAlong);
   return newColor;
}

ColorF
computeHazedColor(const ColorF& in_rColor,
                  const ColorF& in_rHazeColor,
                  const UInt32  in_level,
                  const UInt32  in_numLevels,
                  const float   in_finalPercentage)
{
   ColorF newColor = in_rColor;
   float distanceAlong = (float(in_level) / float(in_numLevels - 1)) * in_finalPercentage;
   AssertFatal(distanceAlong <= 1.0f && distanceAlong >= 0.0f, "Huh?");

   newColor.interpolate(in_rHazeColor, newColor, distanceAlong);
   return newColor;
}

} // namespace {}



//-------------------------------------- Utility STL predicate classes...
//
class QVecWeightGreater {
  public:
   bool operator()(const quantVector& in_rVec1, const quantVector& in_rVec2) const {
      return in_rVec1.weight > in_rVec2.weight;
   }
};

struct QVecWeightLessThanFloat : public std::binary_function<quantVector, float, bool>
{
    bool operator() (const quantVector& in_rVec, const float in_compare) const {
      return in_rVec.weight < in_compare;
    }
};

//------------------------------------------------------------------------------
//-------------------------------------- OBJECT PROPER...
//
MPMPopularityTable::MPMPopularityTable(const MPMOptions& in_rOptions)
 : m_numFixedColors(0),
   m_pFixedColors(NULL),
   m_colorsQuantized(false),
   m_shadeColor(in_rOptions.getShadeColor()),
   m_hazeColor(in_rOptions.getHazeColor()),
   m_shadePercentage(in_rOptions.getShadePercentage()),
   m_hazePercentage(in_rOptions.getHazePercentage()),
   m_shBiasFactor(in_rOptions.getShadeHazeBiasFactor())
{
   in_rOptions.getShadeHazeBiasLevels(m_shadeLevels, m_hazeLevels);

   m_numUniqueColors = 0;
   m_totalWeight     = 0;

   m_pEntryWeightMap = new EntryWeightMap;
}

MPMPopularityTable::~MPMPopularityTable()
{
   delete [] m_pFixedColors;
   m_pFixedColors   = 0;
   m_numFixedColors = 0;

   delete m_pEntryWeightMap;
   m_pEntryWeightMap = NULL;

   m_colorsQuantized = false;
}

void
MPMPopularityTable::translatePEntryToQVectorRGB(const PALETTEENTRY& in_rPEntry,
                                                quantVector&        out_rVector)
{
   out_rVector.pElem[0] = float(in_rPEntry.peRed)   / 255.0f;
   out_rVector.pElem[1] = float(in_rPEntry.peGreen) / 255.0f;
   out_rVector.pElem[2] = float(in_rPEntry.peBlue)  / 255.0f;
}

inline void
MPMPopularityTable::registerRGBColor(const PALETTEENTRY& in_rEntry,
                                     const float         in_weight)
{
   float weight = in_weight;

   // First, no matter what, we register the plain color, without any
   //  weighting...
   //
   EntryWeightMap::iterator itr = m_pEntryWeightMap->find(in_rEntry);
   if (itr != m_pEntryWeightMap->end()) {
      (*itr).second += weight;
   } else {
      (*m_pEntryWeightMap)[in_rEntry] = in_weight;
      m_numUniqueColors++;
   }
   m_totalWeight += weight;

   // Now, register the fog colors, if there are any...
   //
   if (m_shadeLevels == 1 && m_hazeLevels == 1)
      return;

   weight *= m_shBiasFactor;
   for (UInt32 i = 0; i < m_shadeLevels; i++) {
      ColorF shadedColor = computeShadedColor(in_rEntry, m_shadeColor,
                                              i, m_shadeLevels,
                                              m_shadePercentage);

      for (UInt32 j = 0; j < m_hazeLevels; j++) {
         if (i == 0 && j == 0)
            continue;
         ColorF hazedColor = computeHazedColor(shadedColor, m_hazeColor,
                                               j, m_hazeLevels,
                                               m_hazePercentage);
         PALETTEENTRY bogusNew;
         bogusNew.peRed   = UInt8((hazedColor.red   * 255.0f) + 0.5);
         bogusNew.peGreen = UInt8((hazedColor.green * 255.0f) + 0.5);
         bogusNew.peBlue  = UInt8((hazedColor.blue  * 255.0f) + 0.5);

         EntryWeightMap::iterator itr = m_pEntryWeightMap->find(bogusNew);
         if (itr != m_pEntryWeightMap->end()) {
            (*itr).second += weight;
         } else {
            (*m_pEntryWeightMap)[bogusNew] = in_weight;
            m_numUniqueColors++;
         }
         m_totalWeight += weight;
      }
   }
}

void
MPMPopularityTable::prepFixedColors(const MPMOptions&  in_rOptions,
                                    const GFXPalette* in_pPal)
{
   UInt32 chooseFirst, chooseLast;
   UInt32 renderFirst, renderLast;

   in_rOptions.getMasterChooseRange(chooseFirst, chooseLast);
   in_rOptions.getMasterRenderRange(renderFirst, renderLast);

   // If this is satisfied, then there are no fixed colors...
   //
   if (chooseFirst == renderFirst && chooseLast == renderLast) {
      m_numFixedColors = 0;
      return;
   }

   // Otherwise, one of the following conditions are required to be
   //  satisfied...
   //
   int fixedFirst, fixedLast;
   if (chooseFirst == renderFirst) {
      fixedFirst = chooseLast + 1;
      fixedLast  = renderLast;
   } else if (chooseLast == renderLast) {
      fixedFirst = renderFirst;
      fixedLast  = chooseFirst - 1;
   } else {
      AssertFatal(0, "Overlapping on more than one side not allowed...");
   }
   m_numFixedColors = fixedLast - fixedFirst + 1;
   AssertFatal(m_numFixedColors > 0, "Invalid number of fixed colors");

   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(2);
   tracer << "MPMPopularity Table: setting " << m_numFixedColors << " fixed colors\n";
   tracer.popVerbosityLevel();

   // Get the colors from the palette, and copy them into an internal buffer...
   //
   m_pFixedColors = new PALETTEENTRY[m_numFixedColors];
   for (int i = fixedFirst, j = 0; i <= fixedLast; i++, j++)
      m_pFixedColors[j] = in_pPal->palette[0].color[i];
}

bool
MPMPopularityTable::quantizeColors(const MPMOptions& in_rOptions,
                                   GFXPalette*       out_pPal)
{
   UInt32 chooseFirst, chooseLast;
   in_rOptions.getMasterChooseRange(chooseFirst, chooseLast);

   Int32 numDesiredColors = (chooseLast - chooseFirst) + 1;
   numDesiredColors += m_numFixedColors;   

   // Create the quantVector buffers
   //
   int numDim = 3;
   int totalNumVectors = m_numUniqueColors + m_numFixedColors;

   // Prep the input buffer...
   //
   float* pInputFloatBuffer   = new float[totalNumVectors * numDim];
   quantVector* pInputVectors = new quantVector[totalNumVectors];
   for (int i = 0; i < totalNumVectors; i++) {
      pInputVectors[i].numDim = numDim;
      pInputVectors[i].pElem  = &pInputFloatBuffer[i * numDim];
      pInputVectors[i].weight = 0.0f;
   }

   // Prep the output buffer...
   //
   float* pOutputFloatBuffer   = new float[numDesiredColors * numDim];
   quantVector* pOutputVectors = new quantVector[numDesiredColors];
   for (Int32 i = 0; i < numDesiredColors; i++) {
      pOutputVectors[i].numDim = numDim;
      pOutputVectors[i].pElem  = &pOutputFloatBuffer[i * numDim];
      pOutputVectors[i].weight = 0.0f;
   }
   
   // Copy the fixed colors into the input buffer...
   //
   int   currentInputIndex = 0;
   float fixedColorWeight  = 2.0f * float(m_totalWeight);
   for (int i = 0; i < m_numFixedColors; i++) {
      quantVector& rVector = pInputVectors[currentInputIndex++];
      PALETTEENTRY& rEntry = m_pFixedColors[i];
      rVector.weight       = fixedColorWeight;

      translatePEntryToQVectorRGB(rEntry, rVector);
   }

   // Copy the input colors into the input buffer...
   //
   for (EntryWeightMap::iterator itr = m_pEntryWeightMap->begin();
        itr != m_pEntryWeightMap->end(); ++itr) {
      quantVector& rVector = pInputVectors[currentInputIndex++];
      rVector.weight       = (*itr).second;

      translatePEntryToQVectorRGB((*itr).first, rVector);
   }
   AssertFatal(currentInputIndex == totalNumVectors, "Something got lost here...");

   // And quantize...
   //
   quantizeVectors(pInputVectors,  totalNumVectors,
                   pOutputVectors, numDesiredColors);

   // We now need to extract any fixed colors from the palette...
   //
   quantVector* pEndExportColors = &pOutputVectors[numDesiredColors];
   quantVector* pStartExportColors;
   if (m_numFixedColors != 0) {
      // Use STL sort algorithm to sort into descending order, any colors that have weight
      //  > fixedColorWeight are requantized fixed colors, and we can nuke them...
      //
      std::sort(&pOutputVectors[0], &pOutputVectors[numDesiredColors], QVecWeightGreater());

      // Now, it is possible that the fixed colors contained a few that were the same,
      //  and they wound up in the same quantVector, so, we need to find the first
      //  quantVector that has a weight < fixedColorWeight...
      //
      pStartExportColors = std::find_if(&pOutputVectors[0],
                                        &pOutputVectors[numDesiredColors],
                                        std::bind2nd(QVecWeightLessThanFloat(),
                                                     fixedColorWeight));
   } else {
      pStartExportColors = pOutputVectors;
   }

   // Alright, these are the final colors, stick them in the palette...
   //
   UInt32 numFinalizedColors = pEndExportColors - pStartExportColors;
   
   // Hrmph.  If there were two fixed colors that wound up in the same bin, the PCA
   //  algortihm can generate too many colors. we'll take the lowest x colors, and
   //  toss them.  We may want to merge them into the closest color at some future
   //  point...
   //
   if ((numFinalizedColors + chooseFirst - 1) > chooseLast) {
      PseudoTrace& tracer = PseudoTrace::getTracerObject();


      UInt32 newNumFinalizedColors = chooseLast - chooseFirst + 1;

      tracer.pushVerbosityLevel(1);
      tracer << "MPMPopularity Table: tossing the "
             << numFinalizedColors - newNumFinalizedColors
             << " least important colors (FIX DMM)\n";
      tracer.popVerbosityLevel();

      numFinalizedColors = newNumFinalizedColors;
   }
   AssertFatal((numFinalizedColors + chooseFirst - 1) <= chooseLast, "Color overflow?");

   for (UInt32 i = 0; i < numFinalizedColors; i++) {
      PALETTEENTRY& rEntry = out_pPal->palette[0].color[chooseFirst + i];

      rEntry.peRed   = UInt8((pStartExportColors[i].pElem[0] * 255.0f) + 0.5f);
      rEntry.peGreen = UInt8((pStartExportColors[i].pElem[1] * 255.0f) + 0.5f);
      rEntry.peBlue  = UInt8((pStartExportColors[i].pElem[2] * 255.0f) + 0.5f);
      rEntry.peFlags = 0x0;
   }

   if (numFinalizedColors != (chooseLast - chooseFirst + 1)) {
      // Pad the palette...
      //
      PseudoTrace& tracer = PseudoTrace::getTracerObject();

      int numPadEntries = (chooseLast - chooseFirst + 1) - numFinalizedColors;

      tracer.pushVerbosityLevel(1);
      tracer << "MPMPopTable::quantizeColors(): padding " << numPadEntries << " entries";
      tracer.popVerbosityLevel();

      for (UInt32 i = chooseLast; i > chooseLast - numPadEntries; i--) {
         PALETTEENTRY* pEntry = &out_pPal->palette[0].color[i];
         pEntry->peRed   = 0xff;
         pEntry->peGreen = 0x0;
         pEntry->peBlue  = 0xff;
         pEntry->peFlags = 0xff;
      }
   }

   delete [] pOutputVectors;
   delete [] pOutputFloatBuffer;

   delete [] pInputVectors;
   delete [] pInputFloatBuffer;

   m_colorsQuantized = true;
   return true;
}

void
MPMPopularityTable::registerRGBColors(const GFXPalette* in_pPal,
                                      const UInt32      in_weight)
{
   AssertFatal(in_pPal != NULL, "No palette?");

//   if (in_pPal->hasQuantizationInfo() == true) {
//      for (UInt32 i = in_pPal->m_usedRangeStart; i < in_pPal->m_usedRangeEnd; i++) {
//         const PALETTEENTRY& pEntry = in_pPal->palette[0].color[i];
//         registerRGBColor(pEntry, in_weight * in_pPal->m_pColorWeights[i]);
//      }
//   } else {
      for (int i = 0; i < 256; i++) {
         const PALETTEENTRY& pEntry = in_pPal->palette[0].color[i];
         registerRGBColor(pEntry, in_weight * 1.0f);
      }
//   }
}

