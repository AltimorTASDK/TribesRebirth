//---------------------------------------------------------------------------
//
//
//  Filename   : imagefile.cpp
//
//
//  Description: the basic method functions of imagefile, all loading 
//               routines are in their respective io files. They are called
//               from the general Load and Save functions of the class.
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
//  Function Name: imageFile
//
//  Arguments    : none
//
//
//  Comments     : Constructor makes sure that all pointers start with
//                 NULL value.
//                 
//
//
//
//---------------------------------------------------------------------------

imageFile::imageFile(void)
{
    // NULL all pointer values.
    Data24      = NULL;
    Data8       = NULL;
    Filename    = NULL;
    OutPath     = NULL;

}


//---------------------------------------------------------------------------
//
//  Function Name: GetData24
//
//  Arguments    : none
//
//
//  Comments     : Returns the 24-bit Data pointer. 
//                 
//                 
//
//
//
//---------------------------------------------------------------------------

PALETTEENTRY *imageFile::GetData24(void)
{
    return Data24;
}


//---------------------------------------------------------------------------
//
//  Function Name: GetData8
//
//  Arguments    : none
//
//
//  Comments     : Returns the 8-bit Data pointer. 
//                 
//                 
//
//
//
//---------------------------------------------------------------------------

UBYTE *imageFile::GetData8(void)
{
    return Data8;
}


//---------------------------------------------------------------------------
//
//  Function Name: GetPalette
//
//  Arguments    : none
//
//
//  Comments     : Returns the PALETTEENTRY pointer to Pal.
//                 
//                 
//
//
//
//---------------------------------------------------------------------------

PALETTEENTRY *imageFile::GetPalette(void)
{
    return Pal;
}


//---------------------------------------------------------------------------
//
//  Function Name: SetFlags
//
//  Arguments    : a fileFlags pointer (the fileFlags struct is defined in pcatypes.h)
//
//
//  Comments     : the argument should be filled in the parser, and passed in. 
//                 This function allocates memory and copies the fileFlags 
//                 data members. 
//
//
//
//---------------------------------------------------------------------------

void imageFile::SetFlags(fileFlags *inFlags)
{
    COUNTER index;
    
    OutPath = new char[256];
    strcpy(OutPath,inFlags->inOutputPath);
    Filename = new char[FILENAMESIZE];
    for(index = 0; inFlags->inFilename[index] != 0; index++)
    {
        Filename[index] = inFlags->inFilename[index];
    }       
    Filename[index] = 0;
    Type = GetType();

    for(index = 0;index < 256;index++)
    {
        Pal[index].peRed   = 0;
        Pal[index].peBlue  = 0;
        Pal[index].peGreen = 0;
        Pal[index].peFlags = 0;
    }
}


//---------------------------------------------------------------------------
//
//  Function Name: ~imageFile
//
//  Arguments    : none
//
//
//  Comments     : Deletes the relevant data structures if they exist.
//                 
//                 
//
//
//
//---------------------------------------------------------------------------

imageFile::~imageFile(void)
{
    if(Data24)
        delete [] Data24;
    if(Data8)
        delete [] Data8;
    if(Filename)
        delete [] Filename;
    if(OutPath)
        delete [] OutPath;
}


//---------------------------------------------------------------------------
//
//  Function Name: GetType
//
//  Arguments    : none
//
//
//  Comments     : This should become more sophisticated, for
//                 now, it simply checks for .bmp, .tga, or .pal
//                 extension, if .bmp - it reads in the BITMAPINFOHEADER
//                 and checks the biBitCount data for bit info.
//
//
//---------------------------------------------------------------------------

TYPE imageFile::GetType(void)
{
    BITMAPINFOHEADER bmapinfo;
   
    if((strstr(Filename,".tga") != NULL) || (strstr(Filename,".TGA") != NULL))
    {
        return TGA32;
    }
    else if((strstr(Filename,".pal") != NULL) || (strstr(Filename,".PAL") != NULL))
    {
        return PAL;
    }
    else if((strstr(Filename,".tap") != NULL) || (strstr(Filename,".TAP") != NULL))
    {
        return PAL;
    }
    else if((strstr(Filename,".bmp") != NULL) || (strstr(Filename,".BMP") != NULL))
    {
        ReadBMPHead(&bmapinfo);
        if(bmapinfo.biBitCount == 8)
        {
            return BMP8;
        }
        else
        if(bmapinfo.biBitCount == 24)
        {
            return BMP24;
        }
    }
    return UNKNOWN;
}


//---------------------------------------------------------------------------
//
//  Function Name: ReadBMPHead
//
//  Arguments    : A pointer to BITMAPINFO - which holds the biBitCount
//                 data.
//
//  Comments     : opens the bitmap, reads the BITMAPINFO into the
//                 argument and closes.
//
//
//
//
//---------------------------------------------------------------------------

void imageFile::ReadBMPHead(BITMAPINFOHEADER *holdbmapinfo)
{
    FILE                *fp;
    BITMAPFILEHEADER    bfilehead;
    BITMAPINFOHEADER    lbmapinfo;

    if((fp = fopen(Filename,"rb")) == NULL)
    {
        printf("ERROR - Cannot open file : %s\n",Filename);
        AssertFatal(fp, avar("ERROR - Cannot open file : %s\n",Filename));
        exit(0);
    }
    
    fseek(fp, 0, SEEK_SET);
    fread(&bfilehead, sizeof(bfilehead),1,fp );
    fread(&lbmapinfo, sizeof(lbmapinfo), 1, fp);
    holdbmapinfo->biSize = lbmapinfo.biSize;
    holdbmapinfo->biWidth = lbmapinfo.biWidth;
    holdbmapinfo->biHeight = lbmapinfo.biHeight;
    holdbmapinfo->biPlanes = lbmapinfo.biPlanes;
    holdbmapinfo->biBitCount = lbmapinfo.biBitCount;
    holdbmapinfo->biCompression = lbmapinfo.biCompression;
    holdbmapinfo->biSizeImage = lbmapinfo.biSizeImage;

    fclose(fp);
}


//---------------------------------------------------------------------------
//
//  Function Name: SetType
//
//  Arguments    : TYPE inType - the new type
//                 
//
//  Comments     : Sets the type of an imageFile, this is used when creating
//                 a new imageFile instance that will be used to save data
//                 of a specific type (PAL, BMP8, etc.).
//                 
//
//
//---------------------------------------------------------------------------

void imageFile::SetType(TYPE inType)
{
    Type = inType;
}


//---------------------------------------------------------------------------
//
//  Function Name: SetFilename
//
//  Arguments    : char *inFilename - the new filename
//                 
//
//  Comments     : sets the private Filename string.
//                 
//                 
//                 
//
//
//---------------------------------------------------------------------------

void imageFile::SetFilename(char *inFilename)
{
    COUNTER index;
   if(!Filename) Filename = new char[256];
    for(index = 0;inFilename[index] != 0;index++)
    {
        Filename[index] = inFilename[index];
    }
    Filename[index] = 0;
}


//---------------------------------------------------------------------------
//
//  Function Name: Load
//
//  Arguments    : none
//                 
//
//  Comments     : calls the proper loader for the imageFile type.
//                 The particular loaders themselves are in the bmpio.cpp,
//                 tgaio.cpp and palio.cpp files. Note that GetType or SetType
//                 must have been called prior to this. This is most efficiently
//                 done by calling SetFlags, which calls GetType.
//
//---------------------------------------------------------------------------

void imageFile::Load(void)
{
    if(Type == BMP24)
    {
        LoadBMP24();
    }
    else

    if(Type == BMP8)
    {
        LoadBMP8();
    }
    else

    if(Type == TGA32)
    {
        LoadTGA32();
    }
    else

    if(Type == PAL)
    {
        LoadPAL();
    }
    else
    {
        printf("ERROR - Unknown file type : %s\n", Filename);
        exit(0);
    }
}
    
    
//---------------------------------------------------------------------------
//
//  Function Name: Save
//
//  Arguments    : char *saveFilename (becomes Filename if not specified)
//                 
//
//  Comments     : saves based on the imageFile type, if a file name is
//                 given, it sets the private Filename data to it and 
//                 proceeds (restoring at end), if no filename is given, it saves 
//                 to the file "Filename".
//
//
//---------------------------------------------------------------------------

void imageFile::Save(char *saveFilename)
{
    COUNTER index;
    char    *tempFilename = NULL; 
    if(saveFilename != NULL)
    {
        tempFilename = new char[FILENAMESIZE];
        for(index = 0;Filename[index] != 0;index++)
        {
            tempFilename[index] = Filename[index];
        }
        tempFilename[index] = 0;
     
        for(index = 0;saveFilename[index] != 0;index++)
        {
            Filename[index] = saveFilename[index];
        }
        Filename[index] = 0;
    }   

    if(Type == BMP24)
    {
        SaveBMP24();
    }
    else

    if(Type == BMP8)
    {
        // If the original 24-bit art was in a .tga file
        // (i.e. it was 32-bit alpha art), strip the .tga
        // and put in a .bmp extension.
        
        char *dotptr;
        dotptr = strstr(Filename,".tga");
		  if (!dotptr)
	        dotptr = strstr(Filename,".TGA");
        if(dotptr != NULL)
        {
            *(dotptr + 1) = 'b';
            *(dotptr + 2) = 'm';
            *(dotptr + 3) = 'p';
        }
        SaveBMP8();
    }
    else

    if(Type == TGA32)
    {
        SaveTGA32();
    }
    else

    if(Type == PAL)
    {
        SavePAL();
    }
    else
    {
        printf("ERROR - Cannot save file : %s - unknown type\n", Filename);
        exit(0);
    }

    if(tempFilename)
    {
        for(index = 0;tempFilename[index] != 0;index++)
        {
            Filename[index] = tempFilename[index];
        }
        Filename[index] = 0;
        delete [] tempFilename;
    }
}   


//---------------------------------------------------------------------------
//
//  Function Name: ReleaseData
//
//  Arguments    : none
//                 
//
//  Comments     : releases the 24 bit and 8 bit data,
//                 NULLS the pointers.
//                 
//                 
//
//
//---------------------------------------------------------------------------

void imageFile::ReleaseData(void)
{
    if(Data24)
    {
        delete [] Data24;
        Data24 = NULL;
    }
    if(Data8)
    {
        delete [] Data8;
        Data8 = NULL;
    }
}   


//---------------------------------------------------------------------------
//
//  Function Name: GetSize
//
//  Arguments    : none
//                 
//
//  Comments     : Returns the image area -> height*width.
//                 
//                 
//                 
//
//
//---------------------------------------------------------------------------

int imageFile::GetSize(void)
{
    int retVal;
    retVal = Height*Width;
    return retVal;
}


//---------------------------------------------------------------------------
//
//  Function Name: ReturnType
//
//  Arguments    : none
//                 
//
//  Comments     : Returns the type of the imageFile.
//                 
//                 
//                 
//
//
//---------------------------------------------------------------------------

TYPE imageFile::ReturnType(void)
{
    return Type;
}


//---------------------------------------------------------------------------
//
//  Function Name: ReturnPal
//
//  Arguments    : none
//                 
//
//  Comments     : Returns the Pal pointer.
//                 
//                 
//                 
//
//
//---------------------------------------------------------------------------

PALETTEENTRY *imageFile::ReturnPal(void)
{
    return Pal;
}


//---------------------------------------------------------------------------
//
//  Function Name: GetHeight
//
//  Arguments    : none
//                 
//
//  Comments     : Returns the height of the image.
//                 
//                 
//                 
//
//
//---------------------------------------------------------------------------

int imageFile::GetHeight(void)
{
    return Height;
}


//---------------------------------------------------------------------------
//
//  Function Name: GetWidth
//
//  Arguments    : none
//                 
//
//  Comments     : Returns the width of the image.
//                 
//                 
//                 
//
//
//---------------------------------------------------------------------------

int imageFile::GetWidth(void)
{
    return Width;
}


//---------------------------------------------------------------------------
//
//  Function Name: MakeSaveName
//
//  Arguments    : outLine contains an image file name w/ path
//                 
//
//  Comments     : Pulls off the path and inserts the OutputPath in
//                 front of the name.
//                 
//                 
//
//
//---------------------------------------------------------------------------

void imageFile::MakeSaveName(char *outLine)
{
    CHAR *oldLine  = new CHAR[256];
    CHAR *fileName = new CHAR[256];

    COUNTER index;
    COUNTER countSlash = 0;
    COUNTER index2 = 0;

    strcpy(oldLine, outLine);
    for(index = 0;oldLine[index] != 0; index++)
    {
        if(oldLine[index] == '\\') countSlash++;
    }
    if(countSlash == 0)
    {
        strcpy(fileName, oldLine);
    }
    else
    if(countSlash != 0)
    {
        for(index = 0;index2 != countSlash;index++)
        {
            if(oldLine[index] == '\\') index2++;
        }
        strcpy(fileName, oldLine + index);
    }
    
    for(index = 0;outLine[index] != 0;index++)
    {
        outLine[index] = 0;
    }
    strcpy(outLine, OutPath);
    strcat(outLine,"\\");
    strcat(outLine, fileName);

    delete [] oldLine;
    delete [] fileName;
}


//---------------------------------------------------------------------------
//
//  Function Name: SetPalette
//
//  Arguments    : inPalette - sets the internal palette to this
//                 
//
//  Comments     : A straight copy. This functions should be used
//                 for saving the PCA palette, and also plugging in
//                 the palette before rendering.
//                 
//
//
//---------------------------------------------------------------------------

void imageFile::SetPalette(PALETTEENTRY *inPalette)
{
    COUNTER index;

    for(index = 0;index < 256;index++)
    {
        Pal[index].peRed = inPalette[index].peRed;
        Pal[index].peGreen = inPalette[index].peGreen;
        Pal[index].peBlue = inPalette[index].peBlue;
        Pal[index].peFlags = inPalette[index].peFlags;
    }
}


//---------------------------------------------------------------------------
//
//  Function Name: GetZeroColor
//
//  Arguments    : ZeroColor pointer to be filled with the zero color.
//                 
//
//  Comments     : The zerocolor for a bitmap is the upper left hand
//                 corner pixel by default. This can be over-ridden
//                 in the parse file with the keyword ZEROCOLOR. 
//                 This function checks the ul pixel and fills in
//                 ZeroColor with its value. Returns True if the 
//                 Data24 ptr is non-NULL i.e. if the bitmap is loaded.
//---------------------------------------------------------------------------

FLAG imageFile::GetZeroColor(PALETTEENTRY *ZeroColor)
{
    if(Data24)
    {
        ZeroColor->peRed   = Data24[Width*Height - Width].peRed;
        ZeroColor->peGreen = Data24[Width*Height - Width].peGreen;
        ZeroColor->peBlue  = Data24[Width*Height - Width].peBlue;
        ZeroColor->peFlags = Data24[Width*Height - Width].peFlags;
        return True;
    }
    else
        return False;
}
