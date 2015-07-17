//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GD3DTXMNG_H_
#define _GD3DTXMNG_H_

//Includes
#include "gD3DTxTrans.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//------------------------------------------------------------------------------
//-------------------------------------- Different Texture Translators derived
//--------------------------------------  from TextureMunger & LightMapMunger
//
namespace Direct3D {

class MungerPalettized : public TextureMunger {
  private:
   static D3DTexRenderState renderState;
   static DDSURFACEDESC     masterDDSDCopy;

   bool beingUsedForTransparency;
  public:
   static bool canTranslate(DDSURFACEDESC* pDDSD);

   void getSurfDesc(DDSURFACEDESC* out_pDDSD);

   void translateBitmap(const GFXBitmap*  in_pTexture,
                                const Int32       in_mipLevel,
                                void*             out_pDDTex,
                                const Int32       in_dstStride);

   void setPalette(const GFXPalette *in_pPal);
   bool attachPalette() const;
   bool setColorKey() const;
   bool isSurfaceGFXFormat() const;

   virtual const D3DTexRenderState* getRenderState() const;
   int getTextureType();

   MungerPalettized(const bool in_transparentMunger);
   ~MungerPalettized();
};


class Munger565Normal : public TextureMunger {
  private:
   static D3DTexRenderState renderState;
   static DDSURFACEDESC     masterDDSDCopy;

   UInt16* translationTable;
   bool    beingUsedForTransparency;
  public:
   static bool canTranslate(DDSURFACEDESC* pDDSD);

   void getSurfDesc(DDSURFACEDESC* out_pDDSD);

   UInt16 mungeEntry(UInt16 r, UInt16 g, UInt16 b);
   void   translateBitmap(const GFXBitmap*  in_pTexture,
                                  const Int32       in_mipLevel,
                                  void*             out_pDDTex,
                                  const Int32       in_dstStride);

   void setPalette(const GFXPalette *in_pPal);
   bool attachPalette() const;
   bool setColorKey() const;

   virtual const D3DTexRenderState* getRenderState() const;
   int getTextureType();

   Munger565Normal(const bool in_transparentMunger);
   ~Munger565Normal();
};


class Munger555Normal : public TextureMunger {
  private:
   static D3DTexRenderState renderState;
   static DDSURFACEDESC     masterDDSDCopy;

   UInt16* translationTable;
   bool    beingUsedForTransparency;
  public:
   static bool canTranslate(DDSURFACEDESC* pDDSD);

   void getSurfDesc(DDSURFACEDESC* out_pDDSD);

   UInt16 mungeEntry(UInt16 r, UInt16 g, UInt16 b);
   void   translateBitmap(const GFXBitmap*  in_pTexture,
                                  const Int32       in_mipLevel,
                                  void*             out_pDDTex,
                                  const Int32       in_dstStride);

   void setPalette(const GFXPalette *in_pPal);
   bool attachPalette() const;
   bool setColorKey() const;

   virtual const D3DTexRenderState* getRenderState() const;
   int getTextureType();

   Munger555Normal(const bool in_transparentMunger);
   ~Munger555Normal();
};


class Munger4444Alpha : public TextureMunger {
  private:
   static D3DTexRenderState renderState;
   static DDSURFACEDESC     masterDDSDCopy;

   UInt16* translationTable;
   bool    driverAlphaInverted;
   bool    beingUsedForTransparency;
  public:
   static bool canTranslate(DDSURFACEDESC* pDDSD);

   void getSurfDesc(DDSURFACEDESC* out_pDDSD);

   UInt16 mungeEntry(UInt16 r, UInt16 g, UInt16 b, UInt16 a);
   void   translateBitmap(const GFXBitmap*  in_pTexture,
                                  const Int32       in_mipLevel,
                                  void*             out_pDDTex,
                                  const Int32       in_dstStride);

   void setPalette(const GFXPalette *in_pPal);
   bool attachPalette() const;
   bool setColorKey() const;

   virtual const D3DTexRenderState* getRenderState() const;
   int getTextureType();

   Munger4444Alpha(const bool in_driverAlphaInverted,
                              const bool in_transparentMunger);
   ~Munger4444Alpha();
};


class Munger1555Alpha : public TextureMunger {
  private:
   static D3DTexRenderState renderState;
   static DDSURFACEDESC     masterDDSDCopy;

   UInt16* translationTable;
   bool    driverAlphaInverted;
   bool    beingUsedForTransparency;
  public:
   static bool canTranslate(DDSURFACEDESC* pDDSD);

   void getSurfDesc(DDSURFACEDESC* out_pDDSD);

   UInt16 mungeEntry(UInt16 r, UInt16 g, UInt16 b, UInt16 a);
   void   translateBitmap(const GFXBitmap*  in_pTexture,
                                  const Int32       in_mipLevel,
                                  void*             out_pDDTex,
                                  const Int32       in_dstStride);

   void setPalette(const GFXPalette *in_pPal);
   bool attachPalette() const;
   bool setColorKey() const;

   virtual const D3DTexRenderState* getRenderState() const;
   int getTextureType();

   Munger1555Alpha(const bool in_driverAlphaInverted,
                              const bool in_transparentMunger);
   ~Munger1555Alpha();
};


class Munger565LM : public LightMapMunger {
  private:
   static D3DTexRenderState renderState;
   static DDSURFACEDESC     masterDDSDCopy;

   bool lightMapInverted;
  public:
   static bool canTranslate(DDSURFACEDESC* pDDSD);
   
   void getSurfDesc(DDSURFACEDESC* out_pDDSD);

   UInt16 mungeEntry(UInt16 in_lmValue);
   void   translateLightmap(const GFXLightMap* in_pLightmap,
                                    void*              out_pDDLightmap,
                                    const Int32        in_dstStride);

   virtual const D3DTexRenderState* getRenderState() const;
   int getTextureType();

   Munger565LM(const bool  in_lightMapInverted,
                          const DWORD in_lightMapMode);
   ~Munger565LM();
};


class Munger555LM : public LightMapMunger {
  private:
   static D3DTexRenderState renderState;
   static DDSURFACEDESC     masterDDSDCopy;

   bool lightMapInverted;
  public:
   static bool canTranslate(DDSURFACEDESC* pDDSD);
   
   void getSurfDesc(DDSURFACEDESC* out_pDDSD);

   UInt16 mungeEntry(UInt16 in_lmValue);
   void   translateLightmap(const GFXLightMap* in_pLightmap,
                                    void*              out_pDDLightmap,
                                    const Int32        in_dstStride);

   virtual const D3DTexRenderState* getRenderState() const;
   int getTextureType();

   Munger555LM(const bool  in_lightMapInverted,
                          const DWORD in_lightMapMode);
   ~Munger555LM();
};

class Munger444LM : public LightMapMunger {
  private:
   static D3DTexRenderState renderState;
   static DDSURFACEDESC     masterDDSDCopy;

   bool lightMapInverted;
  public:
   static bool canTranslate(DDSURFACEDESC* pDDSD);
   
   void getSurfDesc(DDSURFACEDESC* out_pDDSD);

   UInt16 mungeEntry(UInt16 in_lmValue);
   void   translateLightmap(const GFXLightMap* in_pLightmap,
                                    void*              out_pDDLightmap,
                                    const Int32        in_dstStride);

   virtual const D3DTexRenderState* getRenderState() const;
   int getTextureType();

   Munger444LM(const bool  in_lightMapInverted,
                          const DWORD in_lightMapMode);
   ~Munger444LM();
};

class Munger888LM : public LightMapMunger {
  private:
   static D3DTexRenderState renderState;
   static DDSURFACEDESC     masterDDSDCopy;

   bool lightMapInverted;
  public:
   static bool canTranslate(DDSURFACEDESC* pDDSD);
   
   void getSurfDesc(DDSURFACEDESC* out_pDDSD);

   UInt32 mungeEntry(UInt16 in_lmValue);
   void   translateLightmap(const GFXLightMap* in_pLightmap,
                                    void*              out_pDDLightmap,
                                    const Int32        in_dstStride);

   virtual const D3DTexRenderState* getRenderState() const;
   int getTextureType();

   Munger888LM(const bool  in_lightMapInverted,
                          const DWORD in_lightMapMode);
   ~Munger888LM();
};

}; // namespace Direct3D

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GD3DTXMNG_H_
