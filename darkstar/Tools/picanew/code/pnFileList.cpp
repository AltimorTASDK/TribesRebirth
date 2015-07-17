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

#include "pnFileList.h"
#include "pnPseudoTrace.h"
#include "pnUnquantizedImage.h"

PNFileList::PNFileList()
{
   //
}

PNFileList::~PNFileList()
{
   for (int i = 0; i < m_fileEntries.size(); i++) {
      delete [] m_fileEntries[i].pFileName;
      m_fileEntries[i].pFileName = NULL;

      delete m_fileEntries[i].pUQImage;
      m_fileEntries[i].pUQImage = NULL;
   }
   m_fileEntries.clear();
}


void
PNFileList::processFileString(const char* in_pFileLine)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();

   // First, we need to determine if we have a weight to extract...
   //
   UInt32 weight = 1;

   char  lineProper[512];
   char  weightLine[512];
   const char* pOpenParen;
   const char* pCloseParen;
   if ((pOpenParen = strchr(in_pFileLine, '(')) != NULL) {
      // Ok, there is a weight here...
      //
      pCloseParen = strchr(in_pFileLine, ')');
      
      if (pCloseParen == NULL) {
         tracer.setErrorState();
         tracer << "Error, unbalanced paren in file weight spec. [" << in_pFileLine
                << "] Exiting...\n\n";
         exit(1);
      }
      if (pCloseParen == (pOpenParen + 1)) {
         tracer.setErrorState();
         tracer << "Error, empty weight spec in file [" << in_pFileLine
                << "] Exiting...\n\n";
         exit(1);
      }
      
      int i;
      for (i = 0; pOpenParen + i < pCloseParen - 1; i++)
         weightLine[i] = pOpenParen[i + 1];
      weightLine[i] = '\0';
      weight = atoi(weightLine);
      
      // Now put the file into the lineProper buffer...
      //
      for (i = 0; in_pFileLine + i < pOpenParen; i++)
         lineProper[i] = in_pFileLine[i];
      lineProper[i] = '\0';
   } else {
      // No weight, just copy the filename...
      //
      strcpy(lineProper, in_pFileLine);
   }

   if (strchr(lineProper, '*') != NULL ||
       strchr(lineProper, '?') != NULL) {
      // Line contains wild card...
      //
      tracer.pushVerbosityLevel(4);
      tracer << " --- wildcard fileLine " << lineProper << "\n";

      // First, we need to determine if there is a path that we will need to 
      //  prepend to the filenames returned from the Win32 find functions...
      //
      char  prepBuffer[128];
      char* lastSlash = strrchr(lineProper, '\\');
      if (lastSlash != NULL) {
         int i;
         for (i = 0; lineProper + i <= lastSlash; i++)
            prepBuffer[i] = lineProper[i];
         prepBuffer[i] = '\0';
      } else {
         prepBuffer[0] = '\0';
      }

      // Expand the wild card.  First, search for the first instance of this file
      HANDLE winHandle;
      WIN32_FIND_DATA fillStruct;
      winHandle = FindFirstFile(lineProper, &fillStruct);
      
      if (winHandle != INVALID_HANDLE_VALUE) {
         // Copy out the first file...
         //
         m_fileEntries.increment();
         PNFileEntry& firstEntry = m_fileEntries.last();
         firstEntry.fileWeight = weight;
         firstEntry.pFileName  =
            new char[strlen(prepBuffer) + strlen(fillStruct.cFileName) + 1];
         strcpy(firstEntry.pFileName, prepBuffer);
         strcat(firstEntry.pFileName, fillStruct.cFileName);
         firstEntry.pUQImage = NULL;

         tracer << "Added file: " << firstEntry.pFileName
                << " weight: " << firstEntry.fileWeight << "\n";

         // And then whatever remains...
         //
         while (FindNextFile(winHandle, &fillStruct) != FALSE) {
            m_fileEntries.increment();
            PNFileEntry& rEntry = m_fileEntries.last();
            rEntry.fileWeight = weight;
            rEntry.pFileName  =
               new char[strlen(prepBuffer) + strlen(fillStruct.cFileName) + 1];
            strcpy(rEntry.pFileName, prepBuffer);
            strcat(rEntry.pFileName, fillStruct.cFileName);
            rEntry.pUQImage = NULL;

            tracer << "Added file: " << rEntry.pFileName
                   << " weight: " << rEntry.fileWeight << "\n";
         }

         tracer << " --- end wildcard search\n";
         FindClose(winHandle);
      } else {
         // No files matched the search string or some other error occured...
         //
         tracer << " --- no files matched wildcard, or error occurred\n";
      }

      tracer.popVerbosityLevel();
   } else {
      // Line is normal...
      //
      m_fileEntries.increment();
      PNFileEntry& rEntry = m_fileEntries.last();
            
      rEntry.fileWeight = weight;
      rEntry.pUQImage = NULL;
      rEntry.pFileName  = new char[strlen(lineProper) + 1];
      strcpy(rEntry.pFileName, lineProper);
      
      tracer.pushVerbosityLevel(4);
      tracer << "Added file: " << rEntry.pFileName
             << " weight: " << rEntry.fileWeight << "\n";
      tracer.popVerbosityLevel();
   }
}

