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
#include "pnPopTable.h"
#include "pnPseudoTrace.h"
#include "pnOptions.h"
#include "pnState.h"

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)
#pragma option -Jg
#endif

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
PNPopularityTable::PNPopularityTable(const PNOptions& in_rOptions)
 : m_numFixedColors(0),
   m_pFixedColors(NULL),
   m_colorsQuantized(false)
{
   m_rBits = in_rOptions.getRBits();
   m_gBits = in_rOptions.getGBits();
   m_bBits = in_rOptions.getBBits();
   
   if (in_rOptions.getColorSpace() == PNOptions::ColorSpaceAlpha) {
      m_aBits       = in_rOptions.getABits();
      m_inAlphaMode = true;
   } else {
      m_aBits       = 0;
      m_inAlphaMode = false;
   }

   int totalNumBits = m_rBits + m_gBits + m_bBits + m_aBits;
   m_tableSize      = 1 << totalNumBits;
   
   AssertFatal(m_tableSize < (1 << 27),
               avar("Either %d is more bits than you intended, or you have far too much memory in your computer.  Piss off!",
                    totalNumBits));

   // Set up the zerocolor, if any...
   //
   ColorF zeroColorF;
   if (in_rOptions.getZeroColor(zeroColorF) == true) {
      AssertFatal(m_inAlphaMode == false, "No zerocolors allowed in alpha Mode!");

      m_zeroColor =
         ((UInt8((zeroColorF.red   * 255.0f) + 0.5f) >> (8 - m_rBits)) << (m_gBits + m_bBits)) |
         ((UInt8((zeroColorF.green * 255.0f) + 0.5f) >> (8 - m_gBits)) << (m_bBits))           |
         ((UInt8((zeroColorF.blue  * 255.0f) + 0.5f) >> (8 - m_bBits)) << (0));
   } else {
      // In general, nothing had better match this index...
      //
      m_zeroColor = UInt32(-1);
   }

   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(4);
   m_pTable = new UInt32[m_tableSize];

   memset(m_pTable, 0, sizeof(UInt32) * m_tableSize);
   m_numUniqueColors = 0;
   m_totalWeight     = 0;
   m_colorsQuantized = false;
   tracer << "PopularityTable(): created and zeroed table of " << m_tableSize
          << " entries (" << m_tableSize * sizeof(UInt32) << " bytes)\n";
   tracer.popVerbosityLevel();
}

PNPopularityTable::~PNPopularityTable()
{
   delete [] m_pFixedColors;
   m_pFixedColors   = 0;
   m_numFixedColors = 0;

   delete [] m_pTable;
   m_pTable = NULL;
   
   m_rBits    =
      m_gBits = 
      m_bBits = 
      m_aBits = 0;
}

inline void
PNPopularityTable::translateIndexToQVectorRGB(const UInt32 in_index,
                                              quantVector& out_rVector)
{
   UInt32 r, g, b;

   b = in_index;
   g = in_index >> m_bBits;
   r = in_index >> (m_bBits + m_gBits);

   r &= (1 << m_rBits) - 1;
   g &= (1 << m_gBits) - 1;
   b &= (1 << m_bBits) - 1;

   out_rVector.pElem[0] = float(r) / float((1 << m_rBits) - 1);
   out_rVector.pElem[1] = float(g) / float((1 << m_gBits) - 1);
   out_rVector.pElem[2] = float(b) / float((1 << m_bBits) - 1);
}

inline void
PNPopularityTable::translateIndexToQVectorRGBA(const UInt32 in_index,
                                               quantVector& out_rVector)
{
   UInt32 r, g, b, a;

   b = in_index;
   g = in_index >> m_bBits;
   r = in_index >> (m_bBits + m_gBits);
   a = in_index >> (m_bBits + m_gBits + m_rBits);

   r &= (1 << m_rBits) - 1;
   g &= (1 << m_gBits) - 1;
   b &= (1 << m_bBits) - 1;
   a &= (1 << m_aBits) - 1;

   out_rVector.pElem[0] = float(r) / float((1 << m_rBits) - 1);
   out_rVector.pElem[1] = float(g) / float((1 << m_gBits) - 1);
   out_rVector.pElem[2] = float(b) / float((1 << m_bBits) - 1);
   out_rVector.pElem[3] = float(a) / float((1 << m_aBits) - 1);
}

inline void
PNPopularityTable::translatePEntryToQVectorRGB(const PALETTEENTRY& in_rPEntry,
                                               quantVector&        out_rVector)
{
   out_rVector.pElem[0] = float(in_rPEntry.peRed)   / 255.0f;
   out_rVector.pElem[1] = float(in_rPEntry.peGreen) / 255.0f;
   out_rVector.pElem[2] = float(in_rPEntry.peBlue)  / 255.0f;
}

inline void
PNPopularityTable::translatePEntryToQVectorRGBA(const PALETTEENTRY& in_rPEntry,
                                                quantVector&        out_rVector)
{
   out_rVector.pElem[0] = float(in_rPEntry.peRed)   / 255.0f;
   out_rVector.pElem[1] = float(in_rPEntry.peGreen) / 255.0f;
   out_rVector.pElem[2] = float(in_rPEntry.peBlue)  / 255.0f;
   out_rVector.pElem[3] = float(in_rPEntry.peFlags) / 255.0f;
}


void
PNPopularityTable::prepFixedColors(const PNOptions&  in_rOptions,
                                   const GFXPalette* in_pPal)
{
   UInt32 chooseFirst, chooseLast;
   UInt32 renderFirst, renderLast;

   in_rOptions.getChooseRange(chooseFirst, chooseLast);
   in_rOptions.getRenderRange(renderFirst, renderLast);

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
   tracer << "PNPopularity Table: setting " << m_numFixedColors << " fixed colors\n";
   tracer.popVerbosityLevel();

   // Get the colors from the palette, and copy them into an internal buffer...
   //
   m_pFixedColors = new PALETTEENTRY[m_numFixedColors];
   for (int i = fixedFirst, j = 0; i <= fixedLast; i++, j++)
      m_pFixedColors[j] = in_pPal->palette[0].color[i];
}

bool
PNPopularityTable::quantizeColors(const PNOptions& in_rOptions,
                                  GFXPalette*      out_pPal)
{
   AssertFatal(in_rOptions.areChoosingColors(), "Why are we here?");

   UInt32 chooseFirst, chooseLast;
   in_rOptions.getChooseRange(chooseFirst, chooseLast);

   Int32 numDesiredColors = (chooseLast - chooseFirst) + 1;
   numDesiredColors += m_numFixedColors;   

   // Create the quantVector buffers
   //
   int numDim;
   if (m_inAlphaMode == false) {
      numDim = 3;
   } else {
      numDim = 4;
   }
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
   int currentInputIndex  = 0;
   float fixedColorWeight = 10.0f * float(m_totalWeight);
   for (int i = 0; i < m_numFixedColors; i++) {
      quantVector& rVector = pInputVectors[currentInputIndex++];
      PALETTEENTRY& rEntry = m_pFixedColors[i];
      rVector.weight       = fixedColorWeight;

      if (m_inAlphaMode == true)
         translatePEntryToQVectorRGBA(rEntry, rVector);
      else
         translatePEntryToQVectorRGB(rEntry, rVector);
   }

   // Copy the input colors into the input buffer...
   //
   for (UInt32 i = 0; i < m_tableSize; i++) {
      if (m_pTable[i] != 0) {
         quantVector& rVector = pInputVectors[currentInputIndex++];
         rVector.weight       = float(m_pTable[i]);

         if (m_inAlphaMode == true)
            translateIndexToQVectorRGBA(i, rVector);
         else
            translateIndexToQVectorRGB(i, rVector);
      }
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
      PALETTEENTRY& rEntry  = out_pPal->palette[0].color[chooseFirst + i];

      rEntry.peRed   = UInt8((pStartExportColors[i].pElem[0] * 255.0f) + 0.5f);
      rEntry.peGreen = UInt8((pStartExportColors[i].pElem[1] * 255.0f) + 0.5f);
      rEntry.peBlue  = UInt8((pStartExportColors[i].pElem[2] * 255.0f) + 0.5f);
      
      if (m_inAlphaMode == true)
         rEntry.peFlags = UInt8((pStartExportColors[i].pElem[3] * 255.0f) + 0.5f);
      else
         rEntry.peFlags = 0xff;
   }

   if (numFinalizedColors != (chooseLast - chooseFirst + 1)) {
      // Pad the palette...
      //
      PseudoTrace& tracer = PseudoTrace::getTracerObject();

      int numPadEntries = (chooseLast - chooseFirst + 1) - numFinalizedColors;

      tracer.pushVerbosityLevel(1);
      tracer << "PNPopTable::quantizeColors(): padding " << numPadEntries << " entries";
      tracer.popVerbosityLevel();

      for (UInt32 i = chooseLast; i > chooseLast - numPadEntries; i--) {
         PALETTEENTRY& rEntry = out_pPal->palette[0].color[i];
         rEntry.peRed   = 0xff;
         rEntry.peGreen = 0x0;
         rEntry.peBlue  = 0xff;
         rEntry.peFlags = 0xff;
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
PNPopularityTable::registerRGBColors(const PALETTEENTRY* in_pPEntries,
                                     const UInt32        in_numEntries,
                                     const UInt32        in_weight)
{
   AssertFatal(m_pTable != NULL, "No Table?");
   AssertFatal(m_inAlphaMode == false, "Wrong data type...");

   for (UInt32 i = 0; i < in_numEntries; i++) {
      registerRGBColor(in_pPEntries[i], in_weight);
   }
}

void
PNPopularityTable::registerRGBAColors(const PALETTEENTRY* in_pPEntries,
                                      const UInt32        in_numEntries,
                                      const UInt32        in_weight)
{
   AssertFatal(m_pTable != NULL, "No Table?");

   if (m_inAlphaMode == true) {
      for (UInt32 i = 0; i < in_numEntries; i++) {
         registerRGBAColor(in_pPEntries[i], in_weight);
      }
   } else {
      for (UInt32 i = 0; i < in_numEntries; i++) {
         registerRGBColor(in_pPEntries[i], in_weight);
      }
   }
}

