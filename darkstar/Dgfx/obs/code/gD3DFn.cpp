//------------------------------------------------------------------------------
// Description: Functions to implement Direct3D Drawing
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "r_clip.h"
#include "fn_table.h"
#include "gfxmetrics.h"
#include "gD3DSfc.h"
#include "gD3DHCache.h"
#include "gD3DTxCache.h"


// Copied from D3D Examples for debugging...
//
char *D3DAppErrorToString(HRESULT result);

#define GD3D_ZBUFFERINF  (0.0f);
#define GD3D_ZBUFFERZERO (1.0f);

inline float convertWToZ(const float in_w)
{
   return in_w;
}

namespace Direct3D {

#define UTILITY_FUNCTIONS 1
//------------------------------------------------------------------------------
// NAME 
//    int
//    NearestPow(int size)
//    
// DESCRIPTION 
//    Returns the smallest power of 2 that is greater than size.
//     ie, 13 returns 16, 17 returns 32, etc...
//    
// NOTES 
//    Ripped from glidesfc.cpp DMM
//------------------------------------------------------------------------------
Int32
NearestPow(Int32 size)
{
   int oneCount   = 0;
   int shiftCount = -1;
   
   while(size) {
      if(size & 1)
         oneCount++;
      shiftCount++;
      size >>= 1;
   }
   
   if(oneCount > 1)
      shiftCount++;
   
   return (1 << shiftCount);
}


void 
setHazeNone(Surface*             /*io_pSurface*/,
            D3DTLVERTEX*         io_pVertex,
            const GFXColorInfoF* /*in_c*/)
{
   io_pVertex->specular = D3DRGBA(0.0, 0.0, 0.0, 1.0);
}

void 
setHazeConstant(Surface*             io_pSurface,
                D3DTLVERTEX*         io_pVertex,
                const GFXColorInfoF* /*in_c*/)
{
   float c  = 1.0f - io_pSurface->constantHaze;
   io_pVertex->specular = D3DRGBA(0.0, 0.0, 0.0, c);
}

void 
setHazeVertex(Surface*             /*io_pSurface*/,
              D3DTLVERTEX*         io_pVertex,
              const GFXColorInfoF* in_c)
{
   float c  = 1.0f - in_c->haze;
   io_pVertex->specular = D3DRGBA(0, 0, 0, c);
}


void 
setShadeNone(Surface*             pSurface,
             D3DTLVERTEX*         io_pVertex,
             const GFXColorInfoF* /*in_c*/)
{
   float oldAlpha = float(io_pVertex->color >> 24) / 255.0f;
   if (pSurface->fillMode != GFX_FILL_CONSTANT) {
      io_pVertex->color = D3DRGBA(1.0, 1.0, 1.0, oldAlpha);
   } else {
      io_pVertex->color = D3DRGBA(pSurface->fillColor.r,
                                  pSurface->fillColor.g,
                                  pSurface->fillColor.b,
                                  oldAlpha);
   }
}

void 
setShadeConstant(Surface*             pSurface,
                 D3DTLVERTEX*         io_pVertex,
                 const GFXColorInfoF* /*in_c*/)
{
   float oldAlpha = float(io_pVertex->color >> 24) / 255.0f;
   if (pSurface->fillMode != GFX_FILL_CONSTANT) {
      io_pVertex->color = D3DRGBA(pSurface->shadeColor.r,
                                  pSurface->shadeColor.b,
                                  pSurface->shadeColor.g,
                                  oldAlpha);
   } else {
      io_pVertex->color = D3DRGBA(pSurface->shadeColor.r * pSurface->fillColor.r,
                                  pSurface->shadeColor.b * pSurface->fillColor.g,
                                  pSurface->shadeColor.g * pSurface->fillColor.b,
                                  oldAlpha);
   }
}

void 
setShadeVertex(Surface*             pSurface,
               D3DTLVERTEX*         io_pVertex,
               const GFXColorInfoF* in_c)
{
   float oldAlpha = float(io_pVertex->color >> 24) / 255.0f;
   if (pSurface->fillMode != GFX_FILL_CONSTANT) {
      io_pVertex->color = D3DRGBA(in_c->color.red,
                                  in_c->color.green,
                                  in_c->color.blue,
                                  oldAlpha);
   } else {
      io_pVertex->color = D3DRGBA(in_c->color.red   * pSurface->fillColor.r,
                                  in_c->color.green * pSurface->fillColor.g,
                                  in_c->color.blue  * pSurface->fillColor.b,
                                  oldAlpha);
   }
}


void 
setAlphaNone(Surface*             io_pSurface,
             D3DTLVERTEX*         io_pVertex,
             const GFXColorInfoF* /*in_c*/)
{
   if (io_pSurface->isAlphaInverted() == false) {
      io_pVertex->color = (io_pVertex->color | 0xff000000);
   } else {
      io_pVertex->color = (io_pVertex->color & 0x00ffffff);
   }
}

void 
setAlphaConstant(Surface*             io_pSurface,
                 D3DTLVERTEX*         io_pVertex,
                 const GFXColorInfoF* /*in_c*/)
{
   UInt32 alphaMask  = UInt32(io_pSurface->constantAlpha * 255.0f);
   if (io_pSurface->isAlphaInverted() == false) {
      alphaMask = 255 - alphaMask;
   }
   alphaMask <<= 24;

   io_pVertex->color = (io_pVertex->color & 0x00ffffff) | alphaMask;
}

void 
setAlphaVertex(Surface*             io_pSurface,
               D3DTLVERTEX*         io_pVertex,
               const GFXColorInfoF* in_c)
{
   UInt32 alphaMask  = UInt32(in_c->alpha * 255.0f);
   if (io_pSurface->isAlphaInverted() == false) {
      alphaMask = 255 - alphaMask;
   }
   alphaMask <<= 24;

   io_pVertex->color = (io_pVertex->color & 0x00ffffff) | alphaMask;
}



#define IMPLEMENTED_FUNCTIONS
//------------------------------------------------------------------------------
// NAME 
//    void 
//    EmitPoly(GFXSurface* io_pSurface)
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
EmitPoly(GFXSurface* io_pSurface)                
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   pSurface->emitPoly();
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    ClearScreen(GFXSurface *io_pSurface,
//                DWORD in_color)
//    void 
//    ClearZBuffer(GFXSurface* io_pSurface)            
//    
//
// DESCRIPTION 
//    Clears screen and zbuffer, respectively.  Simply maps these calls to
//   the corresponding Direct3D::Surface functions...
//    
// NOTES 
//    clearScreen does NOT current clear zbuf, and vice versa...
//------------------------------------------------------------------------------
void 
ClearScreen(GFXSurface* io_pSurface,
            DWORD       in_color)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   pSurface->clearScreen(in_color);
}


void 
ClearZBuffer(GFXSurface* io_pSurface)            
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   pSurface->clearZBuffer();
}


void 
DrawRect2d(      GFXSurface* io_pSurface,
           const RectI*      in_rect,
                 DWORD       in_color)
{
   Point2I en;
   Point2I st = in_rect->upperL;
   
   en.x = st.x;
   en.y = in_rect->lowerR.y;
   io_pSurface->drawLine2d(&st, &en, in_color);
   en.y = st.y;
   en.x = in_rect->lowerR.x;
   io_pSurface->drawLine2d(&st, &en, in_color);
   st = in_rect->lowerR;
   en.x = st.x;
   en.y = in_rect->upperL.y;
   io_pSurface->drawLine2d(&st, &en, in_color);
   en.y = st.y;
   en.x = in_rect->upperL.x;
   io_pSurface->drawLine2d(&st, &en, in_color);
}


void 
DrawRect_f(      GFXSurface* io_pSurface,
           const RectI*      in_rect,
                 float       in_w,
                 DWORD       in_color)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   D3DTLVERTEX* pCurrent;
   
   float rectZ = convertWToZ(in_w);
   
   PALETTEENTRY& rColor = io_pSurface->getPalette()->color[in_color];
   float red   = float(rColor.peRed)   / 255.0f;
   float green = float(rColor.peGreen) / 255.0f;
   float blue  = float(rColor.peBlue)  / 255.0f;
   DWORD d3dColor = D3DRGB(red, green, blue);

   pCurrent = pSurface->getPCurrVertex();
   pCurrent->sx    = D3DVALUE(in_rect->upperL.x);
   pCurrent->sy    = D3DVALUE(in_rect->upperL.y);
   pCurrent->sz    = D3DVALUE(rectZ);
   pCurrent->color = d3dColor;
   pSurface->incCurrVertex();
   
   pCurrent = pSurface->getPCurrVertex();
   pCurrent->sx    = D3DVALUE(in_rect->lowerR.x);
   pCurrent->sy    = D3DVALUE(in_rect->upperL.y);
   pCurrent->sz    = D3DVALUE(rectZ);
   pCurrent->color = d3dColor;
   pSurface->incCurrVertex();
   
   pCurrent = pSurface->getPCurrVertex();
   pCurrent->sx    = D3DVALUE(in_rect->lowerR.x);
   pCurrent->sy    = D3DVALUE(in_rect->lowerR.y);
   pCurrent->sz    = D3DVALUE(rectZ);
   pCurrent->color = d3dColor;
   pSurface->incCurrVertex();
   
   pCurrent = pSurface->getPCurrVertex();
   pCurrent->sx    = D3DVALUE(in_rect->upperL.x);
   pCurrent->sy    = D3DVALUE(in_rect->lowerR.y);
   pCurrent->sz    = D3DVALUE(rectZ);
   pCurrent->color = d3dColor;
   pSurface->incCurrVertex();
   
   // Make sure we emit through the function table...
   //
   io_pSurface->setFillMode(GFX_FILL_CONSTANT);
   
   int storeZTest = pSurface->isZEnabled;
   io_pSurface->setZTest(GFX_NO_ZTEST);
   io_pSurface->emitPoly();
   io_pSurface->setZTest(storeZTest);
}



void bitmapDraw(Surface*         io_pSfc,
                const GFXBitmap* in_pBM)
{
   // Scale the bitmaps texture coords to the width and height of the bitmap...
   //
   D3DTLVERTEX* pCurr = io_pSfc->getPCurrVertex();
   float hScale = 1.0f/float(NearestPow(in_pBM->getWidth()));
   float vScale = 1.0f/float(NearestPow(in_pBM->getHeight()));
   for (int i = 0; i < 4; i++) {
      pCurr[i].tu *= hScale;
      pCurr[i].tv *= vScale;
   }

   // Since we cheated, and didn't inc the vertices below, we need to do it
   //  4 times here...
   //
   io_pSfc->incCurrVertex();
   io_pSfc->incCurrVertex();
   io_pSfc->incCurrVertex();
   io_pSfc->incCurrVertex();

   bool storeCK = io_pSfc->transparencyEnabled;
   if (in_pBM->attribute & BMA_TRANSPARENT) {
      io_pSfc->setColorKey(true);
   } else {
      io_pSfc->setColorKey(false);
   }

   // Call the surface's emitPoly directly
   //
   io_pSfc->emitPoly(true, DWORD(in_pBM));
   io_pSfc->setColorKey(storeCK);
}


float lscale;
float rscale;
float tscale;
float bscale;

bool setupBitmapPoints(Surface *sfc,
                       int x0, int y0,
                       int x1, int y1)
{
   lscale = tscale = 0;
   rscale = bscale = 1;

   if(sfc->getFlags() & GFX_DMF_RCLIP) {
      RectI *cr = sfc->getClipRect();
      float le = float(cr->upperL.x);
      float te = float(cr->upperL.y);
      float re = float(cr->lowerR.x + 1);
      float be = float(cr->lowerR.y + 1);

      if(x0 >= re || (x0 + x1) <= le || y0 >= be || (y0 + y1) <= te)
         return false;

      if(x0 < le)
         lscale = float(le - x0) / float(x1);
      if(y0 < te)
         tscale = float(te - y0) / float(y1);
      if(x0 + x1 > re)
         rscale = float(re - x0) / float(x1);
      if(y0 + y1 > be)
         bscale = float(be - y0) / float(y1);
   }

   D3DTLVERTEX* pCurr;
   
   pCurr = sfc->getPCurrVertex();
   pCurr[0].sx = float(x0) + float(lscale * x1);
   pCurr[0].sy = float(y0) + float(lscale * y1);
   pCurr[1].sx = float(x0) + float(rscale * x1);
   pCurr[1].sy = float(y0) + float(tscale * y1);
   pCurr[2].sx = float(x0) + float(rscale * x1);
   pCurr[2].sy = float(y0) + float(bscale * y1);
   pCurr[3].sx = float(x0) + float(lscale * x1);
   pCurr[3].sy = float(y0) + float(bscale * y1);

   for(int i = 0; i < 4; i++) {
      pCurr[i].sz     = GD3D_ZBUFFERZERO;
      pCurr->color    = D3DRGB(1.0, 1.0, 1.0);
      pCurr->specular = D3DRGB(0.0, 0.0, 0.0);
   }

   return true;
}

void
setupBitmapTexCoords(Surface* sfc,
                     float x0, float y0,
                     float x1, float y1)
{
   D3DTLVERTEX* pCurr = sfc->getPCurrVertex();

   pCurr[0].tu  = float(x0) + float(x1 - x0) * lscale;
   pCurr[0].tv  = float(y0) + float(y1 - y0) * tscale;
   pCurr[1].tu  = float(x0) + float(x1 - x0) * rscale;
   pCurr[1].tv  = pCurr[0].tv;
   pCurr[2].tu  = pCurr[1].tu;
   pCurr[2].tv  = float(y0) + float(y1 - y0) * bscale;
   pCurr[3].tu  = pCurr[0].tu;
   pCurr[3].tv  = pCurr[2].tv;

   for(int i = 0; i < 4; i++)
      pCurr[i].rhw = 1.0f;
}

void 
DrawBitmap2d_f(GFXSurface*      io_pSurface,
               const GFXBitmap* in_pBM,
               const Point2I*   in_at,
               GFXFlipFlag      /*in_flip*/)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   if(!setupBitmapPoints(pSurface,
                         in_at->x, in_at->y,
                         in_pBM->getWidth(), in_pBM->getHeight())) {
      return;
   }
   
   setupBitmapTexCoords(pSurface, 0.0f, 0.0f, float(in_pBM->getWidth()), float(in_pBM->getHeight()));

   bitmapDraw(pSurface, in_pBM);
}

void 
DrawBitmap2d_rf(GFXSurface*      io_pSurface,
                const GFXBitmap* in_pBM,
                const RectI*     in_subRegion,
                const Point2I*   in_at,
                GFXFlipFlag      /*in_flip*/)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   if(!setupBitmapPoints(pSurface,
                         in_at->x, in_at->y,
                         in_subRegion->len_x() + 1, in_subRegion->len_y() + 1)) {
      return;
   }
   
   setupBitmapTexCoords(pSurface,
                        float(in_subRegion->upperL.x),     float(in_subRegion->upperL.y),
                        float(in_subRegion->lowerR.x + 1), float(in_subRegion->lowerR.y + 1));

   bitmapDraw(pSurface, in_pBM);
}

void 
DrawBitmap2d_sf(GFXSurface*      io_pSurface,
                const GFXBitmap* in_pBM,
                const Point2I*   in_at,
                const Point2I*   in_stretch,
                GFXFlipFlag      /*in_flip*/)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   if(!setupBitmapPoints(pSurface,
                         in_at->x, in_at->y,
                         in_stretch->x, in_stretch->y)) {
      return;
   }
   
   setupBitmapTexCoords(pSurface, 0, 0, float(in_pBM->getWidth()), float(in_pBM->getHeight()));
   bitmapDraw(pSurface, in_pBM);
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    Draw3dBegin(GFXSurface* io_pSurface)
//    void 
//    Draw3dEnd(GFXSurface* io_pSurface)               
//    
// DESCRIPTION 
//    Called by GFX to enter and exit 3d drawing mode...
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
Draw3dBegin(GFXSurface* io_pSurface)             
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(pSurface->isDrawing3D() == false,
               "mismatched call to Draw3dBegin");

   pSurface->draw3DBegin();
}


void 
Draw3dEnd(GFXSurface* io_pSurface)               
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(pSurface->isDrawing3D() == true, "mismatched call to Draw3dEnd");

   pSurface->draw3DEnd();
}


void 
SetShadeSource(GFXSurface*    io_pSurface,
               GFXShadeSource io_ss)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

	switch(io_ss) {
     case GFX_SHADE_NONE:
      pSurface->vertexShader = setShadeNone;
      break;
     case GFX_SHADE_CONSTANT:
      pSurface->vertexShader = setShadeConstant;
      break;
     case GFX_SHADE_VERTEX:
      pSurface->vertexShader = setShadeVertex;
      break;
     default:
      AssertFatal(0, "Error, unknown shade source...");
      break;
	}
   pSurface->shadeSource = io_ss;
}


void 
SetHazeSource(GFXSurface*   io_pSurface,
              GFXHazeSource io_hs)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   switch(io_hs) {
     case GFX_HAZE_NONE:
      pSurface->vertexHazer = setHazeNone;
      break;
     case GFX_HAZE_CONSTANT:
      pSurface->vertexHazer = setHazeConstant;
      break;
     case GFX_HAZE_VERTEX:
      pSurface->vertexHazer = setHazeVertex;
      break;
     default:
      AssertFatal(0, "Error, unknown haze source");
   }
   pSurface->hazeSource = io_hs;
}


void 
SetAlphaSource(GFXSurface*    io_pSurface,
               GFXAlphaSource io_as)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   switch(io_as) {
     case GFX_ALPHA_NONE:
     case GFX_ALPHA_TEXTURE:
      pSurface->vertexAlpher = setAlphaNone;
      break;
     case GFX_ALPHA_FILL:
     case GFX_ALPHA_CONSTANT:
      pSurface->vertexAlpher = setAlphaConstant;
      break;
     case GFX_ALPHA_VERTEX:
      pSurface->vertexAlpher = setAlphaVertex;
      break;
     default:
      AssertFatal(0, "Error, unknown alpha source...");
      break;
   }
   pSurface->alphaSource = io_as;
}


void 
SetFillColorCF(      GFXSurface* io_pSurface,
               const ColorF*     in_pColor)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(in_pColor != NULL, "Error, no color passed to setconstshadecolor");

   pSurface->fillColor.r = in_pColor->red;
   pSurface->fillColor.g = in_pColor->green;
   pSurface->fillColor.b = in_pColor->blue;
   pSurface->fillColor.a = 1.0;
}


void 
SetFillColorI(GFXSurface* io_pSurface,
              Int32       index)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(pSurface->getPalette() != NULL, "Error, no surface palette");
   AssertFatal(index >= 0 && index < 256, "Error, bad index");

   if (pSurface->alphaSource != GFX_ALPHA_FILL) {
      pSurface->fillColor.r = (pSurface->getPalette()->color[index].peRed)   / 255.0f;
      pSurface->fillColor.g = (pSurface->getPalette()->color[index].peGreen) / 255.0f;
      pSurface->fillColor.b = (pSurface->getPalette()->color[index].peBlue)  / 255.0f;
      pSurface->fillColor.a = 1.0;
   } else {
      pSurface->fillColor.r = (pSurface->getPalette()->transColor[index].peRed)   / 255.0f;
      pSurface->fillColor.g = (pSurface->getPalette()->transColor[index].peGreen) / 255.0f;
      pSurface->fillColor.b = (pSurface->getPalette()->transColor[index].peBlue)  / 255.0f;
      pSurface->setConstantAlpha((pSurface->getPalette()->transColor[index].peBlue)  / 255.0f);
   }
}


void 
SetHazeColorCF(      GFXSurface* io_pSurface,
               const ColorF*     in_pColor)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(in_pColor != NULL, "Error, no color passed to setconstshadecolor");

   pSurface->hazeColor.r = in_pColor->red;
   pSurface->hazeColor.g = in_pColor->green;
   pSurface->hazeColor.b = in_pColor->blue;
   pSurface->hazeColor.a = 1.0;

   D3DCOLOR tempHColor = D3DRGB(pSurface->hazeColor.r,
                                pSurface->hazeColor.g,
                                pSurface->hazeColor.b);
   pSurface->setRenderState(D3DRENDERSTATE_FOGCOLOR, tempHColor);
}


void 
SetHazeColorI(GFXSurface* io_pSurface,
              Int32       index)           
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(pSurface->getPalette() != NULL, "Error, no surface palette");
   AssertFatal(index >= 0 && index < 256, "Error, bad index");

   pSurface->hazeColor.r = (pSurface->getPalette()->color[index].peRed)   / 255.0f;
   pSurface->hazeColor.g = (pSurface->getPalette()->color[index].peGreen) / 255.0f;
   pSurface->hazeColor.b = (pSurface->getPalette()->color[index].peBlue)  / 255.0f;
   pSurface->hazeColor.a = 1.0;

   D3DCOLOR tempHColor = D3DRGB(pSurface->hazeColor.r,
                                pSurface->hazeColor.g,
                                pSurface->hazeColor.b);
   pSurface->setRenderState(D3DRENDERSTATE_FOGCOLOR, tempHColor);
}


void 
SetConstantShadeCF(      GFXSurface* io_pSurface,
                   const ColorF*     in_pColor)      
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(in_pColor != NULL, "Error, no color passed to setconstshadecolor");

   pSurface->shadeColor.r = in_pColor->red;
   pSurface->shadeColor.g = in_pColor->green;
   pSurface->shadeColor.b = in_pColor->blue;
   pSurface->shadeColor.a = 1.0f;
}


void 
SetConstantShadeF(GFXSurface* io_pSurface,
                  float       in_shade)       
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   
   pSurface->shadeColor.r = in_shade;
   pSurface->shadeColor.g = in_shade;
   pSurface->shadeColor.b = in_shade;
   pSurface->shadeColor.a = 1.0f;
}


void 
SetConstantHaze(GFXSurface* io_pSurface,
                float       in_haze)         
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   
   pSurface->constantHaze = in_haze;
}


void 
SetConstantAlpha(GFXSurface* io_pSurface,
                 float       in_alpha)        
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   
   pSurface->constantAlpha = in_alpha;
}


void 
SetZTest(GFXSurface* io_pSurface,
         int         enable)                
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   pSurface->setZEnable(enable);
}


void 
SetTextureWrap(GFXSurface* io_pSurface,
               Bool        wrapEnable)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   pSurface->setWrapEnable(wrapEnable);
}


void 
SetTransparency(GFXSurface* io_pSurface,
                Bool        in_transFlag)         
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   pSurface->setColorKey(in_transFlag);
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    FlushTextureCache(GFXSurface* io_pSurface)
//    
// DESCRIPTION 
//    Does what it says...
//    
//------------------------------------------------------------------------------
void 
FlushTextureCache(GFXSurface* io_pSurface)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   pSurface->flushTextureCache();
}


void 
SetTextureMap(      GFXSurface* io_pSurface,
              const GFXBitmap*  pTextureMap)           
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(pTextureMap != NULL, "Null texture map set");
   AssertFatal(pSurface->pTextureCache != NULL, "Error, surface not locked...");

   pSurface->setTextureMap(pTextureMap);
}


void 
HandleSetTextureMap(      GFXSurface* io_pSurface,
                    const GFXBitmap*  in_pTexture)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(pSurface->pHandleCache != NULL, "Error, surface not locked!");

   pSurface->pHandleCache->handleSetTextureMap(in_pTexture);
}


void 
HandleSetLightMap(GFXSurface*  io_pSurface,
                  int          /*lightScale*/,
                  GFXLightMap* io_pLMap)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(pSurface->pHandleCache != NULL, "Error, surface not locked!");

   pSurface->pHandleCache->handleSetLightMap(io_pLMap);
}


void 
SetFillMode(GFXSurface* io_pSurface,
            GFXFillMode in_fillMode)             
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   pSurface->fillMode = in_fillMode;
}



void 
AddVertexV(      GFXSurface* io_pSurface,
           const Point3F*    in_pt,
                 DWORD       /* */)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   D3DTLVERTEX* pTLVert = pSurface->getPCurrVertex();

   pTLVert->sx  = in_pt->x;   // Set Coords
   pTLVert->sy  = in_pt->y;
   pTLVert->sz  = convertWToZ(in_pt->z);
   pTLVert->rhw = in_pt->z;

   // No shade, haze or alpha on these vertices...
   pSurface->vertexShader(pSurface, pTLVert, NULL);
   pSurface->vertexAlpher(pSurface, pTLVert, NULL);
   pSurface->vertexHazer(pSurface,  pTLVert, NULL);

   // No Texture coords...
   pTLVert->tu = 0.0f;
   pTLVert->tv = 0.0f;
   
   pSurface->incCurrVertex();
}


void 
AddVertexVT(      GFXSurface* io_pSurface,
            const Point3F*    in_pt,
            const Point2F*    in_txPt,
            DWORD             /* */)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   D3DTLVERTEX* pTLVert = pSurface->getPCurrVertex();

   pTLVert->sx  = in_pt->x;   // Set Coords
   pTLVert->sy  = in_pt->y;
   pTLVert->sz  = convertWToZ(in_pt->z);
   pTLVert->rhw = in_pt->z;

   // No shade, haze or alpha on these vertices...
   pSurface->vertexShader(pSurface, pTLVert, NULL);
   pSurface->vertexAlpher(pSurface, pTLVert, NULL);
   pSurface->vertexHazer(pSurface, pTLVert,  NULL);
   
   // Texture coords...
   if (in_txPt != NULL) {
      pTLVert->tu = in_txPt->x;
      pTLVert->tv = in_txPt->y;
   } else {
      pTLVert->tu = 0.0f;
      pTLVert->tv = 0.0f;
   }  

   pSurface->incCurrVertex();
}


void 
AddVertexVC(      GFXSurface*    io_pSurface,
            const Point3F*       in_pt,
            const GFXColorInfoF* in_color,
                  DWORD          /* */)             
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   D3DTLVERTEX* pTLVert = pSurface->getPCurrVertex();

   pTLVert->sx = in_pt->x;   // Set Coords
   pTLVert->sy = in_pt->y;
   pTLVert->sz  = convertWToZ(in_pt->z);
   pTLVert->rhw = in_pt->z;

   pSurface->vertexShader(pSurface, pTLVert, in_color);
   pSurface->vertexAlpher(pSurface, pTLVert, in_color);
   pSurface->vertexHazer(pSurface,  pTLVert, in_color);
   
   // No Texture coords...
   pTLVert->tu = 0.0f;
   pTLVert->tv = 0.0f;
   
   pSurface->incCurrVertex();
}


void 
AddVertexVTC(      GFXSurface*    io_pSurface,
             const Point3F*       in_pt,
             const Point2F*       in_txPt,
             const GFXColorInfoF* in_color,
                   DWORD          /* */)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   D3DTLVERTEX* pTLVert = pSurface->getPCurrVertex();

   pTLVert->sx  = in_pt->x;   // Set Coords
   pTLVert->sy  = in_pt->y;
   pTLVert->sz  = convertWToZ(in_pt->z);
   pTLVert->rhw = in_pt->z;

   pSurface->vertexShader(pSurface, pTLVert, in_color);
   pSurface->vertexAlpher(pSurface, pTLVert, in_color);
   pSurface->vertexHazer(pSurface,  pTLVert,  in_color);
   
   // Texture coords...
   if (in_txPt != NULL) {
      pTLVert->tu = in_txPt->x;
      pTLVert->tv = in_txPt->y;
   } else {
      pTLVert->tu = 0.0f;
      pTLVert->tv = 0.0f;
   }  

   pSurface->incCurrVertex();
}



void 
RegisterTextureCB(GFXSurface*      io_pSurface,
                  GFXTextureHandle io_texHandle,
                  GFXCacheCallback io_callBack,
	               int              sizeX,
	               int              sizeY,
	               int              /*lightScale*/,
	               GFXLightMap*     io_pLMap)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(pSurface->pHandleCache != NULL, "Error, surface not locked...");
   AssertFatal(sizeX == sizeY, "Error, non-square callback texture?");
   AssertFatal(sizeX <= 256 && sizeX > 0, "Error, invalid cb-texture size");

   HandleCache*      pHCache = pSurface->pHandleCache;
   HandleCacheEntry* pEntry  = pHCache->getFreeEntry();

   pEntry->key    = pHCache->getNextCBKey();
   pEntry->handle = io_texHandle;
   pEntry->flags  = 0;
   pEntry->bmp    = NULL;
   pEntry->lMap   = io_pLMap;
   pEntry->size   = sizeX;
   pEntry->cb     = io_callBack;

   pEntry->texScale(1.0, 1.0);
   pEntry->texOffset(0.0, 0.0);

   pEntry->lMapSize.set(NearestPow(io_pLMap->size.x),
                        NearestPow(io_pLMap->size.y));

   AssertFatal(pEntry->lMapSize.x == pEntry->lMapSize.y,
               "Error, lmap not square for CB Texture");
   
   float nearestPow = float(pEntry->lMapSize.x);
   float scale      = float(io_pLMap->size.x-2) / nearestPow;
   float offset     = 0.5f / nearestPow;

   pEntry->lMapScale(scale, scale);
   pEntry->lMapOffset(offset, offset);
   pEntry->clamp = true;

   pHCache->hashInsert(pEntry);
   sizeY;
}


void 
RegisterTexture(GFXSurface*      io_pSurface,
                GFXTextureHandle io_texHandle,
                int sizeX,   int sizeY,
                int offsetX, int offsetY,
                int lightScale,
                GFXLightMap*     io_pLMap,
                const GFXBitmap* in_pTexture,
                int              /*mipLevel*/)
{
   AssertFatal(in_pTexture->getWidth() <= 256 && in_pTexture->getWidth() > 0,
               "Error, invalid texture width...");
   AssertFatal(in_pTexture->getHeight() <= 256 && in_pTexture->getHeight() > 0,
               "Error, invalid texture height...");

   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(pSurface->pHandleCache != NULL, "Error, surface not locked...");

   HandleCache*      pHCache = pSurface->pHandleCache;
   HandleCacheEntry* pEntry  = pHCache->getFreeEntry();

   pEntry->key    = DWORD(in_pTexture);
   pEntry->handle = io_texHandle;
   
   pEntry->size = 0;
   pEntry->cb   = NULL;
   
   pEntry->bmp  = in_pTexture;
   
   if (io_pLMap != NULL) {
      pEntry->lMap = io_pLMap;
      pEntry->lMapSize.set(NearestPow(io_pLMap->size.x),
                           NearestPow(io_pLMap->size.y));
   } else {
      pEntry->lMap = NULL;
   }

   pEntry->flags = 0;

   float hScale = float(sizeX) / float(in_pTexture->getWidth());
   float vScale = float(sizeY) / float(in_pTexture->getHeight());
   pEntry->texScale.set(hScale, vScale);
   pEntry->texOffset.set(hScale * float(offsetX), vScale * float(offsetY));

   if (io_pLMap != NULL) {
      float lMapX = float(sizeX) / float(1 << lightScale);
      float lMapY = float(sizeY) / float(1 << lightScale);
      pEntry->lMapScale.set(lMapX / float(pEntry->lMapSize.x),
                            lMapY / float(pEntry->lMapSize.y));
   
      float baseOffsetX = 0.5f / float(pEntry->lMapSize.x);
      float baseOffsetY = 0.5f / float(pEntry->lMapSize.y);
      pEntry->lMapOffset.set(baseOffsetX + (pEntry->lMapScale.x * io_pLMap->offset.x / float(sizeX)),
                             baseOffsetY + (pEntry->lMapScale.y * io_pLMap->offset.y / float(sizeY)));
   }

   if (io_pLMap != NULL) {
      if (sizeX == in_pTexture->getWidth() && sizeY == in_pTexture->getHeight() &&
          offsetX == 0 && offsetY == 0 &&
          io_pLMap->offset.x == 0 && io_pLMap->offset.y == 0 &&
          ((io_pLMap->size.x - 1) << lightScale) == sizeX &&
          ((io_pLMap->size.y - 1) << lightScale) == sizeY) {
         pEntry->clamp = true;
      } else {
         pEntry->clamp = false;
      }
   } else {
      if (sizeX == in_pTexture->getWidth() && sizeY == in_pTexture->getHeight() &&
          offsetX == 0 && offsetY == 0) {
         pEntry->clamp = true;
      } else {
         pEntry->clamp = false;
      }
   }
   
   pHCache->hashInsert(pEntry);
}


Bool 
SetTextureHandle(GFXSurface*      io_pSurface,
                 GFXTextureHandle io_texHandle)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(pSurface->pHandleCache != NULL, "Error, surface not locked!");

   return pSurface->pHandleCache->setTextureHandle(io_texHandle);
}


void 
DrawPoint(      GFXSurface* io_pSurface,
          const Point2I*    in_pt,
                float       in_w,
                DWORD       in_color)
{
   float red   = float(io_pSurface->getPalette()->color[in_color].peRed)   / 255.0f;
   float green = float(io_pSurface->getPalette()->color[in_color].peGreen) / 255.0f;
   float blue  = float(io_pSurface->getPalette()->color[in_color].peBlue)  / 255.0f;

   D3DCOLOR color    = D3DRGB(red, green, blue);
   D3DCOLOR specular = D3DRGB(0, 0, 0);

   Surface* pSurface  = static_cast<Surface*>(io_pSurface);
   D3DTLVERTEX* pCurr = pSurface->getPCurrVertex();
   
   pCurr->sx       = D3DVALUE(in_pt->x);
   pCurr->sy       = D3DVALUE(in_pt->y);
   pCurr->sz       = convertWToZ(in_w);
   pCurr->rhw      = in_w;
   pCurr->color    = color;
   pCurr->specular = specular;
   pSurface->incCurrVertex();
   pSurface->emitPoint();
}


void 
DrawLine2d(      GFXSurface* io_pSurface,
           const Point2I*    in_st,
           const Point2I*    in_en,
                 DWORD       in_color)
{
   float red   = float(io_pSurface->getPalette()->color[in_color].peRed)   / 255.0f;
   float green = float(io_pSurface->getPalette()->color[in_color].peGreen) / 255.0f;
   float blue  = float(io_pSurface->getPalette()->color[in_color].peBlue)  / 255.0f;

   D3DCOLOR color    = D3DRGB(red, green, blue);
   D3DCOLOR specular = D3DRGB(0, 0, 0);

   Surface* pSurface  = static_cast<Surface*>(io_pSurface);
   D3DTLVERTEX* pCurr = pSurface->getPCurrVertex();

   pCurr[0].sz       = pCurr[1].sz       = 1.0f;
   pCurr[0].rhw      = pCurr[1].rhw      = 1.0f;
   pCurr[0].color    = pCurr[1].color    = color;
   pCurr[0].specular = pCurr[1].specular = specular;
   
   pCurr[0].sx = D3DVALUE(in_st->x);
   pCurr[0].sy = D3DVALUE(in_st->y);
   pCurr[1].sx = D3DVALUE(in_en->x);
   pCurr[1].sy = D3DVALUE(in_en->y);

   // Step vertex twice...
   //
   pSurface->incCurrVertex();
   pSurface->incCurrVertex();
   pSurface->emitLine();
}

void 
FlushTexture(GFXSurface*, const GFXBitmap*, const bool)
{

}


//------------------------------------------------------------------------------
#define DEPRICATED_FUNCTIONS 1
GFXLightMap * 
HandleGetLightMap(GFXSurface* /*io_pSurface*/)
{
   AssertFatal(0, "Error, called obsolete function handlegetlightmap");
   return NULL;
}


void 
HandleSetTextureSize(GFXSurface* io_pSurface,
                     int         in_newSize)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(pSurface->pHandleCache != NULL, "Surface not locked!");

   HandleCacheEntry* pEntry = pSurface->pHandleCache->getCurrEntry();
   if (pEntry->cb && in_newSize != pEntry->size) {
      pEntry->size = in_newSize;
      pEntry->key  = pSurface->pHandleCache->getNextCBKey();
   } else {
      AssertWarn(pEntry->cb != NULL,
                 "Tried to set texturesize on non-callback texture!");
   }
}


#define NOOP_FUNCTIONS
void 
SetColorModel(GFXSurface*   /*io_pSurface*/,
              GFXColorModel /*in_colorModel*/)           
{
   // Nop in DX, always RGB
}


void 
SetTexturePerspective(GFXSurface* /*io_pSurface*/,
                      Bool        /*in_perspTex*/)
{
   // Noop.  In hardware alway perspective...
   //
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    HandleSetMipLevel(GFXSurface* /*io_pSurface*/,
//                      int         /*mipLevel*/)       
// NOTES 
//    Ignored in D3D
//------------------------------------------------------------------------------
void 
HandleSetMipLevel(GFXSurface* /*io_pSurface*/,
                  int         /*mipLevel*/)       
{
   // Miplevel is ignored for D3D, the hardware takes care of it...
   //
}


#define UNIMPLEMENTED_FUNCTIONS
//------------------------------------------------------------------------------
// NAME 
//    void 
//    DrawLine2d(      GFXSurface* /*io_pSurface*/,
//               const Point2I*    /*in_st*/,
//               const Point2I*    /*in_en*/,
//                     DWORD       /*in_color*/)
//    void 
//    DrawCircle2d(      GFXSurface* /*io_pSurface*/,
//                 const RectI*      /*in_rect*/,
//                       DWORD       /*in_color*/)
//    void 
//    DrawCircle2d_f(      GFXSurface* /*io_pSurface*/,
//                   const RectI*      /*in_rect*/,
//                         DWORD       /*in_color*/)
//    void 
//    DrawRect2d(      GFXSurface* /*io_pSurface*/,
//               const RectI*      /*in_rect*/,
//                     DWORD       /*in_color*/)
//    void 
//    DrawBitmap2d_af(      GFXSurface* /*io_pSurface*/,
//                    const GFXBitmap*  /*in_pBM*/,
//                    const Point2I*    /*in_at*/,
//                    const Point2I*    /*in_stretch*/,
//                          Int32       /*rotate*/)
//
// DESCRIPTION 
//    These functions are not implemented for Direct3D yet...
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
DrawCircle2d(      GFXSurface* /*io_pSurface*/,
             const RectI*      /*in_rect*/,
                   DWORD       /*in_color*/)
{
   AssertWarn(0, "circle2d Not implemented for Direct3D");
}

void 
DrawCircle2d_f(      GFXSurface* /*io_pSurface*/,
               const RectI*      /*in_rect*/,
                     DWORD       /*in_color*/)
{
   AssertWarn(0, "circle2d_f Not implemented for Direct3D");
}


void 
DrawBitmap2d_af(      GFXSurface* /*io_pSurface*/,
                const GFXBitmap*  /*in_pBM*/,
                const Point2I*    /*in_at*/,
                const Point2I*    /*in_stretch*/,
                      Int32       /*rotate*/)
{
   AssertWarn(0, "bitmap_af Not implemented for DirectX");
}
                        

}; // namespace Direct3D



//--------------------------------------Function Table for Direct3D::Surface
FunctionTable direct3d_table =
{
	Direct3D::ClearScreen,        // Clear

	Direct3D::DrawPoint,          // Draw Point 2d
	Direct3D::DrawLine2d,         // Draw Line 2d
	Direct3D::DrawCircle2d,       // Draw Circle Outline 2d
	Direct3D::DrawCircle2d_f,     // Draw Circle Filled 2d
	Direct3D::DrawRect2d,         // Draw Rectangle 2d
	Direct3D::DrawRect_f,         // Draw Rectangle 2d, Filled

	Direct3D::DrawBitmap2d_f,     // Draw Bitmap 2d
	Direct3D::DrawBitmap2d_rf,    // Draw Bitmap 2d, Region
	Direct3D::DrawBitmap2d_sf,    // Draw Bitmap 2d, Stretch
	Direct3D::DrawBitmap2d_af,    // Draw Bitmap 2d, Stretch, Rotate

	GFXDrawText_p,          // Draw Text String at Given Point
	GFXDrawText_r,          // Draw Text String at Given Rectangle

	Direct3D::Draw3dBegin,        // Draw 3d scene begin.
	Direct3D::Draw3dEnd,          // Draw 3d scene end.
	Direct3D::AddVertexV,         // Add vertex with no shade/texture
	Direct3D::AddVertexVT,
	Direct3D::AddVertexVC,
	Direct3D::AddVertexVTC,
	Direct3D::EmitPoly,
	Direct3D::SetShadeSource,
	Direct3D::SetHazeSource,
	Direct3D::SetAlphaSource,
	Direct3D::SetFillColorCF,
	Direct3D::SetFillColorI,
	Direct3D::SetHazeColorCF,
	Direct3D::SetHazeColorI,
	Direct3D::SetConstantShadeCF,
	Direct3D::SetConstantShadeF,
	Direct3D::SetConstantHaze,
	Direct3D::SetConstantAlpha,
	Direct3D::SetTransparency,
	Direct3D::SetTextureMap,
	Direct3D::SetFillMode,
	Direct3D::SetColorModel,
	Direct3D::SetTexturePerspective,

	Direct3D::RegisterTexture,
	Direct3D::RegisterTextureCB,
	Direct3D::SetTextureHandle,
	GFXAllocateLightMap,
	Direct3D::HandleGetLightMap,
	Direct3D::HandleSetLightMap,
	Direct3D::HandleSetTextureMap,
	Direct3D::HandleSetMipLevel,
	Direct3D::HandleSetTextureSize,
	Direct3D::FlushTextureCache,
	Direct3D::SetZTest,
	Direct3D::ClearZBuffer,
   Direct3D::SetTextureWrap,
   
   Direct3D::FlushTexture
};
