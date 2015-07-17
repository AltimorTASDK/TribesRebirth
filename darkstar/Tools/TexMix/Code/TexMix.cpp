//----------------------------------------------------------------------------
//
// TexMix24.cpp
//
// This is a tool that creates transition textures between base
// texture types for the ES terrain system.
//
//----------------------------------------------------------------------------

#include <stdio.h>

#include <types.h>
#include <g_bitmap.h>
#include <g_pal.h>

float   Version = 2.02;

//############################################################################

void merge24( GFXBitmap *pDest, GFXBitmap *pSrc1, GFXBitmap *pSrc2, GFXBitmap *pMask );
void merge( GFXBitmap *pDest, GFXBitmap *pSrc1, GFXBitmap *pSrc2, GFXBitmap *pMask, GFXPalette *pPal );

int main(int argc, char *argv[], char *)
{
   GFXBitmap   *pBWWW;
   GFXBitmap   *pWWBB;
   GFXBitmap   *pWBBB;
   GFXBitmap   *pWBBW;
   GFXPalette  *pPal;

   char  *G;
   char  *T;
   char  *R;

   bool  bit24;
   
   Assert.setFlags( ASSERT_NO_DIALOG|ASSERT_NO_WARNING );

   if ( argc == 1 )
   {
      printf( "TexMix, Version %.2f.  A tool for creating landscape texture transitions\n", Version );
      printf( "Works with both 8-bit and 24-bit images\n"
              "Usage:\n"
              "   TexMix [-mName] <type 1 letter> <type 2 letter> [type 3 letter]\n\n"
              " example:  TexMix G T\n"
              "   you must supply:   GGGG.bmp and TTTT.bmp\n"
              "   program produces:\n"
              "   TGGG.bmp\n"
              "   GGTT.bmp\n"
              "   GTTT.bmp\n\n"
              " example (three-way):  TexMix G T R\n"
              "   program produces:\n"
              "   GRTT.bmp\n"
              "   GTRR.bmp\n"
              "   TRGG.bmp\n\n"
              " -mName  where Name is the base name of the masks used in the mix.\n"
              "         The default Name is mask which loads in the mask bmp's:\n"
              "         mask.BWWW.bmp, mask.WBBB.bmp, mask.WBBW.bmp, mask.BWWB.bmp\n"
              "         If you specify a Name, you must supply all of the masks.\n"
              "         If masks are not found in the current directory, then they\n"
              "         are searched for in the TexMix.exe executable directory\n"
              " example:  TexMix -mCoarse G T\n"
              "   You must supply all of the mask bitmaps:\n"
              "      Course.BWWW.bmp, Course.WBBB.bmp, Course.WBBW.bmp, Course.BWWB.bmp\n\n"
               );

   }
   if ( argc > 2 )
   {
      int off=0;
      char *mask = argv[1];
      if ( *mask == '-' )
      {
         off++;
         mask+=2;
      }
      else
         mask = "mask";

      G = argv[1+off];
      T = argv[2+off];

      GFXBitmap *pGGGG = GFXBitmap::load( avar("%s%s%s%s.bmp", G,G,G,G), BMF_INCLUDE_PALETTE );
      GFXBitmap *pTTTT = GFXBitmap::load( avar("%s%s%s%s.bmp", T,T,T,T) );

      if ( !pGGGG || !pTTTT )
      {
         printf( "missing a base file type %s%s%s%s.bmp or %s%s%s%s.bmp\n",G,G,G,G,T,T,T,T );
         return (1);
      }

      bit24 = ( pGGGG->bitDepth == 24 );

      char exePath[512];
      GetModuleFileName(GetModuleHandle(NULL), exePath, 512);
      char *c = strrchr(exePath, '\\');
      if (c) *c = '\0';

      if ( (pBWWW = GFXBitmap::load(avar("%s.BWWW.bmp",mask))) == NULL )
         AssertFatal( ((pBWWW = GFXBitmap::load(avar("%s\\%s.BWWW.bmp",exePath,mask))) != NULL ), avar("Can't find %s\\%s.BWWW.bmp",exePath,mask) );
      if ( (pWWBB = GFXBitmap::load(avar("%s.WWBB.bmp",mask))) == NULL )
         AssertFatal( ((pWWBB = GFXBitmap::load(avar("%s\\%s.WWBB.bmp",exePath,mask))) != NULL ), avar("Can't find %s\\%s.WWBB.bmp",exePath,mask) );
      if ( (pWBBB = GFXBitmap::load(avar("%s.WBBB.bmp",mask))) == NULL )
         AssertFatal( ((pWBBB = GFXBitmap::load(avar("%s\\%s.WBBB.bmp",exePath,mask))) != NULL ), avar("Can't find %s\\%s.WBBB.bmp",exePath,mask) );
      if ( (pWBBW = GFXBitmap::load(avar("%s.WBBW.bmp",mask))) == NULL )
         AssertFatal( ((pWBBW = GFXBitmap::load(avar("%s\\%s.WBBW.bmp",exePath,mask))) != NULL ), avar("Can't find %s\\%s.WBBW.bmp",exePath,mask) );

      if ( argc == 3+off )
      {
         GFXBitmap *pGGGT = GFXBitmap::create( pGGGG->width, pGGGG->width, bit24? 24:8 );
         GFXBitmap *pGGTT = GFXBitmap::create( pGGGG->width, pGGGG->width, bit24? 24:8 );
         GFXBitmap *pTTTG = GFXBitmap::create( pGGGG->width, pGGGG->width, bit24? 24:8 );

         if ( bit24 ) 
         {
            merge24( pGGGT, pGGGG, pTTTT, pBWWW );
            merge24( pGGTT, pGGGG, pTTTT, pWWBB );
            merge24( pTTTG, pGGGG, pTTTT, pWBBB );
         }
         else
         {
            pPal = pGGGG->pPalette;
            merge( pGGGT, pGGGG, pTTTT, pBWWW, pPal );
            merge( pGGTT, pGGGG, pTTTT, pWWBB, pPal );
            merge( pTTTG, pGGGG, pTTTT, pWBBB, pPal );
            pGGGT->pPalette = pPal;
            pGGTT->pPalette = pPal;
            pTTTG->pPalette = pPal;
         }

         pGGGT->write( avar("%s%s%s%s.bmp", T,G,G,G) );
         pGGTT->write( avar("%s%s%s%s.bmp", G,G,T,T) );
         pTTTG->write( avar("%s%s%s%s.bmp", G,T,T,T) );
      }
      else if ( argc == 4+off )
      {
         R = argv[3+off];
         GFXBitmap *pRRRR = GFXBitmap::load( avar("%s%s%s%s.bmp", R,R,R,R) );
         if ( !pRRRR )
         {
            printf( "missing a base file type %s%s%s%s.bmp\n",R,R,R,R );
            return (1);
         }

         GFXBitmap *pTRGG = GFXBitmap::create( pGGGG->width, pGGGG->width, bit24? 24:8 );
         GFXBitmap *pGRTT = GFXBitmap::create( pGGGG->width, pGGGG->width, bit24? 24:8 );
         GFXBitmap *pGTRR = GFXBitmap::create( pGGGG->width, pGGGG->width, bit24? 24:8 );

         if ( bit24 )
         {
            merge24( pTRGG, pTTTT, pRRRR, pWBBW );
            merge24( pTRGG, pTRGG, pGGGG, pWWBB );

            merge24( pGRTT, pGGGG, pRRRR, pWBBW );
            merge24( pGRTT, pGRTT, pTTTT, pWWBB );

            merge24( pGTRR, pGGGG, pTTTT, pWBBW );
            merge24( pGTRR, pGTRR, pRRRR, pWWBB );
         }
         else
         {
            pPal = pGGGG->pPalette;
            merge( pTRGG, pTTTT, pRRRR, pWBBW, pPal );
            merge( pTRGG, pTRGG, pGGGG, pWWBB, pPal );

            merge( pGRTT, pGGGG, pRRRR, pWBBW, pPal );
            merge( pGRTT, pGRTT, pTTTT, pWWBB, pPal );

            merge( pGTRR, pGGGG, pTTTT, pWBBW, pPal );
            merge( pGTRR, pGTRR, pRRRR, pWWBB, pPal );
            pTRGG->pPalette = pPal;
            pGRTT->pPalette = pPal;
            pGTRR->pPalette = pPal;
         }

         pTRGG->write( avar("%s%s%s%s.bmp", T,R,G,G) );
         pGRTT->write( avar("%s%s%s%s.bmp", G,R,T,T) );
         pGTRR->write( avar("%s%s%s%s.bmp", G,T,R,R) );
      }
      return (0);
   }
   return (1);
}

//----------------------------------------------------------------------------

struct RGB
{
   BYTE cRed;
   BYTE cGreen;
   BYTE cBlue;
};
 
void merge24( GFXBitmap *pDest, GFXBitmap *pSrc1, GFXBitmap *pSrc2, GFXBitmap *pMask )
{
   RGB  *pD  = (RGB*)pDest->pBits;
   RGB  *pS1 = (RGB*)pSrc1->pBits;
   RGB  *pS2 = (RGB*)pSrc2->pBits;
   BYTE *pM  = pMask->pBits;

   for ( int i=0; i<pDest->imageSize/3; i++ )
   {
      pD->cRed   = (((int)(pS1->cRed   - pS2->cRed)   * *pM)>>8) + pS2->cRed;
      pD->cGreen = (((int)(pS1->cGreen - pS2->cGreen) * *pM)>>8) + pS2->cGreen;
      pD->cBlue  = (((int)(pS1->cBlue  - pS2->cBlue)  * *pM)>>8) + pS2->cBlue;
      pD++; pS1++; pS2++; pM++;
   }
   int size = pDest->pBits - (BYTE*)pD;
      AssertWarn( size, "wow" );
}

void merge( GFXBitmap *pDest, GFXBitmap *pSrc1, GFXBitmap *pSrc2, GFXBitmap *pMask, GFXPalette *pPal )
{
   BYTE *pD  = pDest->pBits;
   BYTE *pS1 = pSrc1->pBits;
   BYTE *pS2 = pSrc2->pBits;
   BYTE *pM  = pMask->pBits;

   for ( int i=0; i<pDest->imageSize; i++ )
   {
      PALETTEENTRY peD;
      
      peD.peRed   = (((int)(pPal->palette[0].color[*pS1].peRed  - pPal->palette[0].color[*pS2].peRed)   * *pM)>>8) + pPal->palette[0].color[*pS2].peRed;
      peD.peGreen = (((int)(pPal->palette[0].color[*pS1].peGreen - pPal->palette[0].color[*pS2].peGreen) * *pM)>>8) + pPal->palette[0].color[*pS2].peGreen;
      peD.peBlue  = (((int)(pPal->palette[0].color[*pS1].peBlue  - pPal->palette[0].color[*pS2].peBlue)  * *pM)>>8) + pPal->palette[0].color[*pS2].peBlue; 
      *pD = pPal->GetNearestColor( peD );

      pD++; pS1++; pS2++; pM++;
   }
}
