//---------------------------------------------------------------------------
//
//
//  Filename   : tgaio.cpp
//
//
//  Description: private imageFile functions for saving and loading
//               targa files. The saving feature is not implemented. 
//               Loading is only for targas that are 32-bit with alpha
//               channel.
//               
//               
//               
//      
//
//  Author     : Joel Franklin 
//
//
//  Date       : Friday 13th 1997
//
//
//
//---------------------------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "pcatypes.h"
#include "imagefile.h"
#include "tga.h"

BYTE GetByte(FILE*);
WORD GetWord(FILE*);
DWORD GetDword(FILE*);
SHORT ReadTgaHeader(TGA*, FILE*);
SHORT TgaDecodeScanLine( BYTE*,WORD,WORD,FILE*);
void LoadTGA(char*,TGA*);
void FreeTGA(TGA*);


//---------------------------------------------------------------------------
//
//  Function Name: LoadTGA32
//
//  Arguments    : none
//
//
//  Comments     : Loads a 24-bit bitmap. Allocates the Data24 and Data8 buffers
//                 and zeroes them, then reads into Data24. Sets Height,
//                 Width and ImageSize.
//
//
//---------------------------------------------------------------------------

void imageFile::LoadTGA32(void)
{
    TGA tgaInfo;
    COUNTER linIndex = 0;
    COUNTER index;
    LoadTGA(Filename, &tgaInfo);
    Width = tgaInfo.Head.ImageWidth;
    Height = tgaInfo.Head.ImageHeight;

    Data24 = new PALETTEENTRY[Height*Width];
    Data8  = new UBYTE[Height*Width];

    for(index = 0;index < Height*Width;index++)
    {
        Data24[index].peRed   = 0;
        Data24[index].peGreen = 0;
        Data24[index].peBlue  = 0;
        Data24[index].peFlags = 0;
        Data8[index] = 0;
    }

    for(index = 0;index < Width*Height;index++)
    {
        Data24[index].peBlue  = tgaInfo.Data[linIndex];
        Data24[index].peGreen = tgaInfo.Data[linIndex + 1];
        Data24[index].peRed   = tgaInfo.Data[linIndex + 2];
        Data24[index].peFlags = tgaInfo.Data[linIndex + 3];
        linIndex += 4;
    }
    Bmapinfo.biHeight = Height;
    Bmapinfo.biWidth = Width;
    Bmapinfo.biSize = 40;
    Bmapinfo.biPlanes = 1;
}


//---------------------------------------------------------------------------
//
//  Function Name: SaveTGA32
//
//  Arguments    : none
//
//
//  Comments     : NOT YET IMPLEMENTED
//                 
//                 
//                 
//
//
//---------------------------------------------------------------------------

void imageFile::SaveTGA32(void)
{
}



//===========================================================================
// Below here are Dave McClurg's functions for TGA reading. 
//===========================================================================
BYTE GetByte(FILE* fp)
{
   BYTE x;
   fread(&x,sizeof(x),1,fp);
   return(x);
}


WORD GetWord(FILE* fp)
{
   WORD x;
   fread(&x,sizeof(x),1,fp);
   return(x);
}


DWORD GetDword(FILE* fp)
{
   DWORD x;
   fread(&x,sizeof(x),1,fp);
   return(x);
}


SHORT ReadTgaHeader(TGA* TgaHead, FILE* FpTga)
{
   WORD i;

   /*
   ** Read the TGA header (Version 1.0 and 2.0).
   */
   TgaHead->Head.IdLength      = GetByte(FpTga);
   TgaHead->Head.CmapType      = GetByte(FpTga);
   TgaHead->Head.ImageType     = GetByte(FpTga);
   TgaHead->Head.CmapIndex     = GetWord(FpTga);
   TgaHead->Head.CmapLength    = GetWord(FpTga);
   TgaHead->Head.CmapEntrySize = GetByte(FpTga);
   TgaHead->Head.X_Origin      = GetWord(FpTga);
   TgaHead->Head.Y_Origin      = GetWord(FpTga);
   TgaHead->Head.ImageWidth    = GetWord(FpTga);
   TgaHead->Head.ImageHeight   = GetWord(FpTga);
   TgaHead->Head.PixelDepth    = GetByte(FpTga);
   TgaHead->Head.ImagDesc      = GetByte(FpTga);
                                                                                     
   /* Read the Image ID field. */
   if (TgaHead->Head.IdLength)
   {
      /* Allocate memory for the Id Field data */
      if ((TgaHead->Cmap.IdField =
          (CHAR *) calloc(TgaHead->Head.IdLength, sizeof(CHAR))) ==
          (CHAR *) NULL)
      {
          return(-1);     /* Failed to allocate memory */
      }

      /* Read the Image ID data */
      /* Do not assume that it is a NULL-terminated string */
      for (i = 0; i < TgaHead->Head.IdLength; i++)
          TgaHead->Cmap.IdField[i] = (CHAR) GetByte(FpTga);
   }

   /*
   ** Read the color map data (Version 1.0 and 2.0).
   */
   if (TgaHead->Head.CmapType)
   {
      /* Determine the size of the color map */
      WORD cmapsize = ((TgaHead->Head.CmapEntrySize + 7) >> 3) *
                   TgaHead->Head.CmapLength;

      /* Allocate memory for the color map data */
      if ((TgaHead->Cmap.CmapData =
          (BYTE *) calloc(cmapsize, sizeof(BYTE))) == (BYTE *) NULL)
      {
          return(-2);     /* Failed to allocate memory */
      }

      /* Read the color map data */
      for (i = 0; i < cmapsize; i++)
          TgaHead->Cmap.CmapData[i] = GetByte(FpTga);
   }
   return(0);
}


/*
**  Read encoded data from a TGA image file, decode it, and write it to
**  a buffer.  Assume that the FILE pointer is pointing to the first byte
**  of encoded image data.
**
**  Returns: A negative value if a buffer overflow would occur, otherwise 0
**           if no errors occur.
*/
SHORT TgaDecodeScanLine( BYTE* DecodedBuffer,
   WORD LineLength, WORD PixelSize, FILE* FpTga)
{
   WORD    i;              /* Loop counter                                 */
   SHORT   byteCount;      /* Number of bytes written to the buffer        */
   WORD    runCount;       /* The pixel run count                          */
   WORD    bufIndex;       /* The index of DecodedBuffer                   */
   WORD    bufMark;        /* Index marker of DecodedBuffer                */
   WORD    pixelCount;     /* The number of pixels read from the scan line */

   bufIndex   = 0;         /* Initialize buffer index  */
   byteCount  = 0;         /* Initialize byte count    */
   pixelCount = 0;         /* Initialize pixel counter */

   /* Main decoding loop */
   while (pixelCount < LineLength)
   {
       /* Get the pixel count */
       runCount = GetByte(FpTga);
    
       /* Make sure writing this next run will not overflow the buffer */
       if (pixelCount + (runCount & 0x7f) + 1 > LineLength)
           return(-1);     /* Pixel run will overflow buffer */

       /* If the run is encoded... */
       if (runCount & 0x80)
       {
           runCount &= ~0x80;              /* Mask off the upper bit       */

           bufMark = bufIndex;             /* Save the start-of-run index  */

           /* Update total pixel count */
           pixelCount += (runCount + 1);

           /* Update the buffer byte count */ 
           byteCount += ((runCount + 1) * PixelSize);

           /* Write the first pixel of the run to the buffer */
           for (i = 0; i < PixelSize; i++)
               DecodedBuffer[bufIndex++] = GetByte(FpTga);

           /* Write remainder of pixel run to buffer 'runCount' times */
           while (runCount--)
           {
               for (i = 0; i < PixelSize; i++)
                   DecodedBuffer[bufIndex++] = DecodedBuffer[bufMark + i];
           }
       }
       else    /* ...the run is unencoded (raw) */
       {
           /* Update total pixel count */
           pixelCount += (runCount + 1);
           
           /* Update the buffer byte count */
           byteCount  += ((runCount + 1) * PixelSize);

           /* Write runCount pixels */
           do
           {
               for (i = 0; i < PixelSize; i++)
                   DecodedBuffer[bufIndex++] = GetByte(FpTga);
           }
           while (runCount--);
       }
   }
   return(byteCount);
}


void LoadTGA ( char* fName, TGA* tga )
{
   FILE* fpTgaIn;
   DWORD bytesPerPixel;
//   DWORD bufSize;
   ULONG bufSize;
   BYTE* buffer;
   DWORD imageSize;
   BYTE* dest;
   int destInc;
   WORD i,j,k,byteCount;

   tga->Data = 0;
   tga->Cmap.IdField = 0;
   tga->Cmap.CmapData = 0;

   fpTgaIn = fopen(fName, "rb");
   if ( !fpTgaIn )
    {
    printf("Cannot open file: %s", fName);
      exit(0);
   }

   if ( ReadTgaHeader(tga, fpTgaIn) )
   {
      printf("not enough mem");
      exit(0);
   }

   bytesPerPixel = ((tga->Head.PixelDepth + 7) >> 3);
   bufSize = (tga->Head.ImageWidth * bytesPerPixel);
   buffer = (BYTE *) calloc(bufSize, sizeof(BYTE));
   if (!buffer)
   {
      printf("not enough mem");
      exit(0);
   }
   if (bytesPerPixel<3)
   {
      printf("not true color");
      exit(0);
   }

//   imageSize = (tga->Head.ImageWidth * tga->Head.ImageHeight * 3);
   imageSize = (tga->Head.ImageWidth * tga->Head.ImageHeight * 4);
   tga->Data = (BYTE*)malloc(imageSize);
   if ( !tga->Data )
   {
      printf("not enough mem");
      exit(0);
   }

   if (tga->Head.ImageType == 10)   // encoded?
   {
      //dest = tga->Data;
      for (i = 0; i < tga->Head.ImageHeight; i++)
      {
         byteCount = TgaDecodeScanLine(buffer, tga->Head.ImageWidth,
            (WORD)bytesPerPixel, fpTgaIn);
         if (byteCount != (WORD)(bufSize))
        {
            printf("error decoding scan line.");
            exit(0);
         }

         #if 0
         //memcpy ( dest, buffer, bufSize );
         //dest += buffSize;
         for ( k=j=0; j<tga->Head.ImageWidth; j++ )
         {
            *dest++ = buffer[k++];
            *dest++ = buffer[k++];
            *dest++ = buffer[k++];
            *dest++ = buffer[k++];
//            k += (WORD)(bytesPerPixel-3);
            k += (WORD)(bytesPerPixel - 4);

         }
         #else
         if ( tga->Head.ImagDesc & 0x20 ) // top-to-bottom ordering?
//            dest = tga->Data + ((tga->Head.ImageHeight-1 - i) * tga->Head.ImageWidth * 3);
            dest = tga->Data + ((tga->Head.ImageHeight-1 - i) * tga->Head.ImageWidth * 4);
         else
//            dest = tga->Data + (i * tga->Head.ImageWidth * 3);
            dest = tga->Data + (i * tga->Head.ImageWidth * 4);

//         destInc = 3;
         destInc = 4;
         if ( tga->Head.ImagDesc & 0x10 ) // right-to-left ordering?
         {
//            dest += ((tga->Head.ImageWidth-1) * 3);
            dest += ((tga->Head.ImageWidth-1) * 4);

            destInc = -destInc;
         }

         for ( k=j=0; j<tga->Head.ImageWidth; j++ )
         {
            dest[0] = buffer[k++];
            dest[1] = buffer[k++];
            dest[2] = buffer[k++];
            dest[3] = buffer[k++];

            dest += destInc;
//            k += (WORD)(bytesPerPixel-3);
            k += (WORD)(bytesPerPixel - 4);
         }
         #endif
      }
   }
   else
   if (tga->Head.ImageType == 2)
   {
      //dest = tga->Data;
      for (i = 0; i < tga->Head.ImageHeight; i++)
      {
         byteCount = fread (buffer, 1, bufSize, fpTgaIn);
         if (byteCount != (WORD)(bufSize))
         {
            printf("error readding scan line.");
            exit(0);
         }

         #if 0
         //memcpy ( dest, buffer, bufSize );
         //dest += buffSize;
         for ( k=j=0; j<tga->Head.ImageWidth; j++ )
         {
            *dest++ = buffer[k++];
            *dest++ = buffer[k++];
            *dest++ = buffer[k++];
            *dest++ = buffer[k++];
//            k += (WORD)(bytesPerPixel-3);

            k += (WORD)(bytesPerPixel-4);
         }
         #else
         if ( tga->Head.ImagDesc & 0x20 ) // top-to-bottom ordering?
//            dest = tga->Data + ((tga->Head.ImageHeight-1 - i) * tga->Head.ImageWidth * 3);
            dest = tga->Data + ((tga->Head.ImageHeight-1 - i) * tga->Head.ImageWidth * 4);
         else
            dest = tga->Data + (i * tga->Head.ImageWidth * 4);
//            dest = tga->Data + (i * tga->Head.ImageWidth * 3);

//         destInc = 3;
           destInc = 4;
         if ( tga->Head.ImagDesc & 0x10 ) // right-to-left ordering?
         {
//            dest += ((tga->Head.ImageWidth-1) * 3);
            dest += ((tga->Head.ImageWidth-1) * 4);
            destInc = -destInc;
         }

         for ( k=j=0; j<tga->Head.ImageWidth; j++ )
         {
            dest[0] = buffer[k++];
            dest[1] = buffer[k++];
            dest[2] = buffer[k++];
            dest[3] = buffer[k++];
            dest += destInc;
//            k += (WORD)(bytesPerPixel-3);
            k += (WORD)(bytesPerPixel-4);

         }
         #endif
      }
   }
   else
   {
    printf("targa image is not true color");
      exit(0);
   }

   free (buffer);
   fclose(fpTgaIn);
}


void FreeTGA ( TGA* tga )
{
   if (tga->Data)
      free (tga->Data);
   if (tga->Cmap.IdField)
      free (tga->Cmap.IdField);
   if (tga->Cmap.CmapData)
      free (tga->Cmap.CmapData);
   tga->Data = 0;
   tga->Cmap.IdField = 0;
   tga->Cmap.CmapData = 0;
}
