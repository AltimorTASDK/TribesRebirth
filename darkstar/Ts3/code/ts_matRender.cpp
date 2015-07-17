//---------------------------------------------------------------------------
//	ts_matRender.cpp
//---------------------------------------------------------------------------

#include <ts_material.h>
#include <ts_pointArray.h>
#include <ts_shapeInst.h>
#include <ts_celAnimMesh.h>
#include <g_surfac.h>
#include <g_bitmap.h>

//--------------------------------------------------------
// macros defined to statics hanging off material
//--------------------------------------------------------
#define first     (TSMaterial::firstVert)
#define fVerts    (TSMaterial::pVerts)
#define scale     (TSMaterial::scale)
#define origin    (TSMaterial::origin)
#define pFace     ((const TS::CelAnimMesh::Face*) face)
#define srf       (TSMaterial::srf)
#define pa        (TSMaterial::pa)
#define shapeInst (TSMaterial::si)

//--------------------------------------------------------
// lighting macros used by various renderers
//--------------------------------------------------------

#define SmoothShade() \
   int idx = pFace->fVIP[0].fVertexIndex;               \
   pa->lightPoint( first + idx, fVerts[idx],            \
                   scale, origin );                     \
   idx = pFace->fVIP[1].fVertexIndex;                   \
   pa->lightPoint( first + idx, fVerts[idx],            \
                   scale, origin );                     \
   idx = pFace->fVIP[2].fVertexIndex;                   \
   pa->lightPoint( first + idx, fVerts[idx],            \
                   scale, origin );                     \
   srf->setShadeSource( GFX_SHADE_VERTEX )


#define FlatShade() \
   int idx = pFace->fVIP[0].fVertexIndex;               \
   pa->lightPoint( first + idx, fVerts[ idx ],          \
                  scale, origin );                  \
   TransformedVertex & tv =                             \
      pa->getTransformedVertex( first + idx );          \
   ColorF &intens = tv.fColor.color;                    \
   float sh = intens.red * 0.299f +                     \
              intens.blue * 0.587f +                    \
              intens.green * 0.114f;                    \
   if (sh > 1.0f)                                       \
      sh = 1.0f;                                        \
   srf->setConstantShade( sh );                         \
   srf->setShadeSource( GFX_SHADE_CONSTANT )

#define NoShade() \
   srf->setShadeSource(GFX_SHADE_CONSTANT);             \
   srf->setConstantShade(1.0)

#define NoShadeTextureTranslucent() \
   srf->setShadeSource(GFX_SHADE_NONE)
         
//--------------------------------------------------------

namespace TS
{

//--------------------------------------------------------
// non-translucent, non-transparent
//--------------------------------------------------------

// ShadingSmooth | MatTexture:
void matrenderST(const Material * mat, const char * face)
{
   pa->useTextures( true );
   srf->setFillMode(GFX_FILL_TEXTURE);
   srf->setTexturePerspective( false );
   srf->setTransparency(FALSE);

   // make sure verts are lit:
   SmoothShade();

   pa->useIntensities( TRUE );

   if (shapeInst->isPerspective())
      srf->setTexturePerspective(true);

   if (shapeInst->getAlwaysAlpha())
   {
      srf->setAlphaSource(GFX_ALPHA_CONSTANT);
      srf->setConstantAlpha(shapeInst->getAlwaysAlphaValue());
   }
   else
      srf->setAlphaSource(GFX_ALPHA_NONE);

   srf->setTextureMap( mat->getTextureMap() );

   pa->drawTriangle( pFace->fVIP, first);
}

//--------------------------------------------------------------------------------
// ShadingFlat   | MatTexture:
//--------------------------------------------------------------------------------

void matrenderFT(const Material * mat, const char * face)
{
   pa->useTextures( true );
   srf->setFillMode(GFX_FILL_TEXTURE);
   srf->setTexturePerspective( false );

   srf->setTransparency(FALSE);

   FlatShade();
         
   pa->useIntensities( false );
   if (shapeInst->isPerspective())
      srf->setTexturePerspective(true);

   if (shapeInst->getAlwaysAlpha())
   {
      srf->setAlphaSource(GFX_ALPHA_CONSTANT);
      srf->setConstantAlpha(shapeInst->getAlwaysAlphaValue());
   }
   else
      srf->setAlphaSource(GFX_ALPHA_NONE);

   srf->setTextureMap( mat->getTextureMap() );

   pa->drawTriangle( pFace->fVIP, first);
}


//--------------------------------------------------------------------------------
// ShadingNone   | MatRGB    :
//--------------------------------------------------------------------------------
void matrenderNR(const Material * mat, const char * face)
{
   pa->useTextures( false );
   srf->setFillMode(GFX_FILL_CONSTANT);
   // fill color set below, after shade is set

   srf->setTransparency(FALSE);

   NoShade();

   srf->setFillColor( mat->fParams.fIndex );
   pa->useIntensities( false );

   if (shapeInst->getAlwaysAlpha())
   {
      srf->setAlphaSource(GFX_ALPHA_CONSTANT);
      srf->setConstantAlpha(shapeInst->getAlwaysAlphaValue());
   }
   else
      srf->setAlphaSource(GFX_ALPHA_NONE);

   pa->drawTriangle( pFace->fVIP, first);
}

//--------------------------------------------------------------------------------
// ShadingFlat   | MatRGB    :
//--------------------------------------------------------------------------------
void matrenderFR(const Material * mat, const char * face)
{
   pa->useTextures( false );
   srf->setFillMode(GFX_FILL_CONSTANT);
   // fill color set below, after shade is set

   srf->setTransparency(FALSE);

   FlatShade();

   // flat fill color must be set after shade
   srf->setFillColor( mat->fParams.fIndex );
         
   pa->useIntensities( false );

   if (shapeInst->getAlwaysAlpha())
   {
      srf->setAlphaSource(GFX_ALPHA_CONSTANT);
      srf->setConstantAlpha(shapeInst->getAlwaysAlphaValue());
   }
   else
      srf->setAlphaSource(GFX_ALPHA_NONE);

   pa->drawTriangle( pFace->fVIP, first);
}

//--------------------------------------------------------------------------------
// ShadingNone   | MatTexture:
//--------------------------------------------------------------------------------
void matrenderNT(const Material * mat, const char * face)
{
   pa->useTextures( true );
   srf->setFillMode(GFX_FILL_TEXTURE);
   srf->setTexturePerspective( false );
   srf->setTransparency(FALSE);

   NoShade();

   if (shapeInst->isPerspective())
      srf->setTexturePerspective(true);

   pa->useIntensities( false );

   if (shapeInst->getAlwaysAlpha())
   {
      srf->setAlphaSource(GFX_ALPHA_CONSTANT);
      srf->setConstantAlpha(shapeInst->getAlwaysAlphaValue());
   }
   else 
      srf->setAlphaSource(GFX_ALPHA_NONE);

   srf->setTextureMap( mat->getTextureMap() );

   pa->drawTriangle( pFace->fVIP, first);
}

//--------------------------------------------------------------------------------
// ShadingSmooth | MatRGB    :
//--------------------------------------------------------------------------------
void matrenderSR(const Material * mat, const char * face)
{
   pa->useTextures( false );
   srf->setFillMode(GFX_FILL_CONSTANT);
   // fill color set below, after shade is set

   srf->setTransparency(FALSE);

   SmoothShade();

   // flat fill color must be set after shade
   srf->setFillColor( mat->fParams.fIndex );

   pa->useIntensities( TRUE );

   if (shapeInst->getAlwaysAlpha())
   {
      srf->setAlphaSource(GFX_ALPHA_CONSTANT);
      srf->setConstantAlpha(shapeInst->getAlwaysAlphaValue());
   }
   else
      srf->setAlphaSource(GFX_ALPHA_NONE);

   pa->drawTriangle( pFace->fVIP, first);
}

//--------------------------------------------------------------------------------
// ShadingNone   | MatPalette:
//--------------------------------------------------------------------------------
void matrenderNP(const Material * mat, const char * face)
{
   pa->useTextures( false );
   srf->setFillMode(GFX_FILL_CONSTANT);
   // fill color set below, after shade is set

   srf->setTransparency(FALSE);

   NoShade();

   srf->setFillColor( mat->fParams.fIndex );

   pa->useIntensities( false );

   if (shapeInst->getAlwaysAlpha())
   {
      srf->setAlphaSource(GFX_ALPHA_CONSTANT);
      srf->setConstantAlpha(shapeInst->getAlwaysAlphaValue());
   }
   else
      srf->setAlphaSource(GFX_ALPHA_NONE);

   pa->drawTriangle( pFace->fVIP, first);
}

//--------------------------------------------------------------------------------
// ShadingFlat   | MatPalette:
//--------------------------------------------------------------------------------
void matrenderFP(const Material * mat, const char * face)
{
   pa->useTextures( false );
   srf->setFillMode(GFX_FILL_CONSTANT);
   // fill color set below, after shade is set

   srf->setTransparency(FALSE);

   FlatShade();

   // flat fill color must be set after shade
   srf->setFillColor( mat->fParams.fIndex );

   pa->useIntensities( false );

   if (shapeInst->getAlwaysAlpha())
   {
      srf->setAlphaSource(GFX_ALPHA_CONSTANT);
      srf->setConstantAlpha(shapeInst->getAlwaysAlphaValue());
   }
   else
      srf->setAlphaSource(GFX_ALPHA_NONE);

   pa->drawTriangle( pFace->fVIP, first);
}

//--------------------------------------------------------------------------------
// ShadingSmooth | MatPalette:
//--------------------------------------------------------------------------------
void matrenderSP(const Material * mat, const char * face)
{
   pa->useTextures( false );
   srf->setFillMode(GFX_FILL_CONSTANT);
   // fill color set below, after shade is set

   srf->setTransparency(FALSE);

   SmoothShade();

   // flat fill color must be set after shade
   srf->setFillColor( mat->fParams.fIndex );

   pa->useIntensities( TRUE );

   if (shapeInst->getAlwaysAlpha())
   {
      srf->setAlphaSource(GFX_ALPHA_CONSTANT);
      srf->setConstantAlpha(shapeInst->getAlwaysAlphaValue());
   }
   else
      srf->setAlphaSource(GFX_ALPHA_NONE);

   pa->drawTriangle( pFace->fVIP, first);
}


//--------------------------------------------------------
// translucent
//--------------------------------------------------------

//--------------------------------------------------------------------------------
// ShadingSmooth | MatTexture | TextureTranslucent:
//--------------------------------------------------------------------------------
void matrenderSTU(const Material * mat, const char * face)
{
   mat,face;
   
   // not implemented, use shading none
}

//--------------------------------------------------------------------------------
// ShadingNone   | MatTexture | TextureTranslucent:
//--------------------------------------------------------------------------------
void matrenderNTU(const Material * mat, const char * face)
{
   // Erk!  Stupid Software order dependencies!  There's definitely
   //  a better way to do this.  come back and do it later... DMM
   GFXHazeSource oldSource = srf->getHazeSource();
   float constHaze         = srf->getConstantHaze();

   pa->useTextures( true );
   srf->setFillMode(GFX_FILL_TEXTURE);
   srf->setHazeSource(GFX_HAZE_NONE);

   NoShadeTextureTranslucent();

   pa->useIntensities( false );

   const GFXBitmap * pBitmap = mat->getTextureMap();
   if ((pBitmap->attribute & (BMA_ADDITIVE|BMA_SUBTRACTIVE))==0)
      srf->setAlphaSource(GFX_ALPHA_TEXTURE);
   else
   {
      if ((pBitmap->attribute & BMA_ADDITIVE) != 0)
         srf->setAlphaSource(GFX_ALPHA_ADD);
      else
         srf->setAlphaSource(GFX_ALPHA_SUB);
   }
   if (shapeInst->getAlwaysAlpha())
      srf->setConstantAlpha(shapeInst->getAlwaysAlphaValue());

   srf->setTexturePerspective( false );
   srf->setTransparency(FALSE);
   srf->setTextureMap( pBitmap );

   srf->setZTest(GFX_ZTEST);
   pa->drawTriangle( pFace->fVIP, first);
   srf->setZTest(GFX_ZTEST_AND_WRITE);

   if (oldSource != GFX_HAZE_NONE) {
      srf->setHazeSource(oldSource);
      if (oldSource == GFX_HAZE_CONSTANT)
         srf->setConstantHaze(constHaze);
   }
}

//--------------------------------------------------------------------------------
// ShadingFlat   | MatTexture | TextureTranslucent:
//--------------------------------------------------------------------------------
void matrenderFTU(const Material * mat, const char * face)
{
   mat,face;
   
   // not implemented, use shading none
}


//--------------------------------------------------------
// transparent
//--------------------------------------------------------

//--------------------------------------------------------------------------------
// ShadingSmooth | MatTexture | TextureTransparent:
//--------------------------------------------------------------------------------
void matrenderSTA(const Material * mat, const char * face)
{
   mat,face;

   // not implemented -- use shading none
}

//--------------------------------------------------------------------------------
// ShadingNone   | MatTexture | TextureTransparent:
//--------------------------------------------------------------------------------
void matrenderNTA(const Material * mat, const char * face)
{
   pa->useTextures( true );
   srf->setFillMode(GFX_FILL_TEXTURE);
   srf->setTexturePerspective( false );

   srf->setTransparency(TRUE);

   NoShade();

   if (shapeInst->isPerspective())
      srf->setTexturePerspective(true);

   pa->useIntensities( false );

   if (shapeInst->getAlwaysAlpha())
   {
      srf->setAlphaSource(GFX_ALPHA_CONSTANT);
      srf->setConstantAlpha(shapeInst->getAlwaysAlphaValue());
   }
   else
      srf->setAlphaSource(GFX_ALPHA_NONE);

   srf->setTextureMap( mat->getTextureMap() );

   pa->drawTriangle( pFace->fVIP, first);
}

//--------------------------------------------------------------------------------
// ShadingFlat   | MatTexture | TextureTransparent:
//--------------------------------------------------------------------------------
void matrenderFTA(const Material * mat, const char * face)
{
   mat,face;
   
   // not implemented, use shading none
}

}; // namespace TS
