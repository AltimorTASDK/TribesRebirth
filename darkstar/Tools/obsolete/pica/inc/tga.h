//---------------------------------------------------------------------------
//
//
//  Filename   : tga.h
//
//
//  Description: structs needed for Dave McClurg's targa reader.
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

#ifndef _TGA_H
#define _TGA_H

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#define CHAR signed char // CHAR already typedefed in winnt.h
typedef signed short SHORT;
typedef signed long LONG;

/*
**  TGA File Header
*/
typedef struct _TGAHEADER
{                           
    BYTE   IdLength;            /* Image ID Field Length      */
    BYTE   CmapType;            /* Color Map Type             */
    BYTE   ImageType;           /* Image Type                 */
    /*
    ** Color Map Specification
    */
    WORD   CmapIndex;           /* First Entry Index          */
    WORD   CmapLength;          /* Color Map Length           */
    BYTE   CmapEntrySize;       /* Color Map Entry Size       */
    /*
    ** Image Specification
    */
    WORD   X_Origin;            /* X-origin of Image          */
    WORD   Y_Origin;            /* Y-origin of Image          */
    WORD   ImageWidth;          /* Image Width                */
    WORD   ImageHeight;         /* Image Height               */
    BYTE   PixelDepth;          /* Pixel Depth                */
    BYTE   ImagDesc;            /* Image Descriptor           */
} TGAHEADER;


typedef struct _TGACMAP
{
   CHAR*    IdField;
   BYTE*    CmapData;
} TGACMAP;
              

typedef struct _TGA
{
   TGAHEADER   Head;
   TGACMAP     Cmap;
   BYTE*       Data;
} TGA;


void LoadTGA ( char* fName, TGA* tga );
void FreeTGA ( TGA* tga );

#endif //_TGA_H