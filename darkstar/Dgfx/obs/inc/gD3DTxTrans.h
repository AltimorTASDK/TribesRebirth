//------------------------------------------------------------------------------
// Description Texture Intermediary classes and Intermediary Class Factory
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GD3DTXTRANS_H_
#define _GD3DTXTRANS_H_

//Includes

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

#include <base.h>
#include <g_pal.h>
#include <g_bitmap.h>
#include <p_txcach.h>

#include "ddraw.h"
#include "d3d.h"



namespace Direct3D {

typedef struct _d3dtexrenderstate {
   D3DBLEND        srcBlend;
   D3DBLEND        destBlend;

   bool enableAlphaBlend;
} D3DTexRenderState;


//------------------------------------------------------------------------------
//-------------------------------------- Munger
//
class Munger {
  protected:
   enum { GD3D_NORMAL = 0, GD3D_TRANSPARENT = 1, GD3D_ALPHA = 2, GD3D_LIGHTMAP = 3 };

  public:
   static  bool canTranslate(DDSURFACEDESC* pDDSD) { return false; }

   virtual void getSurfDesc(DDSURFACEDESC* out_pDDSD) = 0;

   virtual const D3DTexRenderState* getRenderState() const = 0;
   virtual bool setColorKey() const = 0;

   virtual bool eagerDiscard() const = 0;

   virtual bool attachPalette() const = 0;
   virtual int getTextureType() = 0;

   Munger() { }
   virtual ~Munger() { }
};


//------------------------------------------------------------------------------
//-------------------------------------- TextureMunger
//
class TextureMunger : public Munger {
  private:

  public:
   static  bool canTranslate(DDSURFACEDESC* pDDSD) { return false; }

   virtual void translateBitmap(const GFXBitmap*  in_pTexture,
                                        const Int32       in_mipLevel,
                                        void*             out_pDDTex,
                                        const Int32       in_dstStride) = 0;
   virtual void setPalette(const GFXPalette *in_pPal) = 0;

   virtual const D3DTexRenderState* getRenderState() const = 0;

   virtual bool isSurfaceGFXFormat() const;

   // For textures this is always false...
   //
   bool eagerDiscard() const;

   TextureMunger(const bool in_transparentMunger) { }
   virtual ~TextureMunger() { }
};

//-------------------------------------- LightMapMunger
//
class LightMapMunger : public Munger {
  private:

  public:
   static bool canTranslate(DDSURFACEDESC* pDDSD) { return false; }

   virtual void translateLightmap(const GFXLightMap* in_pLightmap,
                                          void*              out_pDDLightmap,
                                          const Int32        in_dstStride) = 0;

   virtual const D3DTexRenderState* getRenderState() const = 0;
   virtual bool setColorKey() const;

   bool attachPalette() const;

   // For lightmaps this is always true...
   //
   bool eagerDiscard() const;

   LightMapMunger() { }
   virtual ~LightMapMunger() { }
};


//------------------------------------------------------------------------------
//-------------------------------------- TextureTranslator
//
class TextureTranslator {
  private:
   const GFXPalette* pPalette;

  public:
   TextureMunger*  pNormalMunger;
   TextureMunger*  pAlphaMunger;
   TextureMunger*  pTransMunger;
   LightMapMunger* pLightMunger;

   TextureTranslator() : pPalette(NULL), pNormalMunger(NULL),
                                    pAlphaMunger(NULL), pTransMunger(NULL) { }
   ~TextureTranslator();

   void setPalette(const GFXPalette* in_pPalette);
};


//------------------------------------------------------------------------------
//-------------------------------------- TxTranslatorClassFact
//
class TxTranslatorClassFact {
  private:
   static bool                  enumerationInProgress;
   static Vector<DDSURFACEDESC> texFormatVector;

  public:
   static void beginTextureEnum();
   static void endTextureEnum();
   static void addFormat(const LPDDSURFACEDESC in_pDDSD);

   static TextureMunger*  getNormalMunger(const bool in_transparentMunger);
   static TextureMunger*  getAlphaMunger(const bool in_driverAlphaInverted);
   static TextureMunger*  getTransparentMunger(const bool in_driverAlphaInverted,
                                                       const bool in_colorKeySupported);
   static LightMapMunger* getLightMapMunger(const bool  in_colorLightMaps,
                                                    const bool  in_lightMapInverted,
                                                    const DWORD in_lightMapMode);

   static TextureTranslator* getBestTranslator(const bool  in_driverAlphaInverted,
                                                       const bool  in_driverAlphaCmp,
                                                       const bool  in_colorKeySupported,
                                                       const bool  in_colorLightMaps,
                                                       const bool  in_lightMapInverted,
                                                       const DWORD in_lightMapMode);

   TxTranslatorClassFact()  { }
   ~TxTranslatorClassFact() { }
};

}; // namespace Direct3D

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GD3DTXTRANS_H_
