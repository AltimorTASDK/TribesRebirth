#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef MSVC
#include <dir.h>
#else
#include <direct.h>
#endif
#include <string.h>
#include "pcatypes.h"
#include "pcaparse.h"
#include "svector.h"
#include "pcaquan.h"
#include "parsfuncs.h"


//---------------------------------------------------------------------------
//
//  The following are all the key word functions, i.e. the functions 
//  associated with each keyword in the KeyWordTable. They have been
//  registered with the parser, and they all take in a pointer to an
//  allFlags struct.
//
//  
//  
//
//  
//  
//  
//  
//
//---------------------------------------------------------------------------

FLAG rgbdistance(void *inVoidFlags, char * /*line*/ , int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    inFlags[inFlagIndex].inDistanceType = RGB;
    return False;
}


FLAG dithertolerance(void *inVoidFlags, char *line, int inFlagIndex)
{
    float tolValue;
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;


    if(sscanf(line,"DITHERTOLERANCE(%f)",&tolValue) != 1)
    {
        printf("ERROR - Correct syntax is DITHERTOLERANCE(x)\n");
        exit(0);
    }
    CheckParenClosed(line);

    // If weighted RGB distance, set the tolerance to 
    // 6-3-1 times the squared values.
    if(inFlags[inFlagIndex].inDistanceType == RGBW)
    {
       inFlags[inFlagIndex].inTolerance = tolValue*tolValue*6
                                         + tolValue*tolValue*3
                                         + tolValue*tolValue;
    }
    else
    
    if(inFlags[inFlagIndex].inDistanceType == RGB)
    {
       inFlags[inFlagIndex].inTolerance = 3*tolValue*tolValue;
    }
    return False;
}
    
FLAG shadelevels(void *inVoidFlags, char *line, int inFlagIndex)
{
    COUNTER shadeLevs;
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;

    if(sscanf(line,"SHADELEVELS(%d)",&shadeLevs) != 1)
    {
        printf("ERROR - Correct syntax is SHADELEVELS(x)\n");
        exit(0);
    }
    CheckParenClosed(line);
    inFlags[inFlagIndex].inShadeLevels = shadeLevs;
    return False;

}

FLAG hazelevels(void *inVoidFlags, char *line, int inFlagIndex)
{
    COUNTER hazeLevs;
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;

    if(sscanf(line,"HAZELEVELS(%d)",&hazeLevs) != 1)
    {
        printf("ERROR - Correct syntax is HAZELEVELS(x)\n");
        exit(0);
    }
    CheckParenClosed(line);

    inFlags[inFlagIndex].inHazeLevels = hazeLevs;
    return False;
}


FLAG weightedrgbdistance(void *inVoidFlags, char * /*line*/ , int inFlagIndex)
{
   allFlags *inFlags;
   inFlags = (allFlags *)inVoidFlags;
   
   inFlags[inFlagIndex].inDistanceType = RGBW;
   return False;
}


FLAG luvdistance(void *inVoidFlags, char * /*line*/ , int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    inFlags[inFlagIndex].inDistanceType = LUV;
    return False;
}


FLAG nochoose(void *inVoidFlags, char * /*line*/ , int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    inFlags[inFlagIndex].inChooseFlag = False;
    return False;
}


FLAG norender(void *inVoidFlags, char * /*line*/ , int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    inFlags[inFlagIndex].inRenderFlag = FALSE;
    return False;
}


FLAG ditherflag(void *inVoidFlags, char * /*line*/ , int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    inFlags[inFlagIndex].inDitherFlag = True;
    return False;
}
        

FLAG nodither(void *inVoidFlags, char * /*line*/ , int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    inFlags[inFlagIndex].inDitherFlag = False;
    return False;
}
    

FLAG quantizergb(void *inVoidFlags, char * /*line*/ , int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    inFlags[inFlagIndex].inQuantizeFlag = RGB;
    return False;
}


FLAG quantizealpha(void *inVoidFlags, char * /*line*/ , int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    inFlags[inFlagIndex].inQuantizeFlag = ALPHA;
    return False;
}


FLAG render(void *inVoidFlags, char *line, int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    int first;
    int last;
    inFlags[inFlagIndex].inRenderFlag = True;
    
    if(sscanf(line,"RENDER(%d,%d)",&first, &last) != 2)
    {
        printf("ERROR - Correct syntax is RENDER(x,y)\n");
        exit(0);
    }
    
    CheckParenClosed(line);
    
    inFlags[inFlagIndex].inRenderRangeFirst = first;
    inFlags[inFlagIndex].inRenderRangeLast = last;
    return False;
}


FLAG choose(void *inVoidFlags, char *line, int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    int first;
    int last;
    if(sscanf(line,"CHOOSE(%d,%d)",&first, &last) != 2)
    {
        printf("ERROR - Correct syntax is CHOOSE(x,y)\n");
        exit(0);
    }
    CheckParenClosed(line);

    inFlags[inFlagIndex].inPaletteRangeFirst = first;
    inFlags[inFlagIndex].inPaletteRangeLast = last;
    return False;
}


FLAG blendrange(void *inVoidFlags, char *line, int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    int first;
    int last;
    if(sscanf(line,"BLENDRANGE(%d,%d)",&first, &last) != 2)
    {
        printf("ERROR - Correct syntax is BLENDRANGE(x,y)\n");
        exit(0);
    }
    CheckParenClosed(line);

    inFlags[inFlagIndex].inBlendFirst = first;
    inFlags[inFlagIndex].inBlendLast = last;
    return False;
}


FLAG noshade(void *inVoidFlags, char * /*line*/ , int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    if(inFlags[inFlagIndex].inShadeTowards)
        delete inFlags[inFlagIndex].inShadeTowards;
    inFlags[inFlagIndex].inShadeTowards = NULL;
    inFlags[inFlagIndex].inShadeIntensity = 0;
    return False;
}
    

FLAG shade(void *inVoidFlags, char *line, int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    int r,g,b;
    int intensity;
    inFlags[inFlagIndex].inShadeTowards = new PALETTEENTRY;
    if(sscanf(line,"SHADE(%d,%d,%d,%d)", &r,&g,&b,&intensity) != 4)
    {
        printf("ERROR - Correct Syntax is SHADE(r,g,b,intensity)\n");
        exit(0);
    }
    CheckParenClosed(line);

    inFlags[inFlagIndex].inShadeTowards->peRed = (UBYTE)(r);
    inFlags[inFlagIndex].inShadeTowards->peGreen = (UBYTE)(g);
    inFlags[inFlagIndex].inShadeTowards->peBlue = (UBYTE)(b);
    inFlags[inFlagIndex].inShadeIntensity = (float)(intensity)/100.0f;
    return False;
}
    

FLAG nohaze(void *inVoidFlags, char * /*line*/ , int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    if(inFlags[inFlagIndex].inHazeTowards)
        delete inFlags[inFlagIndex].inHazeTowards;
    inFlags[inFlagIndex].inHazeTowards = NULL;
    inFlags[inFlagIndex].inHazeIntensity = 0;
    return False;
}
    

FLAG haze(void *inVoidFlags, char *line, int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    int r,g,b;
    int intensity;
    inFlags[inFlagIndex].inHazeTowards = new PALETTEENTRY;

    if(sscanf(line,"HAZE(%d,%d,%d,%d)", &r,&g,&b,&intensity) != 4)
    {
        printf("ERROR - Correct Syntax is HAZE(r,g,b,intensity)\n");
        exit(0);
    }
    CheckParenClosed(line);

    inFlags[inFlagIndex].inHazeTowards->peRed = (UBYTE)(r);
    inFlags[inFlagIndex].inHazeTowards->peGreen = (UBYTE)(g);
    inFlags[inFlagIndex].inHazeTowards->peBlue = (UBYTE)(b);
    inFlags[inFlagIndex].inHazeIntensity = (float)(intensity)/100.0f;
    return False;
}


FLAG paloutputname(void *inVoidFlags, char *line, int inFlagIndex)
{    
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    strcpy(inFlags[inFlagIndex].inPalOutput, line + 14);
    *(strchr(inFlags[inFlagIndex].inPalOutput,0) - 1) = 0;
    CheckParenClosed(line);

    return False;
}



FLAG noweight(void *inVoidFlags, char * /*line*/ , int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    inFlags[inFlagIndex].weightFlag = False;
    return False;
}


FLAG palname(void *inVoidFlags, char *line, int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    inFlags[inFlagIndex].inFilename = new char[256];
    strcpy(inFlags[inFlagIndex].inFilename, line+8);
    *(strchr(inFlags[inFlagIndex].inFilename,0) - 1) = 0;     // get rid of trailing )
    CheckParenClosed(line);

    return True;
}


FLAG outpath(void *inVoidFlags, char *line, int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    strcpy(inFlags[inFlagIndex].inOutputPath, (line+8));     // get rid of leading (
    *(strchr(inFlags[inFlagIndex].inOutputPath,0) - 1) = 0;  // get rid of trailing )

    CHAR *currentdir = new CHAR[256];
    getcwd(currentdir,256);
    if(chdir(inFlags[inFlagIndex].inOutputPath) == -1)       // if path not exist
    {
        mkdir(inFlags[inFlagIndex].inOutputPath);            // make it.
    }
    else                                        // else move back.
    {
        chdir(currentdir);
    }
    CheckParenClosed(line);

    return False;
}
    

FLAG zerocolor(void *inVoidFlags, char *line, int inFlagIndex)
{
    allFlags *inFlags;
    int fieldCount;
    inFlags = (allFlags *)inVoidFlags;
    
    int r,g,b;
    inFlags[inFlagIndex].inZeroColor = new PALETTEENTRY;

    // If used as a function - as below, then set a specific zero color
    fieldCount = sscanf(line,"ZEROCOLOR(%d,%d,%d)", &r, &g, &b);
    if(fieldCount == 3)
    {
        inFlags[inFlagIndex].inZeroColor->peRed   = (UBYTE)(r);
        inFlags[inFlagIndex].inZeroColor->peGreen = (UBYTE)(g);
        inFlags[inFlagIndex].inZeroColor->peBlue  = (UBYTE)(b);
        inFlags[inFlagIndex].inZeroFlag           = True;
        CheckParenClosed(line);
    }
    // If used as a flag, then use upper left hand corner - to check for
    // flag behavior, look right after the "R", if it's the end of the token list
    // or a return, then it's a flag.
    else
    if((line[9] == '\n') || (line[9] == 0))
    {
        if(inFlags[inFlagIndex].inZeroColor)
            delete inFlags[inFlagIndex].inZeroColor;
        inFlags[inFlagIndex].inZeroColor = NULL;
        inFlags[inFlagIndex].inZeroFlag = True;
    }
    else
    {
        printf("ERROR - Correct Syntax is ZEROCOLOR(r,g,b)");
        exit(0);
    }

    return False;
}

    
FLAG nozerocolor(void *inVoidFlags, char * /*line*/ , int inFlagIndex)
{
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;

    if(inFlags[inFlagIndex].inZeroColor)
        delete inFlags[inFlagIndex].inZeroColor;

    inFlags[inFlagIndex].inZeroColor = NULL;
    inFlags[inFlagIndex].inZeroFlag  = False;
    return False;
}


FLAG ignorecolors(void *inVoidFlags, char *line, int inFlagIndex)
{    
    allFlags *inFlags;
    inFlags = (allFlags *)inVoidFlags;
    
    int num = 0;
    int index;
    int r,g,b;
    int offset = 13;

    for(index = 0;line[index] != ')';index++)   // count list
    {
        if(line[index] == ';') num++;
    }
  // add one - one less partition than elts.
    num++;
    inFlags[inFlagIndex].numIgnores = num;
    inFlags[inFlagIndex].inIgnoreList = new PALETTEENTRY[num];
    for(index=0;index < num;index++)
    {
        if(sscanf(line+offset,"%d,%d,%d%[*]c",&r,&g,&b) != 3)
        {
            printf("ERROR - Correct syntax is IGNORECOLORS(r1,g1,b1;r2,g2,b2;...;rn,gn,bn)\n");
            exit(0);
        }

        if(index != (num-1))
            for(offset = offset; line[offset] != ';'; offset++);
        offset++;                               // to get past the next ;
        inFlags[inFlagIndex].inIgnoreList[index].peRed = (UBYTE) r;
        inFlags[inFlagIndex].inIgnoreList[index].peGreen = (UBYTE) g;
        inFlags[inFlagIndex].inIgnoreList[index].peBlue = (UBYTE) b;
    }
    CheckParenClosed(line);
    return False;
}


void CheckParenClosed(char *line)
{
    COUNTER index;
    COUNTER openParen = 0;
    COUNTER closedParen = 0;

    for(index = 0;line[index] != 0;index++)
    {
        if(line[index] == '(')
            openParen++;
        if(line[index] == ')')
            closedParen++;
    }
    if(openParen != closedParen)
    {
        printf("ERROR - open parentheses on line : %s\n",line);
        exit(0);
    }           

}