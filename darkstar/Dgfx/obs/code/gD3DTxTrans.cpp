//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "gD3DTxTrans.h"
#include "gD3DTxMng.h"


namespace Direct3D {

//-------------------------------------- Static Class members
//
bool                  TxTranslatorClassFact::enumerationInProgress = false;
Vector<DDSURFACEDESC> TxTranslatorClassFact::texFormatVector;


void 
TxTranslatorClassFact::beginTextureEnum()
{
   AssertFatal(enumerationInProgress == false,
               "Error starting format enumeration: one was already in progress");

   texFormatVector.clear();
   enumerationInProgress = true;
}


void 
TxTranslatorClassFact::endTextureEnum()
{
   AssertFatal(enumerationInProgress == true,
               "Error ending format enumeration: none in progress");

   enumerationInProgress = false;
}


void 
TxTranslatorClassFact::addFormat(const LPDDSURFACEDESC in_pDDSD)
{
   AssertFatal(enumerationInProgress == true,
               "Error adding texture format: no enumeration in progress");
   AssertFatal((in_pDDSD->dwFlags & DDSD_PIXELFORMAT) != 0,
               "Error: Surface Description's pixel format member is not valid");

   // Add the format to the current list...
   //
   texFormatVector.push_back(*in_pDDSD);
}


TextureTranslator* 
TxTranslatorClassFact::getBestTranslator(const bool  in_driverAlphaInverted,
                                         const bool  /*in_driverAlphaCmp*/,
                                         const bool  in_colorKeySupported,
                                         const bool  in_colorLightMaps,
                                         const bool  in_lightMapInverted,
                                         const DWORD in_lightMapMode)
{
   TextureMunger*  pNormal = getNormalMunger(false);
   TextureMunger*  pAlpha  = getAlphaMunger(in_driverAlphaInverted);
   TextureMunger*  pTrans  = getTransparentMunger(in_driverAlphaInverted,
                                                  in_colorKeySupported);
   LightMapMunger* pLMap   = getLightMapMunger(in_colorLightMaps,
                                               in_lightMapInverted,
                                               in_lightMapMode);

   if (pNormal == NULL || pAlpha == NULL || pTrans == NULL || pLMap == NULL) {
      // unable to get REQUIRED renderers
      //
      if (pNormal != NULL) delete pNormal;
      if (pAlpha  != NULL) delete pAlpha;
      if (pTrans  != NULL) delete pTrans;
      if (pLMap   != NULL) delete pLMap;

      return NULL;
   }

   TextureTranslator* pTranslator = new TextureTranslator;
   pTranslator->pNormalMunger = pNormal;
   pTranslator->pAlphaMunger  = pAlpha;
   pTranslator->pTransMunger  = pTrans;
   pTranslator->pLightMunger  = pLMap;

   return pTranslator;
}


TextureMunger* 
TxTranslatorClassFact::getNormalMunger(const bool in_transparentMunger)
{
   // Current Prefered order of Normal mungers:
   //  - MungerPalettized
   //  - Munger565Normal
   //
   int i;
   for (i = 0; i < texFormatVector.size(); i++) {
      if (MungerPalettized::canTranslate(&(texFormatVector[i])) == true) {
         return new MungerPalettized(in_transparentMunger);
      }
   }
   for (i = 0; i < texFormatVector.size(); i++) {
      if (Munger565Normal::canTranslate(&(texFormatVector[i])) == true) {
         return new Munger565Normal(in_transparentMunger);
      }
   }
   for (i = 0; i < texFormatVector.size(); i++) {
      if (Munger555Normal::canTranslate(&(texFormatVector[i])) == true) {
         return new Munger555Normal(in_transparentMunger);
      }
   }

   return NULL;
}


TextureMunger* 
TxTranslatorClassFact::getAlphaMunger(const bool in_driverAlphaInverted)
{
   // Current Prefered order of Alpha mungers:
   //  - Munger4444Alpha
   //
   int i;
   for (i = 0; i < texFormatVector.size(); i++) {
      if (Munger4444Alpha::canTranslate(&(texFormatVector[i])) == true) {
         return new Munger4444Alpha(in_driverAlphaInverted, false);
      }
   }
   for (i = 0; i < texFormatVector.size(); i++) {
      if (Munger1555Alpha::canTranslate(&(texFormatVector[i])) == true) {
         AssertWarn(0, "Using lousy alpha format!");
         return new Munger1555Alpha(in_driverAlphaInverted, false);
      }
   }

   return NULL;
}


TextureMunger* 
TxTranslatorClassFact::getTransparentMunger(const bool in_driverAlphaInverted,
                                            const bool in_colorKeySupported)
{
   // If colorkey is not supported, current preferred mungers for transparent
   //  mungers:
   //   - Munger4444Alpha
   // If colorkey _is_ supported, get a Normal munger, and use that...
   //
   int i;
   if (in_colorKeySupported == true) {
      return getNormalMunger(true);
   } else {
      for (i = 0; i < texFormatVector.size(); i++) {
         if (Munger1555Alpha::canTranslate(&(texFormatVector[i])) == true) {
            return new Munger1555Alpha(in_driverAlphaInverted, true);
         }
      }
      for (i = 0; i < texFormatVector.size(); i++) {
         if (Munger4444Alpha::canTranslate(&(texFormatVector[i])) == true) {
            return new Munger4444Alpha(in_driverAlphaInverted, true);
         }
      }
   }

   return NULL;
}


LightMapMunger* 
TxTranslatorClassFact::getLightMapMunger(const bool  in_colorLightMaps,
                                         const bool  in_lightMapInverted,
                                         const DWORD in_lightMapMode)
{
   // Current Prefered order of Lightmap mungers:
   //   - Munger565LM - If colored lightmaps are being used...
   //   - Munger444LM
   //
   int i;
   if (in_colorLightMaps == true) {
      for (i = 0; i < texFormatVector.size(); i++) {
         if (Munger565LM::canTranslate(&(texFormatVector[i])) == true) {
            return new Munger565LM(in_lightMapInverted,
                                   in_lightMapMode);
         }
      }
      for (i = 0; i < texFormatVector.size(); i++) {
         if (Munger555LM::canTranslate(&(texFormatVector[i])) == true) {
            return new Munger555LM(in_lightMapInverted,
                                   in_lightMapMode);
         }
      }
   }
   for (i = 0; i < texFormatVector.size(); i++) {
      if (Munger444LM::canTranslate(&(texFormatVector[i])) == true) {
         return new Munger444LM(in_lightMapInverted,
                                in_lightMapMode);
      }
   }
   for (i = 0; i < texFormatVector.size(); i++) {
      if (Munger888LM::canTranslate(&(texFormatVector[i])) == true) {
         return new Munger888LM(in_lightMapInverted,
                                in_lightMapMode);
      }
   }

   return NULL;
}



TextureTranslator::~TextureTranslator()
{
   if (pNormalMunger != NULL) delete pNormalMunger;
   if (pAlphaMunger  != NULL) delete pAlphaMunger;
   if (pTransMunger  != NULL) delete pTransMunger;
   if (pLightMunger  != NULL) delete pLightMunger;

   pNormalMunger =
    pAlphaMunger =
    pTransMunger = NULL;
   pLightMunger  = NULL;
}


//------------------------------------------------------------------------------
// NAME 
//    void setPalette(const GFXPalette* in_pPalette)
//    
// DESCRIPTION 
//    sets the palette on the textureMungers that require it...
//
// NOTES 
//    
//------------------------------------------------------------------------------
void 
TextureTranslator::setPalette(const GFXPalette* in_pPalette)
{
   if (pNormalMunger != NULL) pNormalMunger->setPalette(in_pPalette);
   if (pAlphaMunger  != NULL) pAlphaMunger->setPalette(in_pPalette);
   if (pTransMunger  != NULL) pTransMunger->setPalette(in_pPalette);
}


//------------------------------------------------------------------------------
// NAME 
//    bool 
//    TextureMunger::isSurfaceGFXFormat()
//    
// DESCRIPTION 
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool 
TextureMunger::isSurfaceGFXFormat() const
{
   return false;
}


//------------------------------------------------------------------------------
// NAME 
//    LightMapMunger::setColorKey()
//    
// DESCRIPTION 
//    Always returns false for now, since lightmaps aren't transparent...
//    
// NOTES 
//    DMMNOTE: Could maybe use this to speed up lightmaps by not blitting full
//   brightness?
//------------------------------------------------------------------------------
bool 
LightMapMunger::setColorKey() const
{
   return false;
}

//------------------------------------------------------------------------------
// NAME 
//    bool 
//    TextureMunger::eagerDiscard()
//    bool 
//    LightMapMunger::eagerDiscard() const
//    
// DESCRIPTION 
//    Describes whether or not the texture cache should throw out the texture
//   at the first opportunity.  For textures we disable this behavior, but
//   since each poly has a unique lightmap, we want to throw them out as soon
//   as possible...
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool 
TextureMunger::eagerDiscard() const
{
   return false;
}

bool 
LightMapMunger::eagerDiscard() const
{
   return true;
}

bool 
LightMapMunger::attachPalette() const
{
   return false;
}

}; // namespace Direct3D