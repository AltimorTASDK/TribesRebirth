//---------------------------------------------------------------------------
//
//
//  Filename   : main.cpp
//
//
//  Description: Here we go - the main function/
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

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef MSVC
#include <dir.h>
#endif
#include <base.h>
#include "pcatypes.h"
#include "pcaparse.h"
#include "svector.h"
#include "pcaquan.h"
#include "imageFile.h"
#include "luvconv.h"
#include "render.h"
#include "makepal.h"


// Just some helper functions we'll need in main to do grunt work.
void BlendPalettes(allFlags *, quantizeFlags *, imageFile *);
void InitFlags(allFlags *, renderFlags *, quantizeFlags *, fileFlags *, COUNTER);
void CopyAuxFlags(allFlags *, renderFlags *, quantizeFlags *, fileFlags *, COUNTER);
#ifdef MSVC
signed int __cdecl SHImageCompare(const void *, const void *);
#else
signed int SHImageCompare(const void *, const void *);
#endif


__cdecl main(int argc, char **argp)
{
    COUNTER     index;
    COUNTER     numFlags;
    COUNTER     choseNum = 0;

    signed int  paletteIndex = -1;

    char        *cmdline = new char[10000];
    pca_parse   *Testparse;
    FLAG        doneSH = False;

    if(argc == 1)
    {
        printf("PICA Version 4.1\nUsage is: pica @<filename> \nSee pica.html, or pica.ps for details\n");
        exit(0);
    }

    // The working palette - the one that is passed to the renderer
    // This is either filled via quantization, or with a palette from
    // a file. It's zeroed to start.
    PALETTEENTRY Palette[256];
    for(index = 0;index < 256;index++)
    {
        Palette[index].peRed   = 0;
        Palette[index].peGreen = 0;
        Palette[index].peBlue  = 0;
        Palette[index].peFlags = 0;
    }

    // If we're dealing with an input file, we need to construct the
    // parser differently.
    if(strstr(argp[1], "@") == NULL)
    {
        strcpy(cmdline, argp[1]);
        strcat(cmdline, " ");
        for(index = 2; index < (COUNTER) argc;index++)
        {
            strcat(cmdline, argp[index]);
            strcat(cmdline, " ");
        }
        Testparse = new pca_parse(cmdline);
    }
    else
    {
        Testparse = new pca_parse(argp[1]);
    }
    pca_parse &testparse = *Testparse;

    // numFlags is the number of filenames, including palette,
    // passed in. If you give 8 bitmaps to scan and remap, and a
    // palette that is being incrementally built, numFlags will
    // be 9.
    numFlags = testparse.GetFlagNumber();

    // allFlags contains all flag information, it is filled by
    // the parser, and then broken down into the class-specific
    // flag structures - renderFlags, quantizeFlags, fileFlags.
    allFlags        *inFlags      = new allFlags[numFlags];
    renderFlags     *renderInfo   = new renderFlags[numFlags];
    quantizeFlags   *quantizeInfo = new quantizeFlags[numFlags];
    fileFlags       *fileInfo     = new fileFlags[numFlags];

    // Set flags to their initial values.
    InitFlags(inFlags, renderInfo, quantizeInfo, fileInfo, numFlags);

    // The parser fills the inFlags with information, we will need to break
    // this up into the various class structures.
    testparse.MakeImageFlags(inFlags);

    // Get the shade/haze filenames up to the top, we'll scan them first.
    qsort(inFlags, numFlags, sizeof(allFlags),SHImageCompare);
    CopyAuxFlags(inFlags, renderInfo, quantizeInfo, fileInfo, numFlags);

    // Make the imageFile array, each file in the parse list
    // gets its own class. Whip through the array setting the flags
    // to their "index"ed value. All indices refer to the same thing,
    // i.e. the 5th entry in the renderFlags goes with the 5th entry of
    // the fileFlags, goes with the 5th entry of the quantizeFlags.
    imageFile *imageArray = new imageFile[numFlags];

    for(index = 0;index < numFlags;index++)
    {
        imageArray[index].SetFlags(&fileInfo[index]);
    }


    // There are two special cases that I want to check for first. Sometimes there
    // is no list of bitmaps, as in the case of a makefile with art that has a variable
    // name but no entries, just a stub for later. In that case, we want to simply save
    // off the input palette and exit. The only flag will be for the PALNAME palette.
    if(numFlags == 1)
    {
        char *test = new char[256];

        imageArray[0].Load();
        if(imageArray[0].ReturnType() == PAL)
        {
            PALETTEENTRY *tempPal;
            tempPal = imageArray[0].GetPalette();
            for(index = 0;index < 256;index++)
            {
                Palette[index].peRed   = tempPal[index].peRed;
                Palette[index].peGreen = tempPal[index].peGreen;
                Palette[index].peBlue  = tempPal[index].peBlue;
                Palette[index].peFlags = tempPal[index].peFlags;
            }
            imageArray[0].ReleaseData();

            strcpy(test,inFlags[0].inPalOutput);

            // Prepends the path held in the imageArray's output path buffer.
            imageArray[0].MakeSaveName(test);
            imageFile PalSave;
            PalSave.SetType(PAL);
            PalSave.SetFilename(test);
            printf("Saving Palette to : %s\n",test);
            PalSave.SetPalette(Palette);
            PalSave.Save(); 
            return 0;
        }
        imageArray[0].ReleaseData();
    }       


    // It's also possible that we have a MERGEPAL setup, where we want to read in a
    // transluscent palette and pick blended colors. What we want to do is "create" 
    // a sort of bitmap with the blended colors in it. The syntax for this in the parse
    // file is to give, as the "bitmap list" a transluscency palette previously created.
    // Putting it in the bitmap list area makes sense (I hope) because we are making a faux
    // image out of it and scanning in colors.
    if(numFlags == 2)
    {
        imageArray[1].Load();

        // If the only file in the "bitmap list" is a palette, we know what to do. Otherwise,
        // we are legitimately quantizing or rendering just one 24-bit bitmap.
        if(imageArray[1].ReturnType() == PAL)
        {
            BlendPalettes(inFlags, quantizeInfo, imageArray);
            return 0;
        }   
    }

    // Throughout, I use the 0th flag for information
    // that must be the same for all files, forex. you cannot
    // quantize some of the files with ALPHA and some with RGB,
    // this requires two seperate calls.

    pca_quantize testquant(inFlags[0].inQuantizeFlag);

    // Moving on to the non-special case, this is the main loop for quantization,
    // it scans through the bitmaps, if it finds a palette, it loads it in as a fixed
    // palette. By the end of this "for" loop, we have scanned in all the bitmaps, and
    // are ready to call the quantization function. 

    printf("Scanning : ");
    for(index = 0;index < numFlags;index++)
    {

        // Load the image data.
        imageArray[index].Load();


        // If it is not a palette, it is something we are 
        // interested in for its zero color.
        if(imageArray[index].ReturnType() != PAL)
        {
            // If no zero color was specified in the parse file,
            // set it to the upper left pixel value.
            if((inFlags[index].inZeroColor == NULL) && (inFlags[index].inZeroFlag == True))
            {
                quantizeInfo[index].inZeroColor = new PALETTEENTRY;
                if(imageArray[index].GetZeroColor(quantizeInfo[index].inZeroColor) == False)
                {
                    printf("Bitmap file %s is not loaded\n", inFlags[index].inFilename);
                    exit(0);
                }
                renderInfo[index].inZeroColor = new PALETTEENTRY;
                renderInfo[index].inZeroColor->peRed   = quantizeInfo[index].inZeroColor->peRed;
                renderInfo[index].inZeroColor->peGreen = quantizeInfo[index].inZeroColor->peGreen;
                renderInfo[index].inZeroColor->peBlue  = quantizeInfo[index].inZeroColor->peBlue;
                renderInfo[index].inZeroColor->peFlags = quantizeInfo[index].inZeroColor->peFlags;
            }
            // if the zero color was specified in the parse file,
            // then we're fine, it's already been set in the quantize
            // and render info flags. Continue.
        }       


        // If it's a palette file, we pull in the palette and add
        // the colors to the quantizer's fixed list.

        if(imageArray[index].ReturnType() == PAL)
        {
            // To refer to the palette file in the future, use the paletteIndex.
            paletteIndex = index;

            // We don't want to render the palette file.
            inFlags[paletteIndex].inRenderFlag = False;
            testquant.SetFlags(&quantizeInfo[index]);
            testquant.AddPaletteColors(imageArray[index].ReturnPal(), 
                                       inFlags[index].inPaletteRangeFirst, 
                                       inFlags[index].inPaletteRangeLast,
                                       inFlags[index].inRenderRangeFirst, 
                                       inFlags[index].inRenderRangeLast);            

            printf("\nFixed Palette : %s\n", inFlags[index].inFilename);
        }
        else


        // Otherwise it must be an image file (bmp/tga) and we can
        // try to find the zero color and scan it into the quantizer's
        // ColorData buffer. If we are supposed to be choosing colors,
        // the inChooseFlag will be true. We need the number "index" for
        // flag information, remember that inFlags[index], renderInfo[index],
        // etc. all refer to the same image file, they all contain attributes
        // needed by some function.

        if(inFlags[index].inChooseFlag == True)
        {
            if(imageArray[index].ReturnType() == BMP8)
            {
                AssertFatal(0, avar("ERROR - bitmap %s is not 24-bit \n", inFlags[index].inFilename));
            }
            testquant.SetFlags(&quantizeInfo[index]);
            testquant.ScanImage(imageArray[index].GetData24(),
                                imageArray[index].GetSize(),
                                inFlags[index].inWeight);

//
//            printf("Scanning : %s\n",inFlags[index].inFilename);
            printf(".");


            // We want to do this once and only once, when the first non-shaded/hazed
            // bitmap comes up. The "doneSH" flag is False until this loop, after which
            // it is true. Because of the sort, we are assured that the first bitmap we
            // hit with neither shade nor haze set will be the beginning of the last of
            // them! If the shade and haze colors are NULL, the HazeShadeScan will
            // return without doing anything.
            if((!inFlags[index].inShadeTowards) && (!inFlags[index].inHazeTowards) && (!doneSH)
                && (index > 0))
            {
                doneSH = True;              
                testquant.HazeShadeScan(inFlags[index-1].inShadeTowards, inFlags[index-1].inShadeIntensity,
                                        inFlags[index-1].inHazeTowards, inFlags[index-1].inHazeIntensity,
                                        inFlags[index-1].inShadeLevels, inFlags[index-1].inHazeLevels);
            }
            choseNum++;
        }
        imageArray[index].ReleaseData();
    }

    printf("\n");

    // Little overkill here, but the point is, if everything was shaded or hazed, we never hit the
    // if in the above, so do it now!
    if((choseNum != 0) && (inFlags[index].inShadeTowards) || (inFlags[index].inHazeTowards) && (!doneSH))
    {
        testquant.HazeShadeScan(inFlags[index-1].inShadeTowards, inFlags[index-1].inShadeIntensity,
                                inFlags[index-1].inHazeTowards, inFlags[index-1].inHazeIntensity,
                                inFlags[index-1].inShadeLevels, inFlags[index-1].inHazeLevels);
    }

    // If we got colors from any bitmap and quantized them, save
    // the palette off and continue.
    if(choseNum != 0)
    {
        PALETTEENTRY *fixedPalette = NULL;

        COUNTER first = inFlags[0].inPaletteRangeFirst;
        COUNTER last  = inFlags[0].inPaletteRangeLast;
        COUNTER renderFirst = inFlags[0].inRenderRangeFirst;
        COUNTER renderLast  = inFlags[0].inRenderRangeLast;
        
        // If we got a palette, paletteIndex, if it exists
        // (i.e. != -1) is the index of the palette file in the flag structures.
        // We've registered the fixed palette with the quantizer, but we also need
        // it for the "RemakeFixedColors" function (from makepal.cpp), which restores
        // the order of the fixed palette.

        if(paletteIndex != -1) fixedPalette = imageArray[paletteIndex].GetPalette();
        
        // Get the palette from the quantizer, and restore it to its original state
        // with the fixed palette.

        testquant.GetPalette(Palette, first, last);
        RemakeFixedColors(Palette, fixedPalette,first, last, renderFirst, renderLast);

        char *test = new char[256];

        // Save it off. If we're choosing, then it's got a name : inPalOutput.
        strcpy(test,inFlags[0].inPalOutput);

        // Prepends the path held in the imageArray's output path buffer, this will be the
        // same for all bitmaps - so I can use "0".
        imageArray[0].MakeSaveName(test);
        imageFile PalSave;
        PalSave.SetType(PAL);
        PalSave.SetFilename(test);
        printf("Saving Palette to : %s\n",test);
        PalSave.SetPalette(Palette);
        PalSave.Save(); 
    }
    else

    // If we didn't choose any colors from bitmaps, then we are
    // obviously just rendering, and for that we need a prefabbed
    // palette, find it in the imageFile list and load the palette.
    {
        for(index = 0;index < numFlags;index++)
        {
            imageArray[index].Load();
            if(imageArray[index].ReturnType() == PAL)
            {
                paletteIndex = index;
                inFlags[paletteIndex].inRenderFlag = False;
            }
            imageArray[index].ReleaseData();
        }
        PALETTEENTRY *tempPal;
        tempPal = imageArray[paletteIndex].GetPalette();
        for(index = 0;index < 256;index++)
        {
            Palette[index].peRed   = tempPal[index].peRed;
            Palette[index].peGreen = tempPal[index].peGreen;
            Palette[index].peBlue  = tempPal[index].peBlue;
            Palette[index].peFlags = tempPal[index].peFlags;
        }
    }
    
    // Get our render object ready. Tell the render object what distance
    // calculation to do : RGB/LUV/ALPHA - this is a little hackey, 
    // if the quantizer is doing alpha quantization, then the distancetype
    // must be ALPHA, so just set it.
    if(inFlags[0].inQuantizeFlag == ALPHA)
        inFlags[0].inDistanceType = ALPHA;
    render testRender(Palette, inFlags[0].inDistanceType);



    printf("Remapping : ");
    // Scan through the file list, and render those that need it.
    for(index = 0;index < numFlags;index++)
    {
        if(inFlags[index].inRenderFlag == True)
        {
            // If we'ere going to remap it, load it.
            imageArray[index].Load();

            // the imageArray has found the height and width of each image, 
            // the render class needs this information, so set the renderFlags 
            // appropriately now.
            renderInfo[index].inWidth  = imageArray[index].GetWidth();
            renderInfo[index].inHeight = imageArray[index].GetHeight();

            // Set the palette to remap to.
            imageArray[index].SetPalette(Palette);

//            printf("Remapping : %s\n",inFlags[index].inFilename);
            printf(".");


            // Set the relevant render data.
            testRender.SetData(imageArray[index].GetData8(), imageArray[index].GetData24(),
                           &renderInfo[index]);
            // do it.      
            testRender.RenderImage();

            // save it as an 8-bit bitmap with the name given in fileFlags.
            imageArray[index].MakeSaveName(inFlags[index].inFilename);
            imageArray[index].SetFilename(inFlags[index].inFilename);
            imageArray[index].SetType(BMP8);
            imageArray[index].Save();
            imageArray[index].ReleaseData();

        }
    }
    printf("\nDone\n");
    return 0;
}


//================================================================================================
// Extra C style functions used in main only.
//================================================================================================



//---------------------------------------------------------------------------
//
//  Function Name: BlendPalettes
//                 
//  Arguments    : an array of allFlags, quantizeFlags, and imageFile.
//                 
//
//  Comments     : This does the palette blending, mixing the alpha palette
//                 and the RGB palette together, choosing colors and then
//                 putting those into a specified range of the game palette.
//
//
//---------------------------------------------------------------------------


void BlendPalettes(allFlags *prgmFlags, quantizeFlags *qFlags, imageFile *imageArray)
{

    COUNTER first       = prgmFlags[0].inPaletteRangeFirst;
    COUNTER last        = prgmFlags[0].inPaletteRangeLast;
    COUNTER renderFirst = prgmFlags[0].inRenderRangeFirst;
    COUNTER renderLast  = prgmFlags[0].inRenderRangeLast;
    PALETTEENTRY Palette[256];
    COUNTER index;
        
    for(index = 0;index < 256;index++)
    {
        Palette[index].peRed    = 0;
        Palette[index].peGreen  = 0;
        Palette[index].peBlue   = 0;
        Palette[index].peFlags  = 0;
    }

    char *test = new char[256];
    
    pca_quantize blendQuantize(RGB);

    imageFile RGBPaletteFile;
    imageFile RGBAPaletteFile;

    RGBPaletteFile.SetType(PAL);
    RGBAPaletteFile.SetType(PAL);
    RGBPaletteFile.SetFilename(prgmFlags[0].inFilename);
    RGBAPaletteFile.SetFilename(prgmFlags[1].inFilename);

    RGBPaletteFile.Load();
    RGBAPaletteFile.Load();
    blendQuantize.SetFlags(&qFlags[0]);
    blendQuantize.AddPaletteColors(RGBPaletteFile.ReturnPal(), 
                                   first, last, renderFirst, renderLast);
    blendQuantize.BlendTransluscent(RGBPaletteFile.ReturnPal(),
                                    RGBAPaletteFile.ReturnPal(),
                                    prgmFlags[0].inBlendFirst,
                                    prgmFlags[1].inBlendLast);
                
    blendQuantize.GetPalette(Palette, first, last);
            
    RemakeFixedColors(Palette, RGBPaletteFile.ReturnPal(),first, last, renderFirst, renderLast);
    RGBPaletteFile.ReleaseData();
    RGBAPaletteFile.ReleaseData();
    
    // So, we've quantized and restored the palette to its
    // initial state with the new colors in place, now we
    // save the new game palette off and the two palettes
    // (new game palette and transluscency palette) are ready
    // to be merged with mpal.

    strcpy(test,prgmFlags[0].inPalOutput);
    
    imageArray[0].MakeSaveName(test);
    imageFile PalSave;
    PalSave.SetType(PAL);
    PalSave.SetFilename(test);
    printf("Saving Palette to : %s\n",test);
    PalSave.SetPalette(Palette);
    PalSave.Save(); 
    delete [] test;
}


//---------------------------------------------------------------------------
//
//  Function Name: InitFlags
//                 
//  Arguments    : ptrs to arrays of the four flag types, and the number of
//                 entries in each.
//
//  Comments     : Whips through setting the default values for each.
//                 
//                 
//
//
//---------------------------------------------------------------------------


void InitFlags(allFlags *prgmFlags, 
               renderFlags *rFlags, 
               quantizeFlags *qFlags, 
               fileFlags *fFlags,
               COUNTER numberofFlags)
{
    COUNTER index;

    // We need to set the flags to their initial values
    for(index = 0;index < numberofFlags;index++)
    {
        prgmFlags[index].inDistanceType      = RGB;
        prgmFlags[index].inWeight            = 1;
        prgmFlags[index].inDitherFlag        = True;
        prgmFlags[index].inQuantizeFlag      = RGB;
        prgmFlags[index].inRenderFlag        = False;
        prgmFlags[index].inRenderRangeFirst  = 10;
        prgmFlags[index].inRenderRangeLast   = 245;
        prgmFlags[index].inPaletteRangeFirst = 0;
        prgmFlags[index].inPaletteRangeLast  = 0;
        prgmFlags[index].inShadeTowards      = NULL;
        prgmFlags[index].inShadeIntensity    = 0;
        prgmFlags[index].inHazeTowards       = NULL;
        prgmFlags[index].inHazeIntensity     = NULL;
        prgmFlags[index].inFilename          = NULL;
        prgmFlags[index].inOutputPath        = new char[256];
        prgmFlags[index].inIgnoreList        = NULL;
        prgmFlags[index].numIgnores          = 0;
        prgmFlags[index].inZeroColor         = NULL;
        prgmFlags[index].inPalOutput         = NULL;
        prgmFlags[index].weightFlag          = True;
        prgmFlags[index].inPalOutput         = new char[256];
        prgmFlags[index].inChooseFlag        = True;
        prgmFlags[index].inTolerance         = 192;
        prgmFlags[index].inZeroFlag          = False;
        prgmFlags[index].inBlendFirst        = BLENDOFF;
        prgmFlags[index].inBlendLast         = BLENDOFF;
        prgmFlags[index].inHazeLevels        = 4;
        prgmFlags[index].inShadeLevels       = 4;


        rFlags[index].inDitherFlag       = True;
        rFlags[index].inRenderRangeFirst = 10;
        rFlags[index].inRenderRangeLast   = 245;
        rFlags[index].inZeroColor        = NULL;
        rFlags[index].inHeight           = 0;
        rFlags[index].inWidth            = 0;
        rFlags[index].inTolerance        = 192;

        qFlags[index].inIgnoreList     = NULL;
        qFlags[index].numIgnores       = 0;
        qFlags[index].inZeroColor      = NULL;

        fFlags[index].inFilename           = NULL;
        fFlags[index].inOutputPath         = NULL;
    }

}


//---------------------------------------------------------------------------
//
//  Function Name: CopyAuxFlags
//                 
//  Arguments    : ptrs to arrays of the four flag types, and the number of
//                 entries in each.
//
//  Comments     : Assumes that the prgmFlags have been set (by the parser),
//                 now we want to propagate the information to the other
//                 flag arrays.
//
//
//---------------------------------------------------------------------------

void CopyAuxFlags(allFlags *prgmFlags, 
               renderFlags *rFlags, 
               quantizeFlags *qFlags, 
               fileFlags *fFlags,
               COUNTER numberofFlags)
{
    COUNTER index;
    COUNTER indexx;

    // Copy the information into the class-specific flag structurs - being careful
    // to allocate when necessary.
    for(index = 0;index < numberofFlags;index++)
    {
        rFlags[index].inDitherFlag      = prgmFlags[index].inDitherFlag;
        rFlags[index].inTolerance       = prgmFlags[index].inTolerance;
        rFlags[index].inRenderRangeFirst= prgmFlags[index].inRenderRangeFirst;
        rFlags[index].inRenderRangeLast = prgmFlags[index].inRenderRangeLast;

        if(prgmFlags[index].inZeroColor)
        {
            rFlags[index].inZeroColor = new PALETTEENTRY;
            qFlags[index].inZeroColor = new PALETTEENTRY;
            rFlags[index].inZeroColor->peRed   = prgmFlags[index].inZeroColor->peRed;
            rFlags[index].inZeroColor->peGreen = prgmFlags[index].inZeroColor->peGreen;
            rFlags[index].inZeroColor->peBlue  = prgmFlags[index].inZeroColor->peBlue;
            rFlags[index].inZeroColor->peFlags = prgmFlags[index].inZeroColor->peFlags;

            qFlags[index].inZeroColor->peRed   = prgmFlags[index].inZeroColor->peRed;
            qFlags[index].inZeroColor->peGreen = prgmFlags[index].inZeroColor->peGreen;
            qFlags[index].inZeroColor->peBlue  = prgmFlags[index].inZeroColor->peBlue;
            qFlags[index].inZeroColor->peFlags = prgmFlags[index].inZeroColor->peFlags;
        }

        if(prgmFlags[index].inIgnoreList)
        {
            qFlags[index].numIgnores    = prgmFlags[index].numIgnores;
            qFlags[index].inIgnoreList  = new PALETTEENTRY[prgmFlags[index].numIgnores];
            for(indexx = 0;indexx < qFlags[index].numIgnores;indexx++)
            {
                qFlags[index].inIgnoreList[indexx].peRed   = prgmFlags[index].inIgnoreList[indexx].peRed;
                qFlags[index].inIgnoreList[indexx].peGreen = prgmFlags[index].inIgnoreList[indexx].peGreen;
                qFlags[index].inIgnoreList[indexx].peBlue  = prgmFlags[index].inIgnoreList[indexx].peBlue;
                qFlags[index].inIgnoreList[indexx].peFlags = prgmFlags[index].inIgnoreList[indexx].peFlags;
            }
        }
        fFlags[index].inFilename = new char[256];
        strcpy(fFlags[index].inFilename, prgmFlags[index].inFilename);
        fFlags[index].inOutputPath = new char[256];
        strcpy(fFlags[index].inOutputPath, prgmFlags[index].inOutputPath);
    }
}


//---------------------------------------------------------------------------
//
//  Function Name: SHImageCompare
//                 
//  Arguments    : *image1 and *image2 are imageFlag structs.
//                 
//                 
//                 
//                 
//  Comments     : This function is used by qsort to make sure that
//                 the shade/haze images are done first, that way the
//                 superfast shade/haze scan can go, based only on the
//                 colors in the popularity buffer.
//---------------------------------------------------------------------------

#ifdef MSVC
signed int __cdecl SHImageCompare(const void *image1, const void *image2)
#else
signed int SHImageCompare(const void *image1, const void *image2)
#endif
{
    float           value1;
    float           value2;
    signed int      retVal;
    allFlags    *flag1;
    allFlags    *flag2;
     
    flag1 = (allFlags *) image1;
    flag2 = (allFlags *) image2;

    if((flag1->inShadeTowards) || (flag1->inHazeTowards))
        value1 = 1;
    else
        value1 = 0;

    if((flag2->inShadeTowards) || (flag2->inHazeTowards))
        value2 = 1;
    else
        value2 = 0;

    retVal = (signed int)(value2 - value1);
    return retVal;
}
