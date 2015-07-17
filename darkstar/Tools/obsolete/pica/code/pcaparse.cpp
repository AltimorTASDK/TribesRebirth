//---------------------------------------------------------------------------
//
//
//  Filename   : pcapars.cpp
//
//
//  Description: The cons/destructor and functions of the parse class.
//               There's nothing fancy here, in the end, it has to do
//               what all parsers do, go down a big big list!
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
#ifndef MSVC
#include <dir.h>
#endif
#include <string.h>
#include <base.h>
#include "pcatypes.h"
#include "pcaparse.h"
#include "svector.h"
#include "pcaquan.h"
#include "parsfuncs.h"

COUNTER KEYWORDTABLESIZE = 50;



//---------------------------------------------------------------------------
//
//  Function Name: pca_parse
//                 
//  Arguments    : takes in a pointer to character data, which can be either
//                 the actual input data, or, if the format is "@<filename>",
//                 the constructor will read in the file and parse it.
//                 inSize allows you to set the size (in chars) of the TokList.
//                 inSize defaults to 10000
//  Comments     : allocates the TokList and gets the TokListSize. Also
//                 generates the token list, this is just a list of strings
//                 which will either represent a keyword or a filename.
//
//---------------------------------------------------------------------------

pca_parse::pca_parse(char *data, COUNTER inSize)
{
    COUNTER fIndex;
    COUNTER index;
    COUNTER openFlag = 0;
    COUNTER TokIndex = 0;
    
    FLAG continueFlag = True;
    
    CHAR inData  = 1;
    CHAR space   = ' ';
    CHAR ret     = '\n';
    CHAR lparen  = '(';
    CHAR rparen  = ')';
    CHAR tab     = '\t';
    
    char *filename;
    FILE *filePointer;

    // Member data initialization.
    TokList     = NULL;
    bufSize     = inSize;
    TokListSize = 0;
    filename    = new char[256];
    tokIndex    = 0;
        
    
    // An "@" indicates an input file, try to open it.
    if(strstr(data,"@") != NULL)
    {
        strcpy( filename, (data+1) );

        if(( filePointer = fopen(filename, "rt")) == NULL)
        {
            printf("ERROR - Input file missing or corrupted : %s\n", filename);
            AssertFatal(filePointer, avar("Input file : %s missing or corrupted\n",filename));
            delete [] filename;
            delete [] TokList;
            exit(0);
        }

        // If we successfully opened the parse file, load it into memory and put
        // its data into the token list.
        TokList = new char[bufSize];
        for(index = 0;index < bufSize;index++)
            TokList[index] = 0;
        
        //for(fIndex = 0; error != 0; fIndex++)
        while(continueFlag != False)
        {
            // fread returns 0 if no data read, False is = 0, this will break
            // the while loop.

            continueFlag = fread(&inData, sizeof(CHAR), 1, filePointer);
            
            // Some keywords have the form <keyword>(data,data,...) we want to
            // preserve the parenthesis state over lines/tabs/spaces.
            if(inData == lparen) openFlag = 1;
            if(inData == rparen) openFlag = 0;
        
            // If we are in the middle of a piece of data, keep going.
            if((inData == space) || (inData == ret) || (inData == tab) || (inData == 0))
            {
                // If we already have a return character in the TokList for this
                // data seperation (a space, tab, or return in the input file),
                // then we don't need yet another return! We also don't want
                // a return at the beginning of the token list because the
                // first character read was a space/tab/ret character.
                if((TokIndex != 0) && (TokList[(TokIndex-1)] != ret) && (!openFlag))
                {                                                  
                    TokList[TokIndex] = ret;
                    TokIndex++;
                }
            }
            else
            {
                TokList[TokIndex] = inData;
                TokIndex++;
            }
        }
        
        TokList[TokIndex] = 0;
        fclose(filePointer);
        for(fIndex = 0;TokList[fIndex] != 0;fIndex++)
        {
            if(TokList[fIndex] == ret) TokListSize++;
        }
    }
    
    // If there is no "@" symbol in the datalist, then we have
    // command line input, and this section tokenizes that.
    else
    {
        TokList = new char[bufSize];
        for(index = 0;index < bufSize;index++)
            TokList[index] = 0;

        for(fIndex = 0; data[fIndex] != 0; fIndex++)
        {
            inData = data[fIndex];
            if(inData == lparen) openFlag = 1;
            if(inData == rparen) openFlag = 0;
        
            if((inData == space) || (inData == ret) || (inData == tab))
            {
                if((TokIndex != 0) && (TokList[(TokIndex-1)] != ret) && (!openFlag))  // if we're in the middle of something,
                {                                                  // keep going!
                    TokList[TokIndex] = ret;
                    TokIndex++;
                }
            }
            else
            {
                TokList[TokIndex] = inData;
                TokIndex++;
            }
        }
        
        // Zero the last token, and count the size of the TokList. The token list
        // has the form <token>\n<token>\n....<finaltoken>0 so counting returns
        // is valid.
        TokList[TokIndex] = 0;
        for(fIndex = 0;TokList[fIndex] != 0;fIndex++)
        {
            if(TokList[fIndex] == ret) TokListSize++;
        }
    }

    // If we were given a token "*.bmp" we need to expand this into the 
    // appropriate filenames, ExpandWildCard returns the number of 
    // files found in expanding the wild card into a list, so we need to increment
    // the token list size by this amount.
    TokListSize += ExpandWildCard();

    // The number of flags that need to be allocated is found by counting the
    // number of image file names, and adding to it an input palette if it exists.
    FindFlagNumber();
    InitKeywordTable();
}       
        

//---------------------------------------------------------------------------
//
//  Function Name: InitKeywordTable
//                 
//  Arguments    : none
//                 
//                 
//                 
//                 
//  Comments     : Allocates memory and registers each keyword and its
//                 associated function in the keyword table.
//
//
//---------------------------------------------------------------------------

void pca_parse::InitKeywordTable(void)
{
    COUNTER index;

    KeyWordTable = new keywordtable[KEYWORDTABLESIZE+1];
    for(index = 0;index < (KEYWORDTABLESIZE+1);index++)
    {
        KeyWordTable[index].keywordname = NULL;
        KeyWordTable[index].keywordfunction = NULL;
    }

    RegisterKeyword("CHOOSE"             , choose);
    RegisterKeyword("NOCHOOSE"           , nochoose);
    RegisterKeyword("NORENDER"           , norender);
    RegisterKeyword("QUANTIZERGB"        , quantizergb);
    RegisterKeyword("QUANTIZEALPHA"      , quantizealpha);
    RegisterKeyword("RENDER"             , render);
    RegisterKeyword("NOSHADE"            , noshade);
    RegisterKeyword("NOHAZE"             , nohaze);
    RegisterKeyword("SHADE"              , shade);
    RegisterKeyword("HAZE"               , haze);
    RegisterKeyword("PALOUTPUTNAME"      , paloutputname);
    RegisterKeyword("NOWEIGHT"           , noweight);
    RegisterKeyword("DITHER"             , ditherflag);
    RegisterKeyword("NODITHER"           , nodither);
    RegisterKeyword("PALNAME"            , palname);
    RegisterKeyword("OUTPATH"            , outpath);
    RegisterKeyword("ZEROCOLOR"          , zerocolor);
    RegisterKeyword("IGNORECOLORS"       , ignorecolors);
    RegisterKeyword("LUVDISTANCE"        , luvdistance);
    RegisterKeyword("RGBDISTANCE"        , rgbdistance);
    RegisterKeyword("DITHERTOLERANCE"    , dithertolerance);
    RegisterKeyword("WEIGHTEDRGBDISTANCE", weightedrgbdistance);
    RegisterKeyword("NOZEROCOLOR"        , nozerocolor);
    RegisterKeyword("BLENDRANGE"         , blendrange);
    RegisterKeyword("HAZELEVELS"         , hazelevels);
    RegisterKeyword("SHADELEVELS"        , shadelevels);

}


//---------------------------------------------------------------------------
//
//  Function Name: RegisterKeyword
//                 
//  Arguments    : the keyword name and associated function pointer.
//                 
//                 
//                 
//                 
//  Comments     : BE SURE THAT THE KEYWORDTABLESIZE is at least as large
//                 as the number of keywords you wish to register, 
//                 otherwise, this function will exit.
//
//---------------------------------------------------------------------------

void pca_parse::RegisterKeyword(const char *keyname, FLAG (*keyfunc)(void *, char*, int) )
{
    static COUNTER currentIndex = 0;
    if(currentIndex == KEYWORDTABLESIZE)
    {
        AssertFatal(0, avar("Make the constant KEYWORDTABLESIZE greater than the number of keywords you wish to register"));
    }
    KeyWordTable[currentIndex].keywordname = new char[1000];
    strcpy(KeyWordTable[currentIndex].keywordname, keyname);
    KeyWordTable[currentIndex].keywordfunction = keyfunc;
    currentIndex++;
}


//---------------------------------------------------------------------------
//
//  Function Name: ~pca_parse
//                 
//  Arguments    : none
//                 
//                 
//                 
//                 
//  Comments     : deletes the token list and KeyWordTable.
//                 
//
//
//---------------------------------------------------------------------------

pca_parse::~pca_parse(void)
{
    if(TokList)
    {
        delete [] TokList;
    }
    if(KeyWordTable)
    {
        delete [] KeyWordTable;
    }
}


//---------------------------------------------------------------------------
//
//  Function Name: GetWeight
//                 
//  Arguments    : A token line containing an image file name and weight operator
//                 e.g. file00.bmp(15) tells the parser to consider file00.bmp 
//                 with weight fifteen.
//
//  Comments     : This function finds the first occurence of "("
//                 in a line, and reads the int directly after it. Make sure
//                 it is used ONLY on token lines with filenames. Returns
//                 the default weight 1 if the file has no (<weight>) param.
//                 Also - strips the weight param off the filename line.
//---------------------------------------------------------------------------

COUNTER pca_parse::GetWeight(char *inLine)
{
    COUNTER index;
    COUNTER lineWidth;
    COUNTER retVal;
  
    // Find the number of characters in the line.
    for(lineWidth = 0;inLine[lineWidth] != 0;lineWidth++);

    // Scan through for first (
    for(index = 0;(inLine[index] != '(') && (inLine[index] != 0);index++);

    // If we fell to end, then there is no specified weight, and we'll
    // return the default.
    if(index == lineWidth)
    {
        return 1;
    }

    // Otherwise, get the weight and return it.
    sscanf(inLine+index+1,"%d)", &retVal);
    inLine[index] = 0;
    return retVal;
}


//---------------------------------------------------------------------------
//
//  Function Name: FindFlagNumber
//                 
//  Arguments    : void
//
//                 
//  Comments     : Sets the FlagNumber COUNTER i.e. the number of flags needed 
//                 (at this point, just a file count).
//
//                 
//
//
//---------------------------------------------------------------------------

void pca_parse::FindFlagNumber(void)
{
    COUNTER index;
    COUNTER lineIndex;
    COUNTER flagCounter = 0;

    char *line = new char[256];


    // Go through the token list and see if the files match one
    // of the descriptive extensions, watch for capitals. 
    for(index = 0;index < TokListSize;index++)
    {
        GetNextLine(line);
        if(strstr(line,".pal") != NULL)
        {
            flagCounter++;
        }
        else

        if(strstr(line,".tap") != NULL)
        {
            flagCounter++;
        }
        else

        if(strstr(line,".bmp") != NULL)
        {
            flagCounter++;
        }
        else
        
        if(strstr(line,".BMP") != NULL)
        {
            flagCounter++;
        }
        else

        if(strstr(line,".PAL") != NULL)
        {
            flagCounter++;
        }
        else

        if(strstr(line,".TAP") != NULL)
        {
            flagCounter++;
        }
        else

        if(strstr(line,".tga") != NULL)
        {
            flagCounter++;
        }

        else
        if(strstr(line,".TGA") != NULL)
        {
            flagCounter++;
        }
 
        // Be careful here - there is the potential for a one off
        // in the flag counter, there may be 2 .pal files specified,
        // one using the PALNAME keyword, that's the one that gives fixed
        // ranges, and one output filename from PALOUTPUTNAME, so
        // scan through the token list and see if PALOUTPUTNAME is in
        // there, if it is, decrement the flagCounter.

    }
    tokIndex = 0;
    for(lineIndex = 0;lineIndex < TokListSize;lineIndex++)
    {
        GetNextLine(line);
        if(strstr(line,"PALOUTPUTNAME") != NULL)
            flagCounter--;
    }
    tokIndex = 0;
    // Set the data member to our temporary counter.        
    FlagNumber = flagCounter;
    delete [] line;
}


//---------------------------------------------------------------------------
//
//  Function Name: GetFlagNumber
//                 
//  Arguments    : void
//
//                 
//  Comments     : returns the number of flags needed by the token list.
//                 i.e. the FlagNumber member data.
//
//                 
//
//
//---------------------------------------------------------------------------

COUNTER pca_parse::GetFlagNumber(void)
{
    return FlagNumber;
}


//---------------------------------------------------------------------------
//
//  Function Name: GetNextLine
//                 
//  Arguments    : outLine is a character buffer in which the next line into
//                 the TokList is read.
//               
//                 
//  Comments     : Starts at the first line, and retrieves each subsequent
//                 line, when no more tokens are available, outLine is set                 
//                 to "END". tokIndex is member data that keeps track of the
//                 position of the next line read, when scanning through the
//                 token list, remember to 0 this if you want to start over.
//---------------------------------------------------------------------------

void pca_parse::GetNextLine(CHAR *outLine)
{
    COUNTER index;

    CHAR ret = '\n';

    // if we are at the end of the tokList, tell us.
    if( TokList[tokIndex] == 0 )
    {
        strcpy(outLine, "END");
        return;
    }

    // Otherwise, read to the next return (\n).
    for(index = 0;TokList[tokIndex + index] != ret; index++)
    {
        outLine[index] = TokList[ tokIndex + index ];
    }
    outLine[index] = 0;
    tokIndex += index;
    tokIndex++;     // To get out of the '\n' area
}


//---------------------------------------------------------------------------
//
//  Function Name: MakeImageFlags 
//                 
//  Arguments    : inFlags is a pointer to an allFlags buffer - this will
//                 be filled in by this function, make sure there have been
//                 FlagNumber of these allocated.
//
//  Comments     : each image has a flag list, and this function gets them.
//                 Notice - you must call GetFlagNumber and allocate
//                 the buffer first.
//
//
//---------------------------------------------------------------------------

void pca_parse::MakeImageFlags(allFlags *inFlags)
{
    COUNTER index;
    char    *line = new char[1000];
    COUNTER countLines = 0;
    FLAG    isKeyword = False;
    FLAG    requireUpdate = False;
                

    while(strcmp(line, "END") != NULL)
    {
        GetNextLine(line);
        for(index = 0; (KeyWordTable[index].keywordname != NULL) && 
                       (KeyWordTable[index].keywordfunction != NULL); index++)
        {
            if(StringValue(line) == index)
            {
                requireUpdate = KeyWordTable[index].keywordfunction(inFlags, line, countLines);
                isKeyword = True;
            }
        }
        if(isKeyword == False)
        {
            IsBitmap(line, inFlags, countLines);
            countLines++;
            if(countLines == FlagNumber) return;
        }
        if(requireUpdate == True) 
        {
            UpdateFlags(inFlags, countLines);
            countLines++;
            if(countLines == FlagNumber) return;
        }
        requireUpdate = False;
        isKeyword = False;
    }
    delete [] line;
}


//---------------------------------------------------------------------------
//
//  Function Name: IsBitmap
//                 
//  Arguments    : line is the token line to consider, inFlags is the allFlags ptr
//                 to fill and inFlagIndex is the number of the inFlag that we
//                 are filling.
//
//  Comments     : When we get to a non-keyword, we assume it is a bitmap 
//                 and finish off its allFlags entry, then UpdateFlags is
//                 called to copy the info to the next allFlags struct in the
//                 inFlags buffer.
//
//---------------------------------------------------------------------------

void pca_parse::IsBitmap(char *line, allFlags *inFlags, COUNTER inFlagIndex)
{
    inFlags[inFlagIndex].inWeight = GetWeight(line);
    if(inFlags[inFlagIndex].weightFlag == False)
    {
        inFlags[inFlagIndex].inWeight = 0;
    }

    if(inFlags[inFlagIndex].inFilename == NULL)
    {
        inFlags[inFlagIndex].inFilename = new char[256];
        strcpy(inFlags[inFlagIndex].inFilename, line);
    }

//  Now we have the filename, it's time to ship it out!
//  That means - move on to the next inFlags entry, copy the previous one to it
//  that way we get the same flags, so consecutive filenames (with no keywords
//  between them) get the same attributes.
    UpdateFlags(inFlags, inFlagIndex);
}




//---------------------------------------------------------------------------
//
//  Function Name: UpdateFlags
//                 
//  Arguments    : inFlags is a pointer to an allFlags buffer, inFlagIndex
//                 is the current flag number.
//
//  Comments     : Copies the current set of flags one forward, returns the
//                 updated index. This copy is pretty large, and not so pretty.
//                 
//
//
//---------------------------------------------------------------------------

COUNTER pca_parse::UpdateFlags(allFlags *inFlags, COUNTER inFlagIndex)
{
    COUNTER index; 

    // Go on to next flag 
    inFlagIndex++;

    // If it's the final one, spit it back
    if(inFlagIndex == FlagNumber) return inFlagIndex;

    // Otherwise start copying.
    inFlags[inFlagIndex].inBlendFirst        = inFlags[inFlagIndex-1].inBlendFirst;
    inFlags[inFlagIndex].inBlendLast         = inFlags[inFlagIndex-1].inBlendLast;

    inFlags[inFlagIndex].inShadeLevels       = inFlags[inFlagIndex-1].inShadeLevels;
    inFlags[inFlagIndex].inHazeLevels        = inFlags[inFlagIndex-1].inHazeLevels;


    inFlags[inFlagIndex].inZeroFlag          = inFlags[inFlagIndex-1].inZeroFlag;
    inFlags[inFlagIndex].inTolerance         = inFlags[inFlagIndex-1].inTolerance;
    inFlags[inFlagIndex].inDistanceType      = inFlags[inFlagIndex-1].inDistanceType;
    inFlags[inFlagIndex].weightFlag          = inFlags[inFlagIndex-1].weightFlag;
    inFlags[inFlagIndex].inWeight            = inFlags[inFlagIndex-1].inWeight;
    inFlags[inFlagIndex].inDitherFlag        = inFlags[inFlagIndex-1].inDitherFlag;
    inFlags[inFlagIndex].inQuantizeFlag      = inFlags[inFlagIndex-1].inQuantizeFlag;

    inFlags[inFlagIndex].inRenderFlag        = inFlags[inFlagIndex-1].inRenderFlag;

    inFlags[inFlagIndex].inRenderRangeFirst  = inFlags[inFlagIndex-1].inRenderRangeFirst;
    inFlags[inFlagIndex].inRenderRangeLast   = inFlags[inFlagIndex-1].inRenderRangeLast;
    inFlags[inFlagIndex].inPaletteRangeFirst = inFlags[inFlagIndex-1].inPaletteRangeFirst;
    inFlags[inFlagIndex].inPaletteRangeLast  = inFlags[inFlagIndex-1].inPaletteRangeLast;
    if(inFlags[inFlagIndex-1].inPalOutput != NULL)
        strcpy(inFlags[inFlagIndex].inPalOutput, inFlags[inFlagIndex-1].inPalOutput);


    if(inFlags[inFlagIndex-1].inShadeTowards != NULL)
    {
        inFlags[inFlagIndex].inShadeTowards = new PALETTEENTRY;
        
        inFlags[inFlagIndex].inShadeTowards->peRed   = inFlags[inFlagIndex-1].inShadeTowards->peRed;
        inFlags[inFlagIndex].inShadeTowards->peGreen = inFlags[inFlagIndex-1].inShadeTowards->peGreen;
        inFlags[inFlagIndex].inShadeTowards->peBlue  = inFlags[inFlagIndex-1].inShadeTowards->peBlue;
    }

    inFlags[inFlagIndex].inShadeIntensity    = inFlags[inFlagIndex-1].inShadeIntensity;

    if(inFlags[inFlagIndex-1].inHazeTowards != NULL)
    {
        inFlags[inFlagIndex].inHazeTowards = new PALETTEENTRY;
        inFlags[inFlagIndex].inHazeTowards->peRed   = inFlags[inFlagIndex-1].inHazeTowards->peRed;
        inFlags[inFlagIndex].inHazeTowards->peGreen = inFlags[inFlagIndex-1].inHazeTowards->peGreen;
        inFlags[inFlagIndex].inHazeTowards->peBlue  = inFlags[inFlagIndex-1].inHazeTowards->peBlue;
    }

    inFlags[inFlagIndex].inHazeIntensity     = inFlags[inFlagIndex-1].inHazeIntensity;
    strcpy(inFlags[inFlagIndex].inOutputPath,inFlags[inFlagIndex-1].inOutputPath);
    inFlags[inFlagIndex].numIgnores          = inFlags[inFlagIndex-1].numIgnores;

    if(inFlags[inFlagIndex-1].inZeroColor != NULL)
    {
        inFlags[inFlagIndex].inZeroColor = new PALETTEENTRY;
        inFlags[inFlagIndex].inZeroColor->peRed   = inFlags[inFlagIndex-1].inZeroColor->peRed;
        inFlags[inFlagIndex].inZeroColor->peGreen = inFlags[inFlagIndex-1].inZeroColor->peGreen;
        inFlags[inFlagIndex].inZeroColor->peBlue  = inFlags[inFlagIndex-1].inZeroColor->peBlue;
    }

    if(inFlags[inFlagIndex-1].inIgnoreList != NULL)
    {
        inFlags[inFlagIndex].inIgnoreList         = new PALETTEENTRY[inFlags[inFlagIndex].numIgnores];
        for(index = 0; index < inFlags[inFlagIndex].numIgnores;index++)
        {
            inFlags[inFlagIndex].inIgnoreList[index].peRed   = inFlags[inFlagIndex-1].inIgnoreList[index].peRed;
            inFlags[inFlagIndex].inIgnoreList[index].peGreen = inFlags[inFlagIndex-1].inIgnoreList[index].peGreen;
            inFlags[inFlagIndex].inIgnoreList[index].peBlue  = inFlags[inFlagIndex-1].inIgnoreList[index].peBlue;
        }
    }
    inFlags[inFlagIndex].inChooseFlag         = inFlags[inFlagIndex - 1].inChooseFlag;
    return inFlagIndex;
}


//---------------------------------------------------------------------------
//
//  Function Name: ExpandWildCard
//                 
//  Arguments    : none
//                 
//
//  Comments     : expands wild card file names, giving each a place in the
//                 token list, the return value is the number of filenames 
//                 found in the wild card expansion.
//
//
//---------------------------------------------------------------------------

COUNTER pca_parse::ExpandWildCard(void)
{
    COUNTER index;
    COUNTER numFiles = 0;
    COUNTER slashCount = 0;
    COUNTER indexy;
    COUNTER counter = 0;

    char *line = new char[256];
    char *holdPath = new char[256];
    char *ext = new char[3];
    ext[0] = 0;
    ext[1] = 0;
    ext[2] = 0;

    HANDLE winHandle;
    WIN32_FIND_DATA fillStruct;

    // Zero the internal member tokIndex so that we start at the beginning
    // of the token list.
    tokIndex = 0;

    // The plan is to copy the expanded token list into a new one inserting
    // the filenames in their proper place. Make a new token list of bufSize
    // that can be filled with the new token info.
    char *newTokList = new char[bufSize];

    // Zero this new list.
    for(index = 0;index < bufSize;index++)
        newTokList[index] = 0;

    // Now just go through the token list looking for wildcards, if
    // a line has now wildcard, it is simply added into the token list
    for(index = 0;index < TokListSize;index++)
    {
        GetNextLine(line);

        // Check for wild card - we need to get the path
        // for the wildcard files, this is stored in holdPath.
        if(strstr(line,"*.") != NULL)
        {
            strcpy(holdPath, line);
            
            // Count back-slashes, then go through to the last
            // one and truncate, that way holdPath earns its namesake.
            for(indexy = 0;holdPath[indexy] != 0;indexy++)
                if(holdPath[indexy] == '\\') slashCount++;

            for(indexy = 0;holdPath[indexy] != 0;indexy++)
            {
                if(counter == slashCount)
                {
                    break;
                }
                if(line[indexy] == '\\')
                    counter++;
            }
            holdPath[indexy] = 0;

            // Next we need to actually find the relevant files. 
            // As always, the first one is special and must be done "by hand".
            winHandle = FindFirstFile(line,&fillStruct);
            strcat(newTokList, holdPath);
            strcat(newTokList, fillStruct.cFileName);
            strcat(newTokList, "\n");
            numFiles++;

            // Get the rest of 'em.
            while(FindNextFile(winHandle, &fillStruct) != False)
            {
                strcat(newTokList, holdPath);
                strcat(newTokList, fillStruct.cFileName);
                strcat(newTokList, "\n");
                numFiles++;
            }
            // close the one open handle.
            FindClose(winHandle);
        }           


        // if a line has no wildcards and is not "END", then just insert it
        // in the new token list
        else
        {
            if(strcmp(line,"END") == 0)
                break;
            strcat(newTokList, line);
            strcat(newTokList, "\n");
        }
    }
    
    
    // Zero the old token list, and copy the new one into it.
    for(index = 0;index < TokListSize;index++)
    {
        TokList[index] = 0;
    }
    strcpy(TokList, newTokList);

    // Reset the tokIndex to zero so the next call to GetLine gets 
    // the first token list entry.
    tokIndex = 0;

    
    delete [] line;
    delete [] newTokList;
    delete [] ext;
    delete [] holdPath;
    
    // Return the number of file names found.
    return numFiles;
}


//---------------------------------------------------------------------------
//
//  Function Name: StringValue
//                 
//  Arguments    : Takes in a line from the token list
//                 
//
//  Comments     : outputs one of the COUNTER'ed parser keywords. This
//                 function is here to eliminate ambiguity introduced with
//                 "strstr" that requires a careful ordering of the if-else
//                 hierarchy.
//
//---------------------------------------------------------------------------


COUNTER pca_parse::StringValue(char *line)
{
    COUNTER index;
    COUNTER charIndex;
    FLAG isEqual = True;

    for(index = 0;KeyWordTable[index].keywordname != NULL;index++)
    {
        for(charIndex = 0;KeyWordTable[index].keywordname[charIndex] != 0;charIndex++)
        {
            if(line[charIndex] != KeyWordTable[index].keywordname[charIndex])
                isEqual = False;
        }
        // if the next line character is an alpha character, we have
        // an exact match on the keyword, but not on the line -
        // forex. DITHER and DITHERTOLERANCE both match to DITHER by
        // the above, but isalpha on the next character of DITHERTOLERANCE
        // is true, "T" is alpha, so we know that DITHER is not a
        // keyword match for line=DITHERTOLERANCE.
        if((isEqual == True) && !(isalpha(line[charIndex]))) return index;
        isEqual = True;
    }
    return KEYWORDTABLESIZE;
}           





    
