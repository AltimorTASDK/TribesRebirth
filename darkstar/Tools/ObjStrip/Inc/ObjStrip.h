//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//北   
//北  $Workfile:   ObjStrip.h  $
//北  $Version$
//北  $Author:     Robert Mobbs
//北  $Revision:   1.00  $
//北    
//北  DESCRIPTION:
//北        Utility to decompose a Borland .OBJ file
//北        Its main function is to seperate the DATA segment into
//北        a seperate file.  Secondary function is to analyze and
//北        display components of the .obj file.
//北        
//北  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//北 
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

#ifdef __BORLANDC__
#pragma option -a1
#endif
#ifdef _MSC_VER
#pragma pack(push,1)
#endif

enum OMF_TYPES
{
   OMF_TYPE_THEADR  = 0x80,
   OMF_TYPE_LHEADR  = 0x82,
   OMF_TYPE_COMENT  = 0x88,
   OMF_TYPE_LNAMES  = 0x96,
   OMF_TYPE_SEGDEF  = 0x98, 
   OMF_TYPE_GRPDEF  = 0x9A,
   OMF_TYPE_PUBDEF  = 0x90,
   OMF_TYPE_LEDATA  = 0xA0,
   OMF_TYPE_MODE32  = 0x8B,
};

enum COMENT_TYPES
{
    COMENT_TYPE_TRANSLATOR = 0x00,
    COMENT_TYPE_SOURCEFILE = 0xE8,
    COMENT_TYPE_DEPENDFILE = 0xE9,
};

typedef struct tagCOMENT
{
   BYTE     flag;       // 80-Purge(NO), 40-List(NO)
   BYTE     Class;      // 00-Translator, E8-SourceFile, E9-DependencyFile
} COMENT;

typedef struct tagPUBDEF
{
   BYTE     seg;        // what SEGDEF does this pubdef belong to?
   BYTE     undef;
   BYTE     size;       // max size is 251
} PUBDEF;

typedef struct tagLEDATA
{
   BYTE     segdef;     // SEGDEF index
   WORD     offset;     
} LEDATA;

typedef struct tagOMFHEADER
{
   BYTE  type;
   WORD  size;
} OMFHEADER;

typedef struct tagCOFFHEADER
{
    unsigned short f_magic;     // magic number
    unsigned short f_nscns;     // number of sections
    long           f_timdat;    // time and date stamp
    long           f_symptr;    // file ptr to symbol table
    long           f_nsyms;     // number of entries in symbol table
    unsigned short f_opthdr;    // size of optional header
    unsigned short f_flags;     // flags
} COFFHEADER;

typedef struct tagSECTIONHEADER
{
    char           s_name[8];   // section name
    long int       s_paddr;     // physical address
    long int       s_vaddr;     // virtual address
    long int       s_size;      // section size
    long int       s_scnptr;    // file ptr to section raw data
    long int       s_relptr;    // file ptr to relocation info
    long int       s_lnnoptr;   // file ptr to line number
    unsigned short s_nreloc;    // number of relocation entries
    unsigned short s_nlnno;     // number of line number entries
    long int       s_flags;     // flags
} SECTIONHEADER;

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
// a couple of globals
int  strip(char *, char *, BOOL);
