//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <process.h>
#include <stdio.h>
#include <core.h>
#include <itrgeometry.h>
#if 0
#include <itrtsimport.h>
#endif
#include <itr3dmimport.h>
#include <itrbsp.h>
#include <itrportal.h>
#include <itrShapeEdit.h>


//----------------------------------------------------------------------------

static const char* const Version = "3.9";

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
extractFile(VolumeRStream& io_volStream,
            const String&  in_fileName)
{
   Bool test = io_volStream.open(in_fileName.c_str());
   AssertISV(test != false, avar("Could not open file %s", in_fileName.c_str()));
   
   char* buffer = new char[io_volStream.getSize()];
   io_volStream.read(io_volStream.getSize(), buffer);
   
   FileWStream output(in_fileName.c_str());
   output.write(io_volStream.getSize(), buffer);
   output.close();
   
   delete [] buffer;
   io_volStream.close();
}

void
removeFile(const String& in_removeFile)
{
   DeleteFile(const_cast<char*>(in_removeFile.c_str()));
}


//----------------------------------------------------------------------------

int
main(int argc, char* argv[])
{
	if (argc == 1)  {
		printf("zedBuild version %s\n",Version);
      printf("usage: <inputVolume>.zvl <geometryFile>.3dm <optionsFile>.igo [-p] [-l]\n");
      printf(" where -p indicates portilization should be performed.\n");
      printf("       -l indicates interior is linked.\n");
      exit(1);
   }

   VolumeRStream inputVolume;
   Bool test = inputVolume.openVolume(argv[1]);
   AssertISV(test != false, avar("Could not open volume: %s", argv[1]));

   // Load the .igo file first...
   //
   test = inputVolume.open(argv[3]);
   AssertISV(test != false, avar("Could not open options file: %s", argv[3]));

   Persistent::Base::Error err;
   ITRBuildOptions* itrBuildOptions =
      dynamic_cast<ITRBuildOptions*>(Persistent::Base::load(inputVolume, &err));
   AssertISV(itrBuildOptions != NULL,
             avar("File: %s does not contain an ITRBuildOptions", argv[3]));
   inputVolume.close();

   // Extract the .3dm file we want...
   //
   String geometryFileName = argv[2];
   extractFile(inputVolume, geometryFileName);

   ITR3DMImport::scale             = itrBuildOptions->m_geometryScale;
   ITR3DMImport::snapPrecision     = itrBuildOptions->m_pointSnapPrecision;
   ITR3DMImport::normalPrecision   = itrBuildOptions->m_planeNormalPrecision;
   ITR3DMImport::distancePrecision = itrBuildOptions->m_planeDistancePrecision;
   ITR3DMImport::textureScale      = itrBuildOptions->m_textureScale;
   
   ITR3DMImport::lowDetailInterior = itrBuildOptions->m_lowDetail;
   ITR3DMImport::setMaxMipLevel(itrBuildOptions->m_maximumTexMip);

   ITRBSPBuild::normalPrecision    = itrBuildOptions->m_planeNormalPrecision;
   ITRBSPBuild::distancePrecision  = itrBuildOptions->m_planeDistancePrecision;

   ITRPortal::normalPrecision   = itrBuildOptions->m_planeNormalPrecision;
   ITRPortal::distancePrecision = itrBuildOptions->m_planeDistancePrecision;

   // Grab any extra params...
   //
   ITRPortal::portalClipping = false;
   bool buildAsLinked        = false;
   for (int arg = 4; arg < argc; arg++) {
      if (argv[arg][0] == '-') {
         if (argv[arg][1] == 'p') {
            printf(" - portal Clipping enabled\n");
            ITRPortal::portalClipping = true;
         }
         if (argv[arg][1] == 'l') {
            printf(" - linked interior enabled\n");
            buildAsLinked = true;
         }
      }
   }

   Vector<UInt32> volumeMasks;
   ITRGeometry    geometry;
   geometry.textureScale = itrBuildOptions->m_textureScale;
   DWORD timerStart = GetTickCount();

   ITR3DMImport::import(geometryFileName.c_str(), &geometry, &volumeMasks);
   ITRBSPBuild::buildTree(&geometry, &volumeMasks, buildAsLinked);
   ITRPortal::buildPVS(&geometry);

   // If this is a linked interior, we need to have no surfaces marked w/ the outside
   //  bit...
   //
   if (buildAsLinked == true) {
      printf(" - Overriding outside visibility bits for linked interior...\n");
      for (int i = 0; i < geometry.surfaceList.size(); i++) {
         ITRGeometry::Surface& rSurface = geometry.surfaceList[i];
         rSurface.visibleToOutside = 0;
      }
   }

   String outputGeometryFileName;
   reextendFileName(geometryFileName, String(".dig"), outputGeometryFileName);
   geometry.fileStore(outputGeometryFileName.c_str());

   Int32 timerTotal = GetTickCount() - timerStart;
   printf("Elapse Time: %.2f sec.\n", float(timerTotal) / 1000.0f);

   // Clean up our temps and tempfiles
   removeFile(geometryFileName);
   delete itrBuildOptions;
   
   return 0;
}
