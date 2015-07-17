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

#include "mpmOptions.h"
#include "mpmFileList.h"
#include "mpmState.h"
#include "mpmPseudoTrace.h"

namespace {

typedef bool (MPMOptions::*MPMParseFunction)(const char*);

//--------------------------------------
// Parse keyword->functions table
//--------------------------------------
//
struct KeywordFunctionMapEntry {
   const char*     pKeyWord;
   MPMParseFunction pFunction;
};

KeywordFunctionMapEntry sg_functionTable[] =
{
   {"masterRenderRange",     MPMOptions::masterRenderRange}, // quantization type
   {"masterChooseRange",     MPMOptions::masterChooseRange}, // quantization type

   {"shadeColor",             MPMOptions::shadeColor},
   {"hazeColor",              MPMOptions::hazeColor},
   {"shadeHazeBiasLevels",    MPMOptions::shadeHazeBiasLevels},
   {"shadeHazeBiasFactor",    MPMOptions::shadeHazeBiasFactor},
   {"shadeHazeLevels",        MPMOptions::shadeHazeLevels},

   {"palOutputName",          MPMOptions::palOutputName},
   {"basePalName",            MPMOptions::basePalName},
   
   {"paletteListStart",       MPMOptions::paletteListStart},   // only parseFunction that returns
                                                               //  false.
   {NULL,                     NULL}
};

}; // namespace {}


MPMOptions::MPMOptions()
 : m_pFileName(NULL),
   m_fileSize(0),
   m_pFileBuffer(NULL),
   m_tokenizationBegun(false),
   m_pOutputPalName(NULL),
   m_pBasePalName(NULL)
{
   m_masterRenderFirst = 1;
   m_masterRenderLast  = 254;
   m_masterChooseFirst = 1;
   m_masterChooseLast  = 254;

   m_shadeColor.set(0.0f, 0.0f, 0.0f);
   m_shadePercentage = 1.0f;

   m_hazeColor.set(0.5f, 0.5f, 0.5f);
   m_hazePercentage = 1.0f;

   m_numShadeBiasLevels  = 1;
   m_numHazeBiasLevels   = 1;
   m_shadeHazeBiasFactor = 1.0f;

   m_numShadeLevels = 0;
   m_numHazeLevels  = 0;
}

MPMOptions::~MPMOptions()
{
   delete [] m_pOutputPalName;
   m_pOutputPalName = NULL;
   delete [] m_pBasePalName;
   m_pBasePalName = NULL;

   unattachResponseFile();
}


bool
MPMOptions::getNextLine(char         out_pLineBuffer[],
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
MPMOptions::validateOptions() const
{
   // DMMTODO
   AssertFatal(isAttached(), "validateOptions(): not attached to a file");

//   PseudoTrace& tracer = PseudoTrace::getTracerObject();
//   tracer.pushVerbosityLevel(-1);
//   tracer << " --- DMMNOTE --- MPMOptions::validateOptions(): implement\n";
//   tracer.popVerbosityLevel();

   return true;
}

bool
MPMOptions::attachResponseFile(const char* in_pFileName)
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
MPMOptions::processResponseFile(MPMFileList& out_rFileList)
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
MPMOptions::unattachResponseFile()
{
   delete [] m_pFileName;
   m_pFileName = NULL;
   
   delete [] m_pFileBuffer;
   m_pFileBuffer       = NULL;
   m_fileSize          = 0;
   m_tokenizationBegun = false;
}


//-------------------------------------- This function throws (const char*) if
//                                        the line syntax is bad.  In particular,
//                                        for right now, it simply throws in_pLine
bool
MPMOptions::parseLine(const char* in_pLine)
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
MPMOptions::masterRenderRange(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "masterRenderRange(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   int first, last;
   int numScanned = sscanf(in_pArgs, "%d, %d", &first, &last);
   
   if (numScanned != 2) {
      tracer.setErrorState();
      tracer << "masterRenderRange: Unable to properly scan args\n";
      throw in_pArgs;
   }


   if (first == 0 || last > 253) {
      tracer.setErrorState();
      tracer << "chooseRange: illegal range, max range is (1,253), scanned: ("
             << first << ", " << last << ")\n";
      throw in_pArgs;
   }

   m_masterRenderFirst = first;
   m_masterRenderLast  = last;

   tracer.pushVerbosityLevel(4);
   tracer << " - Scanned range: (" << m_masterRenderFirst << ", " << m_masterRenderLast << ")\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
MPMOptions::masterChooseRange(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "masterChooseRange(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   int first, last;
   int numScanned = sscanf(in_pArgs, "%d, %d", &first, &last);
   
   if (numScanned != 2) {
      tracer.setErrorState();
      tracer << "masterChooseRange: Unable to properly scan args\n";
      throw in_pArgs;
   }

   if (first == 0 || last > 253) {
      tracer.setErrorState();
      tracer << "chooseRange: illegal range, max range is (1,253), scanned: ("
             << first << ", " << last << ")\n";
      throw in_pArgs;
   }

   m_masterChooseFirst = first;
   m_masterChooseLast  = last;

   tracer.pushVerbosityLevel(4);
   tracer << " - Scanned range: (" << m_masterChooseFirst << ", " << m_masterChooseLast << ")\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
MPMOptions::shadeColor(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "shadeColor(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   int r, g, b;
   float percentage;
   int numScanned = sscanf(in_pArgs, "%d, %d, %d, %f", &r, &g, &b, &percentage);
   
   if (numScanned != 4) {
      tracer.setErrorState();
      tracer << "shadeColor: Unable to properly scan args\n";
      throw in_pArgs;
   }

   m_shadeColor.red   = r;
   m_shadeColor.green = g;
   m_shadeColor.blue  = b;
   m_shadePercentage  = percentage;
   
   tracer.pushVerbosityLevel(4);
   tracer << " - Set shadeColor to ("
          << m_shadeColor.red   << ", "
          << m_shadeColor.green << ", "
          << m_shadeColor.blue  << ")\n"
          << " - At " << m_shadePercentage * 100.0f << "%\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
MPMOptions::hazeColor(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "hazeColor(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   int r, g, b;
   float percentage;
   int numScanned = sscanf(in_pArgs, "%d, %d, %d, %f", &r, &g, &b, &percentage);
   
   if (numScanned != 4) {
      tracer.setErrorState();
      tracer << "hazeColor: Unable to properly scan args\n";
      throw in_pArgs;
   }

   m_hazeColor.red   = float(r) / 255.0f;
   m_hazeColor.green = float(g) / 255.0f;
   m_hazeColor.blue  = float(b) / 255.0f;
   m_hazePercentage  = percentage;
   
   tracer.pushVerbosityLevel(4);
   tracer << " - Set hazeColor to ("
          << m_hazeColor.red   << ", "
          << m_hazeColor.green << ", "
          << m_hazeColor.blue  << ")\n"
          << " - At " << m_hazePercentage * 100.0f << "%\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
MPMOptions::shadeHazeBiasLevels(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "shadeHazeBiasLevels(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   int shade, haze;
   int numScanned = sscanf(in_pArgs, "%d, %d", &shade, &haze);
   
   if (numScanned != 2) {
      tracer.setErrorState();
      tracer << "shadeHazeBiasLevels: Unable to properly scan args\n";
      throw in_pArgs;
   }

   m_numShadeBiasLevels = shade;
   m_numHazeBiasLevels  = haze;

   tracer.pushVerbosityLevel(4);
   tracer << " - Set shadeHazeBiasLevels to ("
          << m_numShadeBiasLevels << ", "
          << m_numHazeBiasLevels  << ")\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
MPMOptions::shadeHazeBiasFactor(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "shadeHazeBiasFactor(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   float factor;
   int numScanned = sscanf(in_pArgs, "%f", &factor);
   
   if (numScanned != 1) {
      tracer.setErrorState();
      tracer << "shadeHazeBiasFactor: Unable to properly scan args\n";
      throw in_pArgs;
   }

   m_shadeHazeBiasFactor = factor;

   tracer.pushVerbosityLevel(4);
   tracer << " - Set shadeHazeBiasFactor to "
          << m_shadeHazeBiasFactor << "\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
MPMOptions::shadeHazeLevels(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "shadeHazeLevels(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   int shade, haze;
   int numScanned = sscanf(in_pArgs, "%d, %d", &shade, &haze);
   
   if (numScanned != 2) {
      tracer.setErrorState();
      tracer << "shadeHazeLevels: Unable to properly scan args\n";
      throw in_pArgs;
   }

   if (shade == 0 || haze == 0) {
      tracer.setErrorState();
      tracer << "shadeHazeLevels: Haze and Shade must be nonzero powers of 2\n";
      throw in_pArgs;
   }

   if (((shade & (shade - 1)) != 0) ||
       ((haze  & (haze  - 1)) != 0)) {
      tracer.setErrorState();
      tracer << "shadeHazeLevels: Haze and Shade must be a power of 2\n";
      throw in_pArgs;
   }


   m_numShadeLevels = shade;
   m_numHazeLevels  = haze;

   tracer.pushVerbosityLevel(4);
   tracer << " - Set shadeHazeLevels to ("
          << m_numShadeLevels << ", "
          << m_numHazeLevels  << ")\n";
   tracer.popVerbosityLevel();

   return true;
}

bool
MPMOptions::palOutputName(const char* in_pArgs)
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
MPMOptions::basePalName(const char* in_pArgs)
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
MPMOptions::paletteListStart(const char* in_pArgs)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   tracer.pushVerbosityLevel(3);
   tracer << "paletteListStart(" << in_pArgs << ")\n";
   tracer.popVerbosityLevel();

   return false;
}

