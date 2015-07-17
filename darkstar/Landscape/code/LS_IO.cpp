//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//北   
//北  $Workfile:   LS_IO.cpp  $
//北  $Version$
//北  $Revision:   1.00  $
//北    
//北  DESCRIPTION:
//北        LandScape IO Methods
//北        
//北  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//北 
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

#include <g_bitmap.h>
#include <simResource.h>

#include "ls.h"      // base class defined

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

bool  LandScape::load( char *_fileName, float maxH ) // if maxH<0, then scaled between 0 and 1
{
	if (rm)
	{
		Resource<GFXBitmap> rBM;
		rBM = rm->load(_fileName, true);
		if (!(bool)rBM)
      {
         sprintf( mExecString, "LOAD: failed to load: '%s'", _fileName );
         mExecStatus = EXEC_FAILURE;
         return false;
      }

		pushBM( rBM,maxH );

		return true;
		// resource manager deals with bitmap when 'rBM' goes out of scope
	}

	// if no resource manager...
   GFXBitmap   *pBM = GFXBitmap::load(_fileName);
	if ( !pBM )
      return FALSE;

   pushBM( pBM,maxH );

   delete pBM; // no resource manager, we need to delete it ourself

   return TRUE;
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

bool  LandScape::save( char *_fileName )
{
   if ( !stack.size() )
   {
      sprintf( mExecString, "SAVE: failed to save: '%s' - stack empty", _fileName );
      mExecStatus = EXEC_FAILURE;
      return FALSE;
   }

   GFXBitmap *pBM = get( 1 );

   if ( !pBM )
   {
      sprintf( mExecString, "SAVE: failed to save: '%s'", _fileName );
      mExecStatus = EXEC_FAILURE;
      return FALSE;
   }

   bool result = !pBM->write( _fileName );

   delete pBM;

   if( !result )
   {
      sprintf( mExecString, "SAVE: failed to save: '%s'", _fileName );
      mExecStatus = EXEC_FAILURE;
   }

   return result;
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//北  Code for Script Parsing and I/O
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

#define MAX_ARGUMENTS   4

struct   LS_OPCODES
{
   char        *name;
   LS_OPCODE   opCode;
   int         numArgs;
   int         argType[MAX_ARGUMENTS];
};

enum  LS_TYPES
{
   LS_ARG_NONE,
   LS_ARG_CHAR,
   LS_ARG_INT,
   LS_ARG_FLOAT,
};

LS_OPCODES   LSOpCode[] = 
{
   {"SEED",      LS_SEED,      1, {LS_ARG_INT} },
   {"LOAD",      LS_LOAD,      1, {LS_ARG_CHAR} },
   {"LOADM",     LS_LOADM,     2, {LS_ARG_CHAR,LS_ARG_FLOAT} },
   {"SAVE",      LS_SAVE,      1, {LS_ARG_CHAR} },
   {"CLEAR",     LS_CLEAR,     0, {LS_ARG_NONE} },
   {"PUSH",      LS_PUSH,      1, {LS_ARG_INT}  },
   {"POP",       LS_POP,       0, {LS_ARG_NONE} },
   {"GET",       LS_GET,       1, {LS_ARG_INT} },
   {"SWAP",      LS_SWAP,      0, {LS_ARG_NONE} },
   {"DUP",       LS_DUP,       0, {LS_ARG_NONE} },
   {"ADD",       LS_ADD,       1, {LS_ARG_FLOAT} },
   {"SUB",       LS_SUB,       1, {LS_ARG_FLOAT} },
   {"MUL",       LS_MUL,       1, {LS_ARG_FLOAT} },
   {"DIV",       LS_DIV,       1, {LS_ARG_FLOAT} },
   {"EXP",       LS_EXP,       1, {LS_ARG_FLOAT} },
   {"NEG",       LS_NEG,       0, {LS_ARG_NONE} },
   {"CLR",       LS_CLR,       1, {LS_ARG_FLOAT} },
   {"DIFF",      LS_DIFF,      0, {LS_ARG_NONE} },
   {"NORMALIZE", LS_NORMALIZE, 2, {LS_ARG_FLOAT, LS_ARG_FLOAT} },
   {"ADD_STACK", LS_ADDS,      1, {LS_ARG_INT} },
   {"SUB_STACK", LS_SUBS,      1, {LS_ARG_INT} },
   {"TERRAIN",   LS_TERRAIN,   2, {LS_ARG_INT, LS_ARG_FLOAT} },
   {"PLASMA",    LS_PLASMA,    2, {LS_ARG_INT, LS_ARG_FLOAT} },
   {"CRATER",    LS_CRATER,    0, {LS_ARG_NONE} },
   {"PEAK",      LS_PEAK,      0, {LS_ARG_NONE} },
   {"ROT",       LS_ROT,       0, {LS_ARG_NONE} },
   {"RING",      LS_RING,      0, {LS_ARG_NONE} },
   {"FILLBASIN", LS_FILLBASIN, 0, {LS_ARG_NONE} },
   {"OVERLAY",   LS_OVERLAY,   2, {LS_ARG_INT, LS_ARG_FLOAT} },
   {"BLEND",     LS_BLEND,     2, {LS_ARG_INT, LS_ARG_INT} },
   {"FLIPX",     LS_FLIPX,     0, {LS_ARG_NONE} },
   {"FLIPY",     LS_FLIPY,     0, {LS_ARG_NONE} },
   {"LPFILTER",  LS_LPFILTER,  0, {LS_ARG_NONE} },
   {"HPFILTER",  LS_HPFILTER,  0, {LS_ARG_NONE} },
   {"BPFILTER",  LS_BPFILTER,  0, {LS_ARG_NONE} },
   {"BRFILTER",  LS_BRFILTER,  0, {LS_ARG_NONE} },
   {"FFLP",      LS_FFLP,      0, {LS_ARG_NONE} },
   {"FFHP",      LS_FFHP,      0, {LS_ARG_NONE} },
   {"FFBP",      LS_FFBP,      0, {LS_ARG_NONE} },
   {"FFBR",      LS_FFBR,      0, {LS_ARG_NONE} },
   {"FFT",       LS_FFT,       1, {LS_ARG_INT} },
   {"SIZE",      LS_SIZE,      1, {LS_ARG_INT} },
   {"SMOOTH",    LS_SMOOTH,    2, {LS_ARG_FLOAT, LS_ARG_FLOAT} },
   {"SLOPE",     LS_SLOPE,     2, {LS_ARG_FLOAT, LS_ARG_INT} },
   {"SHAVE",	  LS_SHAVE,		 3, {LS_ARG_FLOAT, LS_ARG_FLOAT, LS_ARG_FLOAT} },
   {"CURVE",     LS_CURVE,     2, {LS_ARG_FLOAT, LS_ARG_INT} },
   {"FLOOR",     LS_FLOOR,     3, {LS_ARG_FLOAT, LS_ARG_FLOAT, LS_ARG_FLOAT} },
   {"CEIL",      LS_CEIL,      3, {LS_ARG_FLOAT, LS_ARG_FLOAT, LS_ARG_FLOAT} },
   {"FILL_N",    LS_FILL_N,    1, {LS_ARG_FLOAT} },
   {"ALPHABLEND", LS_ALPHABLEND, 2, {LS_ARG_INT, LS_ARG_INT} },
   {"TILE",      LS_TILE,      0, {LS_ARG_NONE} },
   {"WRAP",      LS_WRAP,      0, {LS_ARG_NONE} },
   {"MASK",      LS_MASK,      2, {LS_ARG_INT, LS_ARG_FLOAT} },
   {"CLAMP",     LS_CLAMP,     2, {LS_ARG_INT, LS_ARG_FLOAT} },
   {"CLIPMIN",   LS_CLIPMIN,   1, {LS_ARG_FLOAT} },
   {"CLIPMAX",   LS_CLIPMAX,   1, {LS_ARG_FLOAT} },
   {NULL,(LS_OPCODE)NULL,NULL,NULL}
};

void LandScape::parseScript( char *_buff, bool clearStack )
{
    // remove comments from the buffer
    char *targetBuff  = new char[strlen(_buff) + 1];
    char *source      = _buff;
    char *target      = targetBuff;

    while (*source)
    {
        // does a comment start here?
        if (strncmp(source, "//", 2) == 0)
        {
            // eat chars until the end of the line or end of the string
            while (*source)
            {
                if (*source == 0x0A) { source++; break; }
                source++;
            }
            if (*source == '\0') break;
            else continue;
        }
        *target++ = *source++;
    }
    *target = '\0';
    strcpy(_buff, targetBuff);
    delete [] targetBuff;
   

   char     tokens[] = " ,\x09\x0D\x0A";
   char     *pChr = strtok( _buff, tokens );
   char     *arguments[MAX_ARGUMENTS*8];

   int      opIndex;
   int      line=1;

   // zap the matrix stack before executing the script
   if(clearStack)
      clear();

   while ( pChr )
   {
      // first identify the opcode from the list above
      for ( opIndex=0; LSOpCode[opIndex].name && strcmpi( pChr, LSOpCode[opIndex].name ); opIndex++ );

      if ( LSOpCode[opIndex].name )  // if name was found
      {
         // now push the arguments onto our argument list
         void     *pArg = arguments;
         for ( int p=0; p<LSOpCode[opIndex].numArgs; p++ )
         {
            pChr = strtok( NULL, tokens );
            if (!pChr)
            {
                 char  tmp[256];
                 wsprintf( tmp, "Could not find argument %d in line: %i", p+1, line );
                 MessageBox( HWND_DESKTOP, tmp, "Syntax Error", MB_OK );
                 clear();
                 return;
            }
            switch( LSOpCode[opIndex].argType[p] )
            {
               case LS_ARG_INT:
                  *(int*)pArg = atoi( pChr );
                  pArg = ((char*)pArg) + 4;
                  break;

               case LS_ARG_FLOAT:
                  *(double*)pArg = atof( pChr );
                  pArg = ((char*)pArg) + 8;
                  break;

               case LS_ARG_CHAR:
                  *(char**)pArg = &pChr[0];
                  pArg = ((char*)pArg) + 4;
                  break;

               case LS_ARG_NONE:
                  break;
            }
         }
         // now exec the opcode with the arguments
         execV( LSOpCode[opIndex].opCode, (va_list)arguments );
         
         // check the exec_status from this function
         if( mExecStatus == EXEC_FAILURE )
         {
            char tmp[512];
            sprintf(tmp, "   Failed to execute command on line: %i\n   %s\n\nDo you wish to continue?", line, mExecString );
            if( MessageBox(HWND_DESKTOP,tmp,"Exec failure",MB_YESNO) == IDNO )
            {
               clear();
               return;
            }
         }
         
         pChr = strtok( NULL, tokens );
         line++;
       }
      else
      {
         char  tmp[256];
         wsprintf( tmp, "Could not find OpCode: %s in line: %i", pChr, line );
         MessageBox( HWND_DESKTOP, tmp, "Syntax Error", MB_OK );
         clear();
         return;         
      }
   }
}

