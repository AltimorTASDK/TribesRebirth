//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <core.h>
#include <g_bitmap.h>
#include <g_pal.h>
#include <itrgeometry.h>
#include <itrlighting.h>
#include <itrbasiclighting.h>
#include <zedPersLight.h>
#include <itrShapeEdit.h>
#include "tplane.h"
#include "ts_material.h"

using namespace ZedLight;


//----------------------------------------------------------------------------
void initLights(PersLightList&               in_rLightList,
                ITRBasicLighting::LightList& out_rLightList,
                const float                  in_scaleFactor);
void loadMaterialProperties(FILE* fp,ITRBasicLighting::MaterialPropList* matPropList);

const char* const sg_pVersion = "3.9";
float MinIntensity = 0.025f;

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


//----------------------------------------------------------------------------
// Calculate distance at which light fades to MinIntensity
//
float getRadius(float a,float b,float c,int id)
{
   float distance;
   if (a == .0f) {
      if (b == .0f)
         // Light never fades away
         distance = 1.0E20f;
      else {
         // Linear
         distance = (1.0f / MinIntensity - c) / b;
      }
   }
   else {
      // Quadratic
      c -= 1.0f / MinIntensity;
      float sqr = b * b - 4.0f * a * c;
      if (sqr < .0f) {
         printf("   Warning: light %d distance coeficients bad\n",id);
         return .0f;
      }
      float sgnb = (b < .0f)? -1.0f: 1.0f;
      float q = -0.5 * (b + sgnb * sqrt(sqr));

      // Largest positive root?
      distance = q / a;
      if (distance < .0f)
         distance = .0f;
      if (q != .0f) {
         float x2 = c / q;
         if (x2 > distance)
            distance = x2;
      }
   }
   return distance;
}



//----------------------------------------------------------------------------

int main(int argc,char** argv)
{
	if (argc == 1)  {
		printf("zedLight version %s\n", sg_pVersion);
      printf("usage: <input>.zvl <geom>.dig <light>.ilt <options>.ilo <matList>.dml [-f]\n");
      printf(" where -f indicates filtering should be performed.\n");
      exit(1);
   }

   VolumeRStream inputVolume;
   Bool test = inputVolume.openVolume(argv[1]);
   AssertISV(test != false, avar("Could not open volume: %s", argv[1]));

   // Load the .igo file first...
   //
   test = inputVolume.open(argv[4]);
   AssertISV(test != false, avar("Could not open options file: %s", argv[4]));

   Persistent::Base::Error err;
   Persistent::Base * obj = Persistent::Base::load( inputVolume, &err );
   AssertISV( err == Persistent::Base::Ok, avar( "Failed to load file '%s'.\n", argv[4] ) );
      
   // determine the light options types.. there is the chance for
   // 3 different options coming in here... ITRLightingOptions
   // is the current option format and is versioned
   ITRLightingOptions * itrLightingOptions = dynamic_cast<ITRLightingOptions*>(obj);

   if( !itrLightingOptions )
   {
      // create an options class
      itrLightingOptions = new ITRLightingOptions;
      
      ITRLightOptions * itrLightOptions = dynamic_cast<ITRLightOptions*>(obj);
      AssertISV( itrLightOptions, avar("File: %s does not containt an ITRLightingOptions.\n", argv[4]));

      // fill in the data...
      itrLightingOptions->m_lightingType = (ITRLightingOptions::LightingType)itrLightOptions->m_lightingType;
      itrLightingOptions->m_geometryScale = itrLightOptions->m_geometryScale;
      itrLightingOptions->m_lightScale = itrLightOptions->m_lightScale;
      itrLightingOptions->m_useNormals = itrLightOptions->m_useNormals;
      itrLightingOptions->m_emissionQuantumNumber = itrLightOptions->m_emissionQuantumNumber;

      // grab the ambient intensity
      ITRLightOptions2 * itrLightOptions2 = dynamic_cast<ITRLightOptions2*>(obj);
      if( itrLightOptions2 )
         itrLightingOptions->m_ambientIntensity = itrLightOptions2->m_ambientIntensity;

      itrLightingOptions->m_applyAmbientOutside = true;
   }
   
   printf( "   Ambient Intensity( %f, %f, %f )\n", 
      itrLightingOptions->m_ambientIntensity.red,
      itrLightingOptions->m_ambientIntensity.green,
      itrLightingOptions->m_ambientIntensity.blue );

   ITRBasicLighting::m_ambientIntensity = itrLightingOptions->m_ambientIntensity;
   ITRBasicLighting::g_applyAmbientOutside = itrLightingOptions->m_applyAmbientOutside;

   inputVolume.close();

   // Extract the .ilt file we want...
   //
   String iltFileName = argv[3];
   extractFile(inputVolume, iltFileName);

   // Load up the geometry
   //
   ITRGeometry* pGeometry =
      dynamic_cast<ITRGeometry*>(Persistent::Base::fileLoad(argv[2], &err));
   AssertISV(pGeometry != NULL, avar("Could not load geometry: %s", argv[2]));

   ITRBasicLighting::lightScale      = itrLightingOptions->m_lightScale;
   ITRBasicLighting::g_quantumNumber = itrLightingOptions->m_emissionQuantumNumber;
   ITRBasicLighting::g_useNormal     = itrLightingOptions->m_useNormals;

   // Start the timer...
   //
   DWORD timerStart = GetTickCount();

   // Do the lighting
   //
   ITRLighting lighting;
   ITRBasicLighting::LightList        lightList;
   ITRBasicLighting::MaterialPropList propList;

   if (itrLightingOptions->m_lightingType == ITRLightingOptions::Normal) {
      // Load the lights
      //
      Persistent::Base::Error err;
      Persistent::Base* pBase = Persistent::Base::fileLoad(iltFileName.c_str(), &err);
      AssertISV(pBase != NULL, avar("Unable to create persist object from file: %s",
                                    iltFileName.c_str()));
      PersLightList* pLightList = dynamic_cast<PersLightList*>(pBase);
      AssertFatal(pLightList != NULL, avar("Error, no light list in file: %s",
                                           iltFileName.c_str()));

      initLights(*pLightList, lightList, itrLightingOptions->m_geometryScale);
      printf("   Number of Lights:   %d\n", lightList.size());
      delete pLightList;
   } else {
      printf("   No light file: using default lighting\n");
   }

   if (itrLightingOptions->m_useMaterialProperties == true) {
      // Load the materialList;
      String materialListFileName = argv[5];
      TS::MaterialList* materialList = 
         dynamic_cast<TS::MaterialList *>(Persistent::Base::fileLoad(materialListFileName.c_str(), &err));
      AssertFatal(materialList != NULL,
                  avar("Error, could not load materialList: %s", argv[5]));

      // set up the matPropList
      propList.setSize(materialList->getMaterialsCount());
      for (int i = 0; i < propList.size(); i++) {
         propList[i].modified  = false;
         propList[i].intensity = 0.0f;
         propList[i].phongExp  = 1.0f;
      }

      String matPropFileName;
      reextendFileName(iltFileName, String(".imp"), matPropFileName);

      FILE* fp = fopen(matPropFileName.c_str(), "r");
      AssertISV(fp != NULL, "Error: could not open material property file");
      loadMaterialProperties(fp, &propList);
      
      delete materialList;
   }
   
   // Find out if we are filtering or testing - also grab ambient flag
   //
   ITRBasicLighting::filterScale = 0.0f;
   for (int i = 0; i < argc; i++) {
      if (argv[i][0] == '-' )
      {
         if( argv[i][1] == 'f' )
            ITRBasicLighting::filterScale = 0.5f;
      }
   }
   printf("   Filter Scale: %f\n", ITRBasicLighting::filterScale);
   
//   if (itrLightingOptions->m_lightingType == ITRLightingOptions::TestPattern)
//      ITRBasicLighting::g_testPattern = true;

   // Actually run it through the lighter...
   //
   ITRBasicLighting::light(*pGeometry, lightList, propList, &lighting);

   String outputDilName;
   reextendFileName(iltFileName, String(".dil"), outputDilName);
   lighting.fileStore(outputDilName.c_str());

   Int32 timerTotal = GetTickCount() - timerStart;
   printf("Elapsed Time: %.2f sec.\n", float(timerTotal) / 1000.0f);

   // Clean up our temps and tempfiles
   delete pGeometry;
   removeFile(iltFileName);
   delete itrLightingOptions;
   
   return 0;
}


void
initLights(PersLightList&               in_rLightList,
           ITRBasicLighting::LightList& out_rLightList,
           const float                  in_scaleFactor)
{
   for (int i = 0; i < in_rLightList.size(); i++) {
      PersLight&               rPersLight = *(in_rLightList[i]);

      ITRBasicLighting::Light* pZedLight = new ITRBasicLighting::Light;
      out_rLightList.push_back(pZedLight);
      ITRBasicLighting::Light& rZedLight  = *(out_rLightList.last());

      // First, copy the things that don't change from state to state,
      //  flags, duration, name, and assign a light id.
      //
      rZedLight.id                = rPersLight.id;
      rZedLight.name              = rPersLight.name;
      rZedLight.animationDuration = 0.0f;
      rZedLight.flags             = rPersLight.flags;

      // states
      rZedLight.state.setSize(rPersLight.state.size());
      for (int j = 0; j < rPersLight.state.size(); j++) {
         rZedLight.state[j] = new ITRBasicLighting::Light::State;

         // Emitter independant data...
         //
         rZedLight.state[j]->stateDuration = rPersLight.state[j]->stateDuration;
         rZedLight.animationDuration += rZedLight.state[j]->stateDuration;
         rZedLight.state[j]->color.red     = rPersLight.state[j]->color.red;
         rZedLight.state[j]->color.green   = rPersLight.state[j]->color.green;
         rZedLight.state[j]->color.blue    = rPersLight.state[j]->color.blue;

         rZedLight.state[j]->m_emitterList.setSize(rPersLight.state[j]->m_emitterList.size());
         for (int k = 0; k < rPersLight.state[j]->m_emitterList.size(); k++) {
            ITRBasicLighting::Light::State::Emitter& rZedEmitter  = rZedLight.state[j]->m_emitterList[k];
            PersLight::PersState::Emitter&           rPersEmitter = rPersLight.state[j]->m_emitterList[k];

            rZedEmitter.lightType  = ITRBasicLighting::Light::State::LightType(rPersEmitter.lightType);
            rZedEmitter.pos        = rPersEmitter.pos * in_scaleFactor;
            float temp = rZedEmitter.pos.y;
            rZedEmitter.pos.y = rZedEmitter.pos.z;
            rZedEmitter.pos.z = temp;
            rZedEmitter.spotVector = rPersEmitter.spotVector;
            if (rZedEmitter.lightType == ITRBasicLighting::Light::State::SpotLight)
               rZedEmitter.spotVector.normalize();
            rZedEmitter.spotTheta  = rPersEmitter.spotTheta;
            rZedEmitter.spotPhi    = rPersEmitter.spotPhi;
            rZedEmitter.falloff    = ITRBasicLighting::Light::State::Falloff(rPersEmitter.falloff);
            rZedEmitter.d1         = rPersEmitter.d1;
            rZedEmitter.d2         = rPersEmitter.d2;
            rZedEmitter.d3         = rPersEmitter.d3;

            if (rZedEmitter.falloff == ITRBasicLighting::Light::State::Distance) {
               rZedEmitter.d2 *= 1.0f / in_scaleFactor;
               rZedEmitter.d3 *= 1.0f / in_scaleFactor;
               rZedEmitter.radius = getRadius(rZedEmitter.d3,
                                              rZedEmitter.d2,
                                              rZedEmitter.d1,
                                              rPersLight.id);
            } else if (rZedEmitter.falloff == ITRBasicLighting::Light::State::Linear) {
               rZedEmitter.d1 *= in_scaleFactor;
               rZedEmitter.d2 *= in_scaleFactor;
               rZedEmitter.d3  = 0.0f;
               rZedEmitter.radius = rZedEmitter.d2;
            } else {
               AssertFatal(0, "invalid falloff type...");
            }
         }
      }
   }
}
//w64elevlink2.zvl w64elevlink2-00.dig w64elevlink2-000.ilt w64elevlink2-000.ilo

//----------------------------------------------------------------------------
// Loads any modified materials properties...
//
void loadMaterialProperties(FILE* fp, ITRBasicLighting::MaterialPropList* matPropList)
{
   char buff[256];
   int  matIndex;

   if (fscanf(fp,"%s",buff) == EOF)
      return;
   
   while (!stricmp(buff,"Material")) {
      if (!stricmp(buff, "Material")) {
         fscanf(fp, "%d\n", &matIndex);
         ITRBasicLighting::MaterialProp &prop = (*matPropList)[matIndex];
         bool breakFlag = false;
         while (!stricmp(buff,"Material") && breakFlag == false) {
            while (true) {
               if (fscanf(fp,"%s",buff) == EOF)
                  return;
               else if (!stricmp(buff,"flatColor")) {
                  fscanf(fp,"%f %f %f\n",&prop.color.red,
                                         &prop.color.green,
                                         &prop.color.blue);
                  prop.modified = true;
                  prop.emissionType = ITRBasicLighting::MaterialProp::flatColor;
               } else if (!stricmp(buff,"flatEmission")) {
                  fscanf(fp,"%f %f %f %f\n",&prop.color.red,
                                            &prop.color.green,
                                            &prop.color.blue,
                                            &prop.intensity);
                  prop.modified = true;
                  prop.emissionType = ITRBasicLighting::MaterialProp::flatEmission;
               } else {
                  breakFlag = true;
                  break;
               }
            }
         }
      }
      else {
         char tmp[10];
         fscanf(fp,"\n",tmp);
         break;
      }
   }
}


void
loadBinaryLights(FileRStream&                 io_frs,
                 ITRBasicLighting::LightList& lightList,
                 float                        scaleFactor)
{
   Persistent::Base::Error err;
   PersLightList* pPersLightList =
      dynamic_cast<PersLightList*>(Persistent::Base::load(io_frs,&err));
   AssertFatal(pPersLightList != NULL, "Could not load Lights...");

   // It's easier to deal with the vector operations when the perslightlist
   //  is a reference...
   //
   PersLightList& rPersLightList = *pPersLightList;      

   // Set the lighter's listSize to match
   //
   lightList.setSize(rPersLightList.size());

   for (int i = 0; i < rPersLightList.size(); i++) {
      PersLight&               rPersLight = *rPersLightList[i];
      ITRBasicLighting::Light& rLight     = *lightList[i];
      
      // Let the light's operator= handle the conversion...
      rLight = rPersLight;
      rLight.scale(scaleFactor);
   }
}
