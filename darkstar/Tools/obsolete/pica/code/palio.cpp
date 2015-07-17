//---------------------------------------------------------------------------
//
//
//  Filename   : palio.cpp
//
//
//  Description: private imageFile functions for saving and loading
//               palette files.
//               
//               
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
#include "base.h"

//---------------------------------------------------------------------------
//
//  Function Name: SavePAL
//
//  Arguments    : none
//
//
//  Comments     : Saves a Palette from a ".pal" file OR an 8-bit bitmap
//                 file.
//                 
//                 
//
//
//---------------------------------------------------------------------------

void imageFile::SavePAL(void)
{
    FILE    *fp;
    RIFFHDR palette = {{'R','I','F','F'},1040,{'P','A','L',' '},{'d','a','t','a'},4*256+4,0x1000300};


    if ((fp = fopen(Filename,"wb")) == NULL)
    {
        printf("Cannot save Palette\n");
        AssertFatal(fp,avar("Cannot save Palette to %s\n",Filename));
        exit(0);
    }
    fwrite(&palette,sizeof(RIFFHDR),1,fp);
    fwrite(Pal,sizeof(PALETTEENTRY),256,fp);
    fclose(fp);
}   


//---------------------------------------------------------------------------
//
//  Function Name: LoadPAL
//
//  Arguments    : none
//
//
//  Comments     : Load a Palette from a ".pal" file OR an 8-bit bitmap
//                 file.
//                 
//                 
//
//
//---------------------------------------------------------------------------

void imageFile::LoadPAL(void)
{
    FILE                *fp;
    RIFFHDR             palette;
    PALETTEENTRY        inPal[256];
    BITMAPFILEHEADER    testforBmp;
    BITMAPINFOHEADER    bmapinfo;
    COUNTER             numColors;
    COUNTER             index;
    WORD                bmpWord;

    bmpWord = MAKEWORD('B','M');

   if((strstr(Filename,".tap") != NULL) || (strstr(Filename,".TAP") != NULL)) {
      LoadTextAlphaPAL();
      return;
   }

    if((fp = fopen(Filename,"rb")) == NULL)
    {
        printf("ERROR - Cannot read Palette from : %s\n",Filename);
        exit(0);
    }
    fread(&testforBmp,sizeof(BITMAPFILEHEADER),1,fp);

    // If it's a bitmap file, try to read in the color table.
    if(testforBmp.bfType == bmpWord)
    {
        fread(&bmapinfo, sizeof(BITMAPINFOHEADER),1,fp);
        if(bmapinfo.biBitCount != 8)
        {
            printf("Cannot read a palette from a >8bit bmp : %s\n",Filename);
            AssertFatal(1,avar("Cannot read a palette from a >8bit bmp: %s\n",Filename));
            exit(0);
        }
        numColors = bmapinfo.biClrUsed;
        fread(&inPal, sizeof(PALETTEENTRY), numColors, fp);
        fclose(fp);
    }

    // Otherwise it's an actual Microsoft palette file:
    else
    {
        fclose(fp);
        fp = fopen(Filename,"rb");
        fread(&palette,sizeof(RIFFHDR),1,fp);
        fread(inPal,sizeof(PALETTEENTRY),256,fp);
        numColors = 256;
        fclose(fp);
    }

    // Copy the palette from our read-in source (inPal) to the class's internal
    // palette member : Pal
    for(index = 0;index < numColors;index++)
    {
        Pal[index].peRed    = inPal[index].peRed;
        Pal[index].peBlue   = inPal[index].peBlue;
        Pal[index].peGreen  = inPal[index].peGreen;
        Pal[index].peFlags  = inPal[index].peFlags;
    }       
}   


// Note: Not a heck of a lot of room for error in this function...
//
void imageFile::LoadTextAlphaPAL(void)
{
   FILE *fp;
   if((fp = fopen(Filename,"rb")) == NULL)
   {
       printf("ERROR - Cannot read Palette from : %s\n",Filename);
       exit(0);
   }

   for (int i = 0; i < 256; i++) {
      int red, green, blue, alpha;
      fscanf(fp, "%d %d %d %d", &red,
                                &green,
                                &blue,
                                &alpha);
      Pal[i].peRed   = UInt8(red);
      Pal[i].peGreen = UInt8(green);
      Pal[i].peBlue  = UInt8(blue);
      Pal[i].peFlags = UInt8(alpha);
   }
   fclose(fp);
}