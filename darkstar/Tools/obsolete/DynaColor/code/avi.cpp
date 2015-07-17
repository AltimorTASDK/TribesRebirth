#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#if 0 // Borland 5.02 can't compile vfw.h
#include <vfw.h>

#include "dy_types.h"
#include "globs.h"


typedef struct {
    BITMAPINFOHEADER    bih;
    u8                  pal[1024];
} BIP;


static u8* sourceFile;
static u8* destFile;

static PAVIFILE      pf,pf2;
static PAVISTREAM    ps,ps2;
static PGETFRAME     pgf;

static int numFrames = 0;
static int frameSize = 0;

static u8 *CutBack = 0;
static u8 *CutMask = 0;
static s32 MaxW, MaxH;

COMPVARS CompVars;

static BITMAPINFOHEADER bihIn, bihOut;
static BIP bipIn, bipOut;


void error( const char *format, ... );
void message( const char *format, ... );
void doBmp(u8* name);


static void openSource(void)
{
   if ( AVIFileOpen(&pf, sourceFile, OF_SHARE_DENY_WRITE, 0L) )
      error ("Cannot open file: %s", sourceFile); 

   if ( AVIFileGetStream (pf,&ps,streamtypeVIDEO,0L) )
      error ("%s: Unable to open stream", sourceFile); 

   pgf = AVIStreamGetFrameOpen (ps,NULL);
   if ( !pgf )
   {
      HRESULT          hr;
      AVISTREAMINFO    strhdr;
      char buff1[5],buff2[5];
    
      hr = AVIStreamInfo(ps, &strhdr, sizeof(strhdr)); 
      if (hr != 0) 
         error ("%s: Unable to get stream info",sourceFile);

      strncpy ( buff1, (char*)&strhdr.fccType, 4 );
      strncpy ( buff2, (char*)&strhdr.fccHandler, 4 );
      buff1[4] = 0;
      buff2[4] = 0;

      error ("%s: Unable to open frame (stream type=%s, handler=%s)",
         sourceFile,buff1,buff2);
   }
}


static void CompressFrame ( LONG lFrameNum, LPVOID lpIn )
{
   LPVOID pBits;
   BOOL bKey = 0;
   LONG lSize = 0;


   pBits = ICSeqCompressFrame(&CompVars,0,lpIn,&bKey,&lSize);
   if ( !pBits )
      error ("could not compress frame");
   if ( !bKey )
      error ("not key frame");

   AVIStreamWrite(ps2, lFrameNum, 1, pBits,
      lSize, AVIIF_KEYFRAME, NULL, NULL);
}   

 
static void openDest(void)
{
   HRESULT          hr;
   AVISTREAMINFO    strhdr;
   LONG             lStreamSize;
   int              stride;
    
   // Determine the size of the format data using 
   // AVIStreamFormatSize. 
   AVIStreamFormatSize(ps, 0, &lStreamSize); 
   if (lStreamSize > sizeof(bihIn)) // Format too large?
      error ( "%s: avi must be 24-bit", sourceFile );
 
   lStreamSize = sizeof(bihIn); 
   AVIStreamReadFormat(ps, 0, &bihIn, &lStreamSize); // Read format 
   AVIStreamInfo(ps, &strhdr, sizeof(strhdr)); 

   stride = (bihIn.biWidth+3)&(~3);   //round up to nearest DWORD
   frameSize = stride * bihIn.biHeight;

   // Initialize the bitmap structure. 
   bihIn.biSize = sizeof(BITMAPINFOHEADER); 
   bihIn.biPlanes = 1;
   bihIn.biCompression = BI_RGB;
   bihIn.biBitCount = 8;  // 8 bits-per-pixel format 
   bihIn.biSizeImage = frameSize;
   bihIn.biXPelsPerMeter = bihIn.biYPelsPerMeter = 0;
   bihIn.biClrUsed = bihIn.biClrImportant = 256;

   bipIn.bih = bihIn;
   memcpy ( bipIn.pal, RenderPal, sizeof(bipIn.pal) );

   bihOut = bihIn;
   bipOut = bipIn;

   strhdr.dwSampleSize = frameSize; // fixed sample size

   if (CompressFlag)
   {
      #if 0
      hIC = ICLocate (ICTYPE_VIDEO, 0L,
          (LPBITMAPINFOHEADER)&bihIn, 
          (LPBITMAPINFOHEADER)&bihOut, ICMODE_COMPRESS); 
      #endif

      bihOut.biCompression = BI_RLE8;      // 8-bit RLE for output format
      bipOut.bih.biCompression = BI_RLE8;

      memset (&CompVars,0,sizeof(COMPVARS));
      CompVars.cbSize = sizeof(COMPVARS);
      CompVars.dwFlags = ICMF_COMPVARS_VALID;
      CompVars.hic = ICOpen (ICTYPE_VIDEO,mmioFOURCC('m','r','l','e'),ICMODE_COMPRESS);
      CompVars.lpbiOut = (BITMAPINFO*)&bipOut;
      CompVars.lKey = 1;
      CompVars.lQ = ICQUALITY_HIGH;
      if (CompVars.hic == 0)
         error ("cannot find compressor");

      if ( !ICSeqCompressFrameStart( &CompVars, (BITMAPINFO*)&bipIn ) )
         error ("cannot start compressor");

      strhdr.fccHandler = mmioFOURCC('m','r','l','e');
      strhdr.dwSampleSize = 0;
   }

   // Create new AVI file using AVIFileOpen. 
   hr = AVIFileOpen(&pf2, destFile, OF_WRITE | OF_CREATE, NULL); 
   if (hr != 0) 
       error ("%s: cannot open file",destFile);

   // Create a stream using AVIFileCreateStream. 
   hr = AVIFileCreateStream(pf2, &ps2, &strhdr); 
   if (hr != 0)            //Stream created OK? If not close file.
      error ("%s: cannot create stream",destFile);

   // Set format of new stream using AVIStreamSetFormat. 
   hr = AVIStreamSetFormat(ps2, 0, &bipOut, sizeof(bipOut)); 
   if (hr != 0)
     error ("%s: cannot set format",destFile);
}


static void closeAll(void)
{
   if ( ps2 )
      AVIStreamRelease(ps2);
   if ( pf2 )
      AVIFileRelease(pf2);
   if ( pgf )
      AVIStreamGetFrameClose (pgf);
   if ( ps )
      AVIStreamRelease(ps);
   if ( pf )
      AVIFileRelease(pf);  // closes the file
   ps2 = 0;
   pf2 = 0;
   pgf = 0;
   ps = 0;
   pf = 0;

   if (CompVars.hic)
   {
      ICSeqCompressFrameEnd(&CompVars);
      ICCompressorFree(&CompVars);
      CompVars.hic = 0;
   }
   if (CutBack)
   {
      free(CutBack);
      CutBack = 0;
   }
   if (CutMask)
   {
      free(CutMask);
      CutMask = 0;
   }
}


void initAVI(void)
{
   AVIFileInit();          // opens AVIFile library
}


void exitAVI(void)
{
   closeAll();
   AVIFileExit();          // releases AVIFile library 
}


static s32 X,Y,W,H;


static void CutBitmap ()
{
   if ( numFrames == 0 )
   {
      CutBack = (u8*)malloc(Width*Height*3);
      CutMask = (u8*)malloc(Width*Height);
      if ( !CutBack || !CutMask )
         error ("not enough memory for -cut");

      memcpy ( CutBack, Data, Width*Height*3 );
      memset ( CutMask, 0, Width*Height );

      MaxW = 0;
      MaxH = 0;
   }
   else
   {
      s32 X1,X2,Y1,Y2;
      u8* dataPtr = Data;
      u8* backPtr = CutBack;
      u8* maskPtr = CutMask;

      X1=Y1=999;
      X2=Y2=0;

      for ( Y=0; Y<(s32)Height; Y++ )
      for ( X=0; X<(s32)Width; X++ )
      {
         s32 d1,d2,d3,dist;

         d1 = *dataPtr++ - *backPtr++;
         d2 = *dataPtr++ - *backPtr++;
         d3 = *dataPtr++ - *backPtr++;

         d1 = abs(d1);
         d2 = abs(d2);
         d3 = abs(d3);

         dist=d1;
         if (d2>dist)
            dist=d2;
         if (d3>dist)
            dist=d3;

         if ( dist > (s32)CutThreshold )
         {
            *maskPtr++ = 1;

            if (X<X1)
               X1=X;
            if (X>X2)
               X2=X;
            if (Y<Y1)
               Y1=Y;
            if (Y>Y2)
               Y2=Y;
         }
         else
         {
            *maskPtr++ = 0;
         }
      }

      X=X1;
      if (X1<=X2)
         W=X2-X1+1;
      else
         W=0;

      Y=Y1;
      if (Y1<=Y2)
         H=Y2-Y1+1;
      else
         H=0;

      if (W>MaxW)
         MaxW=W;
      if (H>MaxH)
         MaxH=H;
   }
}


static void DrawBox ( u8 color )
{
   s32 i,off;
   s32 X2=X-1;
   s32 Y2=Y-1;
   s32 W2=W+2;
   s32 H2=H+2;

   if (Y2 >= 0)
   {
      off = X2+Y2*Width;
      for ( i=0; i<W2; i++, off++ )
         if (X2+i >= 0 && X2+i < (s32)Width)
            RenderData[off] = color;
   }

   if (Y2+H2-1 < (s32)Height)
   {
      off = X2+(Y2+H2-1)*Width;
      for ( i=0; i<W2; i++, off++ )
         if (X2+i >= 0 && X2+i < (s32)Width)
            RenderData[off] = color;
   }

   if (X2 >= 0)
   {
      off = X2+Y2*Width;
      for ( i=0; i<H2; i++, off+=Width )
         if (Y2+i >= 0 && Y2+i < (s32)Height)
            RenderData[off] = color;
   }

   if (X2+W2-1 < (s32)Width)
   {
      off = (X2+W2-1)+Y2*Width;
      for ( i=0; i<H2; i++, off+=Width )
         if (Y2+i >= 0 && Y2+i < (s32)Height)
            RenderData[off] = color;
   }
}


static void ProcessBitmap (BITMAPINFOHEADER* bi)
{
   BITMAPINFO* bmi = (BITMAPINFO*)bi;
   VOID* pImage = &bmi->bmiColors[0];
   char buff[256];

   int stride = (bi->biWidth+3)&(~3);   //round up to nearest DWORD
   if ( stride != bi->biWidth )
      error ( "%s: avi width not mod 4", sourceFile );

   if ( bi->biBitCount != 24 )
      error ( "%s: avi must be 24-bit", sourceFile );

   Data=(unsigned char *)pImage;
   Mask=0;
   Width=bi->biWidth;
   Height=bi->biHeight;

   // Cut bitmaps out of avi?
   if (RenderABitmap==1 && CutFlag)
   {
      CutBitmap ();
      Mask=CutMask;
   }

   sprintf (buff,"%s, Frame %d",sourceFile,numFrames+1);
   doBmp (buff);

   if (RenderABitmap==1)
   {
      if (CutFlag)
      {
         if ( numFrames == 0 )
         {
            *(RenderPal+255*4)=255;
            *(RenderPal+255*4+1)=255;
            *(RenderPal+255*4+2)=255;
         }
         else
            DrawBox(255);
      }

      if ( numFrames == 0 )
         openDest ();

      if (CompressFlag)
      {
         CompressFrame ( (LONG)numFrames, RenderData );
      }
      else
      {
         AVIStreamWrite(ps2, (LONG)numFrames, 1, RenderData,
            frameSize, AVIIF_KEYFRAME, NULL, NULL);
      }
   }
}


static s32 ClampFrame ( s32 val, s32 minVal, s32 maxVal )
{
   if ( val < minVal )
      return(minVal);
   else if ( val > maxVal )
      return(maxVal);
   return(val);
}


static void ProcessStream ()
{
   u32 frameCount = AVIStreamLength (ps);
   if (frameCount>0)
   {
      s32 firstFrame, lastFrame;
      s32 pos, frame;
      LPVOID pbitmap;

      numFrames = 0;

      firstFrame = ClampFrame(MinAviFrame,0,frameCount-1);
      lastFrame = ClampFrame(MaxAviFrame,0,frameCount-1);
      if ( TrimFrames )
      {
         if ( TrimFrames < 0 )
            firstFrame = ClampFrame(firstFrame-TrimFrames,0,frameCount-1);
         else
            lastFrame = ClampFrame(lastFrame-TrimFrames,0,frameCount-1);
      }

      #if 0
      frame = 0;
      pos = AVIStreamFindSample(ps, 0L, FIND_FROM_START | FIND_ANY);
      while ( pos != -1 )
      {
         if ( frame >= firstFrame && frame <= lastFrame )
         {
            pbitmap = AVIStreamGetFrame (pgf,pos);
            if ( !pbitmap )
               error ("%s: Unable to get frame",sourceFile);

            ProcessBitmap ((BITMAPINFOHEADER*)pbitmap);
            numFrames ++;
         }

         frame ++;
         pos = AVIStreamFindSample(ps, pos + 1, FIND_NEXT | FIND_ANY);
      }
      #else
      for ( frame=firstFrame; frame<=lastFrame; frame++ )
      {
         s32 frame2 = frame;
         if ( frame2>firstFrame )
         {
            s32 count2 = (lastFrame-firstFrame);
            frame2 -= (firstFrame+1);
            frame2 = (frame2+count2+RotateFrames) % count2;
            frame2 += (firstFrame+1);
         }

         pos = AVIStreamFindSample(ps, frame2, FIND_FROM_START | FIND_ANY);
         if ( pos != frame2 )
            error("%s: Unable to find frame",sourceFile);

         pbitmap = AVIStreamGetFrame (pgf,pos);
         if ( !pbitmap )
            error ("%s: Unable to get frame",sourceFile);

         ProcessBitmap ((BITMAPINFOHEADER*)pbitmap);
         numFrames ++;
      }
      #endif
   }
   message("\n");
}


void loadAVI(u8* _sourceFile,u8* _destFile)
{
   sourceFile = _sourceFile;
   destFile = _destFile;
   openSource();
   ProcessStream ();
   closeAll ();
   if (RenderABitmap==1 && CutFlag)
      message ("rect=(%d,%d)",MaxW,MaxH);
}

#endif // Borland 5.02 can't compile vfw.h