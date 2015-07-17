//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <process.h>
#include <stdio.h>
#include <resManager.h>
#include <volstrm.h>
#include <ml.h>
#include "itrShape.h"
#include "itrShapeEdit.h"
#include <windows.h>
#include <tString.h>

void buildGeometry(const String& in_inputVolumeName,
                   const String& in_geometryFileName);
void buildLighting(const ITRShapeEdit* in_pEditShape,
                   const String& in_inputVolumeName,
                   const String& in_geometryFileName,
                   const String& in_lightingFileName);

namespace {

bool g_highDetailBuild = false;
bool g_isLinked = false;

const char* const sg_pProgramName        = "zedShape";
const char* const sg_pFileVersion        = "3.9";

const UInt32 sg_linkableDefaultAll = ITRShape::sm_allFacesLinkable;

void
usage(const char* in_progName)
{
   char usageBuffer[1024];
   sprintf(usageBuffer, "Usage: %s <zedVolume>.zvl [-h]\n", in_progName);
   MessageBox(NULL, usageBuffer, "Program Usage", MB_ICONEXCLAMATION | MB_OK);
}

void lowerFileName(String& in_rFileName)
{
   for (String::iterator itr = in_rFileName.begin();
        itr != in_rFileName.end(); itr++) {
      *itr = tolower(*itr);
   }
}

void
reextendFileName(const String& in_rFileName,
                 const String& in_rNewExtension,
                 String&       out_rNewFileName)
{
   out_rNewFileName = in_rFileName;
   String::iterator itr;
   for (itr = out_rNewFileName.end(); itr != out_rNewFileName.begin(); --itr) {
      if (*itr == '.')
         break;
   }
   if (itr == out_rNewFileName.begin() ||
       (out_rNewFileName.end() - itr) < in_rNewExtension.length()) {
      // There was no extension, or not enough room for the new one...
      //
      char tmpBuf[512];
      strcpy(tmpBuf, out_rNewFileName.c_str());
      strcat(tmpBuf, in_rNewExtension.c_str());
      out_rNewFileName = tmpBuf;
   } else {
      char* pExt = itr;
      strcpy(pExt, in_rNewExtension.c_str());
   }
}

void
dumpFileToVolumeAndRemove(const String&   in_outVolName,
                          const String&   in_fileName)
{
   // Do the dump...
   //
   char* argv[5];
   argv[0] = "vt.exe";
   argv[1] = "-sp";
   argv[2] = const_cast<char*>(in_outVolName.c_str());
   argv[3] = const_cast<char*>(in_fileName.c_str());
   argv[4] = NULL;

   spawnvp(P_WAIT, "vt.exe", argv);
   DeleteFile(in_fileName.c_str());
}


void
dumpShapeToVolume(const String&   in_outVolName,
                  const String&   in_shapeName,
                  const ITRShape* in_pShape)
{
   in_pShape->fileStore(in_shapeName.c_str());
   dumpFileToVolumeAndRemove(in_outVolName, in_shapeName);
}

};

bool extractFile( VolumeRStream & io_volStream,  const String & in_fileName )
{
   if( !io_volStream.open( in_fileName.c_str() ) )
      return( false );
      
   char* buffer = new char[io_volStream.getSize()];
   io_volStream.read(io_volStream.getSize(), buffer);
   
   FileWStream output(in_fileName.c_str());
   output.write(io_volStream.getSize(), buffer);
   output.close();
   
   delete [] buffer;
   io_volStream.close();
   return( true );
}

int
main(int argc, char* argv[])
{
   printf("%s v%s\n", sg_pProgramName, sg_pFileVersion);
   if( argc < 2 || argc > 4 )
   {
      usage(sg_pProgramName);
      return EXIT_FAILURE;
   }

   if( argc > 2 )
   {
      for( int i = 3; i <= argc; i++ )
      {
         if( !stricmp( "-h", argv[i-1] ) )
            g_highDetailBuild = true;
      }
   }
   
   // To begin, open the volume, and first read in the shape description file
   //
   Persistent::Base::Error err;
   VolumeRWStream digOutputVolume;
   VolumeRStream  zedVolume;

   String inputArg = argv[1], actualVol;
   reextendFileName(inputArg, String(".zvl"), actualVol);

   Bool test = zedVolume.openVolume(actualVol.c_str());
   AssertISV(test != false, avar("Could not open zedVolume: %s", actualVol.c_str()));

   test = zedVolume.open("input.its");
   AssertISV(test != false, "Could not open shape options (Not a zedVolume?)");

   ITRShapeEdit* pEditShape =
      dynamic_cast<ITRShapeEdit*>(Persistent::Base::load(zedVolume, &err));
   AssertISV(pEditShape != NULL, "Shape options invalid (?)");

   // OK, export to a proper ITRShape object
   //
   ITRShape* pShape = new ITRShape;
   pEditShape->exportToITRShape(pShape);

   // Find out if the shape is a linked interior...
   //
   if (pShape->isLinked() == true) {
      g_isLinked = true;
   } else {
      g_isLinked = false;
   }

   // Open the output volume, and write out the shape...
   //
   String outputFile;
   String shapeFile;

   reextendFileName(inputArg, String(".div"), outputFile);
   lowerFileName(outputFile);
   reextendFileName(inputArg, String(".dis"), shapeFile);
   dumpShapeToVolume(outputFile, shapeFile, pShape);

   digOutputVolume.closeVolume();
   
   for (int i = 0; i < pEditShape->m_pStates.size(); i++) {
      ITRShapeEdit::EditState* pEditState = pEditShape->m_pStates[i];
      for (int j = 0; j < pEditState->m_pLODVector.size(); j++) {
         ITRShapeEdit::EditLOD* pLOD = pEditState->m_pLODVector[j];
         
         buildGeometry(actualVol,pLOD->m_geometryFileName);
         AssertISV(pLOD->m_pLightStates.size() == pEditShape->m_numLightStates,
                   "Inappropriate number of light states in LOD");
         for (int k = 0; k < pEditShape->m_numLightStates; k++) {
            ITRShapeEdit::EditLightState* pLS = pLOD->m_pLightStates[k];
            
            buildLighting(pEditShape, actualVol,
                          pLOD->m_geometryFileName,
                          pLS->m_lightFileName);
            String dilFileName;
            reextendFileName(pLS->m_lightFileName, String(".dil"), dilFileName);
            dumpFileToVolumeAndRemove(outputFile, dilFileName);
         }
         
         String digFileName;
         reextendFileName(pLOD->m_geometryFileName, String(".dig"), digFileName);
         dumpFileToVolumeAndRemove(outputFile, digFileName);
      }
   }

   // add the material list to the volume ( if it exists - 1.80 + will have it )
   String materialFile;
   reextendFileName( inputArg, String(".dml"), materialFile );
   
   // open the file and write to the new volume
   if( extractFile( zedVolume, materialFile ) )
      dumpFileToVolumeAndRemove( outputFile, materialFile );
   
   zedVolume.close();

   return EXIT_SUCCESS;
}

void buildGeometry(const String& in_inputVolumeName,
              const String& in_geometryFileName)
{
   String optionsFileName;
   reextendFileName(in_geometryFileName, String(".igo"), optionsFileName);
   
   // Ok, we have the information required to start the build tool,
   //  Arguments to said tool are:
   //    0: inputVolume
   //    1: geometry (.3dm) file
   //    2: options filename
   //
   char* argv[7];
   argv[0] = "zedBuild.exe";
   argv[1] = const_cast<char*>(in_inputVolumeName.c_str());
   argv[2] = const_cast<char*>(in_geometryFileName.c_str());
   argv[3] = const_cast<char*>(optionsFileName.c_str());
   argv[4] = NULL;

   int extraArg = 4;
   if (g_highDetailBuild == true) {
      argv[extraArg++] = "-p";
      argv[extraArg] = NULL;
   }
   if (g_isLinked == true) {
      argv[extraArg++] = "-l";
      argv[extraArg] = NULL;
   }
   
   spawnvp(P_WAIT, "zedBuild.exe", argv);
}

void
buildLighting(const ITRShapeEdit* in_pEditShape,
              const String& in_inputVolumeName,
              const String& in_geometryFileName,
              const String& in_lightingFileName)
{
   String optionsFileName;
   reextendFileName(in_lightingFileName, String(".ilo"), optionsFileName);
   
   String geometryFileName;
   reextendFileName(in_geometryFileName, String(".dig"), geometryFileName);
   
   // Ok, we have the information required to start the build tool,
   //  Arguments to said tool are:
   //    1: inputVolume
   //    2: geometry (.dig) file
   //    3: lighting (.ilt) filename
   //    4: options filename
   //    5: material filename
   //    6: -f if filtering..
   //    7: -a if no outside ambient..
   //
   char* argv[8];
   argv[0] = "zedLight.exe";
   argv[1] = const_cast<char*>(in_inputVolumeName.c_str());
   argv[2] = const_cast<char*>(geometryFileName.c_str());
   argv[3] = const_cast<char*>(in_lightingFileName.c_str());
   argv[4] = const_cast<char*>(optionsFileName.c_str());
   argv[5] = const_cast<char*>(in_pEditShape->m_materialList.c_str());
   
   int argc = 6;
   if( g_highDetailBuild == true )
      argv[argc++] = "-f";
   argv[argc] = NULL;

   int returnVal = spawnvp(P_WAIT, "zedLight.exe", argv);
   AssertISV( returnVal == EXIT_SUCCESS,
      avar("command line: \"zedBuild.exe %s %s %s %s %s %s %s\" failed",
         argv[1], argv[2], argv[3], argv[4], argv[5], ( argc > 6 ) ? argv[6] : "", ( argc > 7 ) ? argv[7] : "" ) );
}
