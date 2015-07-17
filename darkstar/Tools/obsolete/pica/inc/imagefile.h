//---------------------------------------------------------------------------
//
//
//  Filename   : imagefile.h
//
//
//  Description: defines the imagefile class, which controls the loading/saving
//               of palettes, bmps, tga's, etc. It also keeps track of the
//               naming of these files, and can be used to create output names
//               given an output path (strips original path, and prepends the
//               output).
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


#ifndef _IMAGEFILE_H
#define _IMAGEFILE_H

class imageFile
{
    private:
    
    TYPE Type;
    
    PALETTEENTRY Pal[256];
    PALETTEENTRY FixedPal[256];
    PALETTEENTRY *Data24;
    UBYTE        *Data8;
    
    char *Filename;
    
    COUNTER  Height;
    COUNTER  Width;
    COUNTER  ImageSize;
    
    TYPE GetType(void);
    void ReadBMPHead(BITMAPINFOHEADER *);
    char *OutPath;
    BITMAPINFOHEADER Bmapinfo;
    BITMAPFILEHEADER Bmapfhead;

    void LoadBMP24(void);
    void LoadBMP8(void);
    void LoadTGA32(void);

    void LoadPAL(void);
    void LoadTextAlphaPAL(void);
    
    void SaveBMP24(void);
    void SaveBMP8(void);
    void SaveTGA32(void);
    void SavePAL(void);
    
    public:
    
    imageFile(void);
    ~imageFile(void);
    void SetFlags(fileFlags *);
    void Load(void);
    void Save(char *saveFilename = NULL);
    void ReleaseData(void);
    PALETTEENTRY *GetData24(void);
    UBYTE *GetData8(void);
    int GetSize(void);
    TYPE ReturnType(void);
    PALETTEENTRY *ReturnPal(void);
    void MakeSaveName(char *);
    void SetType(TYPE);
    void SetFilename(char *);
    void SetPalette(PALETTEENTRY *);
    PALETTEENTRY *GetPalette(void);
    int GetHeight(void);
    int GetWidth(void);
    FLAG GetZeroColor(PALETTEENTRY *ZeroColor);
    
};

#endif //_IMAGEFILE_H
    
    
    
    