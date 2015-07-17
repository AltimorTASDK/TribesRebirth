//---------------------------------------------------------------------------
//
//
//  Filename   : pcatypes.h
//
//
//  Description: The types needed for the pica program
//                
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

#ifndef _PCATYPE_H
#define _PCATYPE_H

typedef unsigned char UBYTE;
typedef unsigned long ULONG;
typedef char          FLAG;
typedef char          TYPE;
typedef unsigned int           COUNTER;
typedef signed short  SSHORT;
typedef signed long   SLONG;

const FLAG True = 1;
const FLAG False = 0;

const TYPE BMP24 = 1;
const TYPE BMP8  = 2;
const TYPE TGA32 = 3;
const TYPE PAL   = 4;
const TYPE UNKNOWN = 5;

const TYPE ALPHA = 0;
const TYPE RGB = 1;
const TYPE LUV = 2;
const TYPE RGBW = 3;

const ULONG ALPHASPACESIZE = 1048576;
const ULONG COLORSPACESIZE = 2097152;
const COUNTER BLENDOFF     = 999;          

const float MAXHAZELEVELS = 4.0;
const float MAXSHADELEVELS = 4.0;

const int FILENAMESIZE = 256;

// This structure is passed from the parser to the
// imageFile class, it is the argument for the constructor
// that is why all of the data members are prepended with "in"

struct allFlags
{
    FLAG inDitherFlag;
    FLAG inQuantizeFlag;
    FLAG inRenderFlag;
    FLAG weightFlag;
    COUNTER inRenderRangeFirst;
    COUNTER inRenderRangeLast;
    COUNTER inPaletteRangeFirst;
    COUNTER inPaletteRangeLast;
    PALETTEENTRY *inShadeTowards;
    float inShadeIntensity;
    PALETTEENTRY *inHazeTowards;
    float inHazeIntensity;
    char *inFilename;
    char *inOutputPath;
    PALETTEENTRY *inIgnoreList;
    COUNTER numIgnores;
    PALETTEENTRY *inZeroColor;
    COUNTER inWeight;
    char *inPalOutput;
    FLAG inChooseFlag;
    TYPE inDistanceType;
    float inTolerance;
    FLAG inZeroFlag;
    COUNTER inBlendFirst;
    COUNTER inBlendLast;
    COUNTER inShadeLevels;
    COUNTER inHazeLevels;
};

struct renderFlags
{
    FLAG inDitherFlag;
    int inRenderRangeFirst;
    int inRenderRangeLast;
    PALETTEENTRY *inZeroColor;
    int inHeight;
    int inWidth;
    float inTolerance;
};

struct quantizeFlags
{
    PALETTEENTRY *inIgnoreList;
    COUNTER          numIgnores;
    PALETTEENTRY *inZeroColor;    
};    

struct fileFlags
{
    char *inFilename;
    char *inOutputPath;
};    



// None of the following are used within the quantizer or remapper,
// they are only used by the saving functions to correctly write, all
// internal data is stored as PALETTEENTRY's

// For 24-bit data saving
struct BGRDATA
{
    UBYTE peBlue;
    UBYTE peGreen;
    UBYTE peRed;
};

// The RIFFHDR struct for saving palettes
struct RIFFHDR
{
   char  hdr[4];
   int   fsize;
   char  typ[4];
   char  dta[4];
   int   dsize;
   int   version;
};

struct FLOATCOLOR
{
    float R;
    float G;
    float B;
};


#endif // _PCATYPE_H

