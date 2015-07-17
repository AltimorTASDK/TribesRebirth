//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>

#include <filstrm.h>

#include "pnOptions.h"
#include "pnFileList.h"
#include "pnState.h"
#include "pnPseudoTrace.h"

namespace {

typedef bool (PNOptions::*PNParseFunction)(const char*);

//--------------------------------------
// Parse keyword->functions table
//--------------------------------------
//
struct KeywordFunctionMapEntry {
   const char*     pKeyWord;
   PNParseFunction pFunction;
};

KeywordFunctionMapEntry sg_functionTable[] =
{
   {"quantizeAlpha",       PNOptions::quantizeAlpha},    // quantization type
   {"quantizeRGB",         PNOptions::quantizeRGB},
   {"quantizeAdditive",    PNOptions::quantizeAdditive},
   {"quantizeSubtractive", PNOptions::quantizeSubtractive},
   
   {"chooseRange",         PNOptions::chooseRange},
   {"renderRange",         PNOptions::renderRange},
   {"noRender",            PNOptions::noRender},
   
   {"outputPath",          PNOptions::outputPath},
   {"palOutputName",       PNOptions::palOutputName},
   {"basePalName",         PNOptions::basePalName},
   
   {"zeroColor",           PNOptions::zeroColor},

   {"quantizeBitsRGB",     PNOptions::quantizeBitsRGB},
   {"quantizeBitsAlpha",   PNOptions::quantizeBitsAlpha},
   
   {"paletteKey",          PNOptions::paletteKey},

   {"stripQuantizationInfo", PNOptions::stripQuantizationInfo},
   {"extrudeMipLevels",      PNOptions::extrudeMipLevels},

   {"enableDithering",     PNOptions::enableDithering},
   {"ditherTolerance",     PNOptions::ditherTolerance},

   {"standAlonePalette",   PNOptions::standAlonePalette},

   {"enforcePhoenixSave",  PNOptions::enforcePhoenixSave},

   {"maintainEdges",       PNOptions::maintainEdges},

   {"bitmapListStart",     PNOptions::bitmapListStart},  // only parseFunction that returns
                                                         //  false.
   {NULL,                  NULL}
};

}; // namespace {}


PNOptions::PNOptions()
 : m_pFileName(NULL),
   m_fileSize(0),
   m_pFileBuffer(NULL),
   m_tokenizationBegun(false)
{
   // Extracted options, note, almost all of these will be overwritten
   m_overrideRenderRange = false;
   m_renderingBitmaps    = false;
   m_choosingColors      = false;
   
   m_colorSpace  = ColorSpaceUndetermined;

   m_chooseFirst = 10;
   m_chooseLast  = 246;
   m_renderFirst = 10;
   m_renderLast  = 246;

   m_numRBits    = 6;
   m_numGBits    = 6;
   m_numBBits    = 6;
   m_numABits    = 5;

   m_pOutPath       = NULL;
   m_pOutputPalName = NULL;
   m_pBasePalName   = NULL;

   m_zeroColor.set(-1.0f, -1.0f, -1.0f);

   m_stripQuantizationInfo = false;
   m_extrudingMipLevels    = false;

   m_areDithering    = false;
   m_ditherTolerance = 1;

   m_paletteKey = 0;

   m_standAlonePalette = false;
   m_maintainEdges = false;

   m_asPhoenix = false;
}

PNOptions::~PNOptions()
{
   unattachResponseFile();
}


void
PNOptions::getRenderRange(UInt32& out_rRangeFirst, UInt32& out_rRangeLast) const
{
   if (m_renderingBitmaps == true) {
      out_rRangeFirst = m_renderFirst;
      out_rRangeLast  = m_renderLast;
   } else {
      out_rRangeFirst = m_chooseFirst;
      out_rRangeLast  = m_chooseLast;
   }
}


bool
PNOptions::getNextLine(char         out_pLineBuffer[],
                       const UInt32 in_bufferSize)
{
   AssertFatal(isAttached(),      "getNextLine(): not attached to a file");
   AssertFatal(in_bufferSize > 0, "No size?");

   static const char* s_pSeparators = "\r\n";

   char* currTok;
   if (m_tokenizationBegun == false) {
      currTok = strtok(m_pFileBuffer, s_pSeparators);

      if (currTok == NULL) {
         return false;
      }
      
      AssertFatal(strlen(currTok) < in_bufferSize, "too short buffer");
      m_tokenizationBegun = true;
   } else {
      currTok  = strtok(NULL, s_pSeparators);
      
      if (currTok == NULL) {
         return false;
      }
      
      AssertFatal(strlen(currTok) < in_bufferSize, "too short buffer");
   }

   // Ok, we have a valid line, now strip off any leading spaces and tabs...
   //
   int startIndex = 0;
   while ((currTok[startIndex] == ' ' ||
           currTok[startIndex] == '\t') &&
          currTok[startIndex] != '\0') {
      startIndex++;
   }

   int endIndex = strlen(currTok);
   while ((currTok[startIndex] == ' '  ||
           currTok[startIndex] == '\t' ||
           currTok[startIndex] == '\0') &&
          endIndex > 0) {
      endIndex--;
   }

   if (startIndex > endIndex || (strncmp("//", &currTok[startIndex], 2) == 0)) {
      // This was an empty or comment line, try to recurse, and find a non-empty line...
      //
      return getNextLine(out_pLineBuffer, in_bufferSize);
   } else {
      int i;
      for (i = startIndex; i <= endIndex; i++) {
         out_pLineBuffer[i - startIndex] = currTok[i];
      }
      out_pLineBuffer[i] = '\0';
      
      return true;
   }
}

bool
PNOptions::validateOptions() const
{
   // DMMTODO
   AssertFatal(isAttached(), "validateOptions(): not attached to a file");

//   PseudoTrace& tracer = PseudoTrace::getTracerObject();
//   tracer.pushVerbosityLevel(-1);
//   tracer << " --- DMMNOTE --- PNOptions::validateOptions(): implement\n";
//   tracer.popVerbosityLevel();

   return true;
}

bool
PNOptions::attachResponseFile(const char* in_pFileName)
{
   AssertFatal(in_pFileName != NULL, "Invalid filename");

   FileRStream frs(in_pFileName);

   if (frs.getStatus() != STRM_OK ||
       frs.getSize()   <= 0)
      return false;
      
   m_fileSize         = frs.getSize();
   m_pFileBuffer      = new char[m_fileSize + 1];

   if (m_pFileBuffer == NULL)
      return false;

   frs.read(m_fileSize, m_pFileBuffer);
   m_pFileBuffer[m_fileSize] = '\0';

   delete [] m_pFileName;
   m_pFileName = new char[strlen(in_pFileName) + 1];
   strcpy(m_pFileName, in_pFileName);
      
   return true;
}

void
PNOptions::processResponseFile(PNFileList& out_rFileList)
{
   AssertFatal(isAttached(), "processResponseFile(): not attached to a file");

   bool listingBitmaps = false;

   char pLineBuffer[1024];
   while (getNextLine(pLineBuffer, 1023) != false) {
      if (listingBitmaps == false) {

         try {
            if (parseLine(pLineBuffer) == false) {
               listingBitmaps = true;
            }
         } catch (const char* pBadLine) {
            cerr << "Bad line: " << pBadLine << '\n'
                 << " ... Terminating\n\n";
            exit(1);
         }
      } else {
         out_rFileList.processFileString(pLineBuffer);
      }
   }
}

void
PNOptions::unattachResponseFile()
{
   delete [] m_pFileName;
   m_pFileName = NULL;
   
   delete [] m_pFileBuffer;
   m_pFileBuffer       = NULL;
   m_fileSize          = 0;
   m_tokenizationBegun = false;
   
   delete [] m_pOutPath;
   m_pOutPath = NULL;
   delete [] m_pOutputPalName;
   m_pOutputPalName = NULL;
   delete [] m_pBasePalName;
   m_pBasePalName = NULL;
}


//-------------------------------------- This function throws (const char*) if
//                                        the line syntax is bad.  In particular,
//                                        for right now, it simply throws in_pLine
bool
PNOptions::parseLine(const char* in_pLine)
{
   AssertFatal(in_pLine != NULL, "No line passed to ParseLine");

   const char* pOpenParen  = strchr(in_pLine, '(');
   const char* pCloseParen = strchr(in_pLine, ')');
   
   if (pOpenParen  == NULL ||
       pCloseParen == NULL) {
      throw in_pLine;
   }

   // Extract the portion of the line we will compare to the symbol table, and the
   //  portion we will pass to the parsing function...
   //
   int   i;
   char  functionName[256];
   char  argumentList[256];

   for (i = 0; i < pOpenParen - in_pLine; i++)
      functionName[i] = in_pLine[i];
   functionName[i] = '\0';
   
   for (i = 0; i < pCloseParen - pOpenParen - 1; i++)
      argumentList[i] = pOpenParen[i + 1];
   argumentList[i] = '\0';


   // Search the function table for the appropriate function...
   //
   int j;
   for (j = 0; sg_functionTable[j].pKeyWord != NULL; j++) {
      if (stricmp(functionName, sg_functionTable[j].pKeyWord) == 0)
         break;
   }
   
   if (sg_functionTable[j].pFunction == NULL) {
      throw in_pLine;
   }

   return (this->*(sg_functionTable[j].pFunction))(argumentList);
}


//------------------------------------------------------------------------------
//--------------------------------------
// Parsing functions...
//--------------------------------------
//
bool
PNOptions::quantizeAlpha(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "quantizeAlpha(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   if (m_colorSpace != ColorSpaceUndetermined) {
      throw "quantizeAlpha() : ColorSpace already set";
   }
   
   m_colorSpace = ColorSpaceAlpha;
   return true;
}

bool
PNOptions::quantizeRGB(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "quantizeRGB(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   if (m_colorSpace != ColorSpaceUndetermined) {
      throw "quantizeRGB() : ColorSpace already set";
   }
   
   m_colorSpace = ColorSpaceRGB;
   return true;
}

bool
PNOptions::quantizeAdditive(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "quantizeAdditive(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   if (m_colorSpace != ColorSpaceUndetermined) {
      throw "quantizeAdditive() : ColorSpace already set";
   }
   
   m_colorSpace = ColorSpaceAdditive;
   return true;
}

bool
PNOptions::quantizeSubtractive(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "quantizeSubtractive(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   if (m_colorSpace != ColorSpaceUndetermined) {
      throw "quantizeSubtractive() : ColorSpace already set";
   }
   
   m_colorSpace = ColorSpaceSubtractive;
   return true;
}

bool
PNOptions::chooseRange(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "chooseRange(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   int first, last;
   int numScanned = sscanf(in_pArgs, "%d, %d", &first, &last);
   
   if (numScanned != 2) {
      tracer.setErrorState();
      tracer << "chooseRange: Unable to properly scan args\n";
      throw in_pArgs;
   }

   if (first == 0) {
      tracer.setErrorState();
      tracer << "chooseRange: illegal range, max range is (1,255), scanned: ("
             << first << ", " << last << ")\n";
      throw in_pArgs;
   }

   m_choosingColors = true;
   m_chooseFirst    = first;
   m_chooseLast     = last;

   tracer.pushVerbosityLevel(4);
   tracer << " - Scanned range: (" << m_chooseFirst << ", " << m_chooseLast << ")\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
PNOptions::renderRange(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "renderRange(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   int first, last;
   int numScanned = sscanf(in_pArgs, "%d, %d", &first, &last);
   
   if (numScanned != 2) {
      tracer.setErrorState();
      tracer << "renderRange: Unable to properly scan args\n";
      throw in_pArgs;
   }

   if (first == 0) {
      tracer.setErrorState();
      tracer << "renderRange: illegal range, max range is (1,255), scanned: ("
             << first << ", " << last << ")\n";
      tracer << "Note that transparent bitmaps and alpha bitmaps will automagically use color 0\n";
      throw in_pArgs;
   }

   m_renderingBitmaps = true;
   m_renderFirst      = first;
   m_renderLast       = last;

   tracer.pushVerbosityLevel(4);
   tracer << " - Scanned range: (" << m_renderFirst << ", " << m_renderLast << ")\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
PNOptions::outputPath(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "outputPath(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   delete [] m_pOutPath;
   m_pOutPath = new char[strlen(in_pArgs) + 4];
   strcpy(m_pOutPath, in_pArgs);
   
   if (strlen(m_pOutPath) == 0) {
      // Assume the current directory...
      //
      strcpy(m_pOutPath, ".\\");

      tracer.pushVerbosityLevel(1);
      tracer << " - Warning, no outpath specified: using current directory\n";
      tracer.popVerbosityLevel();
   } else {
      // make sure the the path is backslash terminated
      //
      int strLen = strlen(m_pOutPath);
      if (m_pOutPath[strLen - 1] != '\\') {
         m_pOutPath[strLen] = '\\';
         m_pOutPath[strLen + 1] = '\0';
      }
   }

   tracer.pushVerbosityLevel(4);
   tracer << " - Set output path to: " << m_pOutPath << "\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
PNOptions::palOutputName(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "palOutputName(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   delete [] m_pOutputPalName;
   m_pOutputPalName = new char[strlen(in_pArgs) + 2];
   strcpy(m_pOutputPalName, in_pArgs);

   tracer.pushVerbosityLevel(4);
   tracer << " - Set output palette name to: " << m_pOutputPalName << "\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
PNOptions::basePalName(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "basePalName(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   delete [] m_pBasePalName;
   m_pBasePalName = new char[strlen(in_pArgs) + 2];
   strcpy(m_pBasePalName, in_pArgs);

   tracer.pushVerbosityLevel(4);
   tracer << " - Set base palette name to: " << m_pBasePalName << "\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
PNOptions::zeroColor(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "zeroColor(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   int r, g, b;
   int numScanned = sscanf(in_pArgs, "%d, %d, %d", &r, &g, &b);
   
   if (numScanned != 3) {
      tracer.setErrorState();
      tracer << "zeroColor: Unable to properly scan args\n";
      throw in_pArgs;
   }

   m_zeroColor.red   = float(r) / 255.0f;
   m_zeroColor.green = float(g) / 255.0f;
   m_zeroColor.blue  = float(b) / 255.0f;

   tracer.pushVerbosityLevel(4);
   tracer << " - Set zeroColor to ("
          << m_zeroColor.red   << ", "
          << m_zeroColor.green << ", "
          << m_zeroColor.blue  << ")\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
PNOptions::quantizeBitsRGB(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "quantizeBitsRGB(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   int rBits;
   int gBits;
   int bBits;
   int numScanned = sscanf(in_pArgs, "%d, %d, %d", &rBits, &gBits, &bBits);
   
   if (numScanned != 3) {
      tracer.setErrorState();
      tracer << "quantizeBitsRGB: Unable to properly scan args\n";
      throw in_pArgs;
   }

   m_numRBits = rBits;
   m_numGBits = gBits;
   m_numBBits = bBits;

   tracer.pushVerbosityLevel(4);
   tracer << " - Set rgbBits to ("
          << m_numRBits << ", "
          << m_numGBits << ", "
          << m_numBBits << ")\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
PNOptions::quantizeBitsAlpha(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "quantizeBitsAlpha(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   int alphaBits;
   int numScanned = sscanf(in_pArgs, "%d", &alphaBits);
   
   if (numScanned != 1) {
      tracer.setErrorState();
      tracer << "quantizeBitsAlpha: Unable to properly scan args\n";
      throw in_pArgs;
   }

   m_numABits = alphaBits;

   tracer.pushVerbosityLevel(4);
   tracer << " - Set alphaBits to "
          << m_numABits << "\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
PNOptions::paletteKey(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "paletteKey(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   int paletteKey;
   int numScanned = sscanf(in_pArgs, "%lu", &paletteKey);
   
   if (numScanned != 1) {
      tracer.setErrorState();
      tracer << "paletteKey: Unable to properly scan args\n";
      throw in_pArgs;
   }

   m_paletteKey = paletteKey;

   tracer.pushVerbosityLevel(4);
   tracer << " - Set paletteKey to "
          << m_paletteKey << "\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
PNOptions::noRender(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "noRender(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   m_overrideRenderRange = true;
   return true;
}

bool
PNOptions::stripQuantizationInfo(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "stripQuantizationInfo(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   m_stripQuantizationInfo = true;

   return true;
}

bool
PNOptions::extrudeMipLevels(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "extrudeMipLevels(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   m_extrudingMipLevels = true;

   return true;
}

bool
PNOptions::enableDithering(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "enableDithering(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   m_areDithering = true;

   return true;
}

bool
PNOptions::ditherTolerance(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "ditherTolerance(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   int ditherTolerance;
   int numScanned = sscanf(in_pArgs, "%lu", &ditherTolerance);
   
   if (numScanned != 1) {
      tracer.setErrorState();
      tracer << "ditherTolerance: Unable to properly scan args\n";
      throw in_pArgs;
   }

   m_ditherTolerance = ditherTolerance;

   tracer.pushVerbosityLevel(4);
   tracer << " - Set ditherTolerance to "
          << m_ditherTolerance << "\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
PNOptions::standAlonePalette(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "standAlonePalette(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   m_standAlonePalette = true;

   return true;
}

bool
PNOptions::enforcePhoenixSave(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "enforcePhoenixSave(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   m_asPhoenix = true;

   return true;
}

bool
PNOptions::maintainEdges(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "maintainEdges(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   m_maintainEdges = true;

   return true;
}

bool
PNOptions::bitmapListStart(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "bitmapListStart(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   return false;
}

