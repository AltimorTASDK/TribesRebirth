//---------------------------------------------------------------------------
//
//
//  Filename   : bmpio.cpp
//
//
//  Description: private imageFile functions for saving and loading
//               particular types of bitmap files.
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
//  Function Name: LoadBMP24
//
//  Arguments    : none
//
//
//  Comments     : Loads a 24-bit bitmap, needs the temp struct data24BGR
//                 to get it right. Allocates the Data24 and Data8 buffers
//                 and zeroes them, then reads into Data24. Sets Height,
//                 Width and ImageSize.
//
//
//---------------------------------------------------------------------------

void imageFile::LoadBMP24(void)
{
    COUNTER indexx;
    COUNTER indexy;
    COUNTER padIndex = 0;
    COUNTER padWidth;

    UBYTE   *inData;
    FILE    *fp;

    fp = fopen(Filename, "rb");
    if(fp == NULL)
    {
        printf("ERROR - Bitmap file unavailable or corrupted : %s\n", Filename);
        exit(0);
    }

    // Read the header info, set the relevant class data.
    fread(&Bmapfhead, sizeof(Bmapfhead),1,fp );
    fread(&Bmapinfo, sizeof(Bmapinfo), 1, fp);

    Height = Bmapinfo.biHeight;
    Width = Bmapinfo.biWidth;

    // Round up to nearest DWORD. And allocate the 24-bit and 8-bit buffers
    padWidth = ((Width * Bmapinfo.biBitCount >> 3)+3)&(~3);
    ImageSize = Height*Width;
    Data24 = new PALETTEENTRY[ImageSize];
    Data8 = new UBYTE[ImageSize];


    // Zero the buffers.
    for(indexx = 0;indexx < ImageSize;indexx++)
    {
        Data24[indexx].peRed    = 0;
        Data24[indexx].peGreen  = 0;
        Data24[indexx].peBlue   = 0;
        Data24[indexx].peFlags  = 0;
        Data8[indexx]           = 0;
    }

    // Read past the header info
    fseek(fp,Bmapfhead.bfOffBits, SEEK_SET);

    // Unfortunately, because of the DWORD alignment, I cannot just read in BGR structs,
    // I have to read in the extra bytes of padding. Most of our bmps are already
    // DWORD aligned in Width (and Height, for that matter), but on the off chance that some
    // are not, the following takes care of the whole problem.

    inData = new UBYTE[Height*padWidth];
    for(indexx = 0;indexx < Height*padWidth;indexx++)
        inData[indexx] = 0; 

    fread(inData,1,Height*padWidth,fp);
    for(indexy = 0;indexy < Height;indexy++)
    {
        for(indexx = 0;indexx < Width;indexx++)
        {
            Data24[indexy*Width + indexx].peRed   = inData[padIndex + 2];
            Data24[indexy*Width + indexx].peGreen = inData[padIndex + 1];
            Data24[indexy*Width + indexx].peBlue  = inData[padIndex + 0];
            Data24[indexy*Width + indexx].peFlags = 0;
            padIndex += 3;
        }
        padIndex += padWidth - 3*Width;
    }
    fclose(fp);
    delete [] inData;
}
    

//---------------------------------------------------------------------------
//
//  Function Name: LoadBMP8
//
//  Arguments    : none
//
//
//  Comments     : Loads an 8-bit Microsoft bitmap, puts the colors in
//                 the Pal private member, allocates and reads data into
//                 Data8, allocates and zeroes Data24.
//                 
//                 WARNING - this function is never used in Pica, and lacks
//                           DWORD alignment!!
//---------------------------------------------------------------------------

void imageFile::LoadBMP8(void)
{
    FILE    *fp;
    COUNTER index;
    int     numColors;

    fp = fopen(Filename, "rb");
    if(fp == NULL)
    {
        printf("Bitmap file unavailable or corrupted\n");
        exit(0);
    }
    fread(&Bmapfhead, sizeof(Bmapfhead),1,fp );
    fread(&Bmapinfo, sizeof(Bmapinfo), 1, fp);

    Height = Bmapinfo.biHeight;
    Width = Bmapinfo.biWidth;

    ImageSize = Height*Width;
    Data24 = new PALETTEENTRY[Height*Width];
    Data8 = new UBYTE[Height*Width];

    for(index = 0;index < Height*Width;index++)
    {
        Data24[index].peRed = 0;
        Data24[index].peGreen = 0;
        Data24[index].peBlue = 0;
        Data24[index].peFlags = 0;
        Data8[index] = 0;
    }
    numColors = Bmapinfo.biClrUsed;
    fread(&Pal, sizeof(PALETTEENTRY), numColors, fp);
    
    fseek(fp,Bmapfhead.bfOffBits, SEEK_SET);
    fread(Data8,1,Height*Width,fp);
    fclose(fp);
}
    
        
//---------------------------------------------------------------------------
//
//  Function Name: SaveBMP24
//
//  Arguments    : none
//
//
//  Comments     : Saves a 24-bit Microsoft bitmap. Dumps the current
//                 Data24 (in BGR) to the file given by Filename.
//                 
//                 WARNING - another unused function without DWORD alignment.
//                           This is here for testing, but it will not work
//                           with non-DWORD aligned bmps.
//---------------------------------------------------------------------------

void imageFile::SaveBMP24(void)
{
    BITMAPFILEHEADER    bfSave;
    COUNTER             index;
    BGRDATA             *Data24BGR;
    FILE                *fp;

    if ((fp = fopen(Filename,"wb")) == NULL)
    {
        printf("ERROR - Can't save 24 bit bitmap to file %s\n", Filename);
        exit(0);
    }

    bfSave.bfType = MAKEWORD('B','M');
    bfSave.bfReserved1 = 0;
    bfSave.bfReserved2 = 0;
    bfSave.bfOffBits = sizeof (bfSave) + Bmapinfo.biSize;
    bfSave.bfSize = bfSave.bfOffBits + Height*Width*3;
    Bmapinfo.biSizeImage = Height*Width*3;
    fwrite (&bfSave,sizeof(bfSave),1,fp);
    fwrite (&Bmapinfo,sizeof(Bmapinfo),1,fp);
    Data24BGR = new BGRDATA[Height*Width];
    for(index = 0;index < Height*Width;index++)
    {
        Data24BGR[index].peBlue = 0;
        Data24BGR[index].peGreen = 0;
        Data24BGR[index].peRed = 0;
    }
    for(index = 0;index < Height*Width;index++)
    {
        Data24BGR[index].peBlue = Data24[index].peBlue;
        Data24BGR[index].peGreen = Data24[index].peGreen;
        Data24BGR[index].peRed = Data24[index].peRed;
    }
    fwrite(Data24BGR,3,Height*Width,fp);

    fclose(fp);
    delete [] Data24BGR;
}   


//---------------------------------------------------------------------------
//
//  Function Name: SaveBMP8
//
//  Arguments    : none
//
//
//  Comments     : Saves an 8-bit Microsoft bitmap. Dumps the current
//                 Data8 to the file given by Filename. Also saves the 
//                 Pal member in the .bmp file. Zeroes the peFlags (our alpha
//                 channel) - do NOT save alpha channels in the .bmp pals,
//                 Microsoft cannot handle this.
//
//---------------------------------------------------------------------------

void imageFile::SaveBMP8(void)
{
    COUNTER             indexx;
    COUNTER             indexy;
    COUNTER             padWidth;
    COUNTER             padIndex;

    BITMAPFILEHEADER    bfSave;

    FILE                *fp;
    UBYTE               *padData8;
    UBYTE               *outPalette;
    
    outPalette = new UBYTE[256*4];
    COUNTER outIndex = 0;

    if ((fp = fopen(Filename,"wb")) == NULL)
    {
        printf("ERROR - Can't save 8 bit bitmap to file %s\n", Filename);
        AssertFatal(fp, avar("Can't save 8-bit bitmap to file %s\n", Filename));
        exit(0);
    }

    // Sets the relevant information for 8-bit format with full palette.
    // We're reintroducing the padding here, so there is some fiddling
    // going on.
    Bmapinfo.biBitCount = 8;
    padWidth = (Width+3)&(~3);

    Bmapinfo.biClrUsed = 256;
    Bmapinfo.biClrImportant = 256;
    bfSave.bfType = MAKEWORD('B','M');
    bfSave.bfReserved1 = 0;
    bfSave.bfReserved2 = 0;
    // 256*4 is the size of the palette, 256 colors, 4 bytes per color.
    bfSave.bfOffBits = sizeof (bfSave) + Bmapinfo.biSize +  256*4;
    bfSave.bfSize = bfSave.bfOffBits + Height*padWidth;
    Bmapinfo.biSizeImage = Height*padWidth;

    // save
    fwrite (&bfSave,sizeof(bfSave),1,fp);
    fwrite (&Bmapinfo,sizeof(Bmapinfo),1,fp);
           

    for(indexx = 0;indexx < 256;indexx++)
    {
        outPalette[outIndex] = Pal[indexx].peBlue;
        outPalette[outIndex + 1] = Pal[indexx].peGreen;
        outPalette[outIndex + 2] = Pal[indexx].peRed;
        outPalette[outIndex + 3] = Pal[indexx].peFlags;
        outIndex += 4;
    }
    fwrite(outPalette, 1, 256*4, fp);

    // Here is the 8-bit padded data buffer - zeroed to start.
    padData8 = new UBYTE[Height*padWidth];
    for(indexx = 0;indexx < Height*padWidth;indexx++) padData8[indexx] = 0;

    // And copying in the 8bit data from the class
    for(indexy = 0;indexy < Height;indexy++)
    {
        for(indexx = 0;indexx < Width;indexx++)
        {
            padData8[indexy*padWidth + indexx] = Data8[indexy*Width + indexx];
        }
        for(padIndex = indexx;padIndex < padWidth;padIndex++)
        {
            padData8[indexy*padWidth + padIndex] = 0;
        }
    }
    
    // Write.
    fwrite(padData8, 1, Height*padWidth, fp);
    // Close.
    fclose(fp);
    // Cleanup.
    delete [] padData8;
    delete [] outPalette;
}
