//---------------------------------------------------------------------------
//
//
//  Filename   : makepal.cpp
//
//
//  Description: This file contains a number of functions that don't really
//               fit into a class - the RemakeFixedColors function takes
//               the palette output by the quantizer and a fixed palette
//               and essentially reconstructs the entries so that the final
//               output palette has only the specified CHOOSE entries modified.
//               It requires a FindClosestMatch function, which is identical
//               to the one found in the Render class, but must be repeated
//               here - as well as ColorDifference. 
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


//---------------------------------------------------------------------------
//
//  Function Name: RemakeFixedColors
//
//  Arguments    : inPal is the palette that PCA has made via a call to 
//                 GetPalette: fixedPal is the fixed palette, and the
//                 two ranges specified indicate the fixed color range. Everything
//                 not in the choose range, but in the render range is
//                 considered to be fixed.
//
//
//  Comments     : Goes through the inPal looking for the fixed entries
//                 - which should be there because of their large weighting
//                 going into the quantizer. The final palette goes in
//                 inPal. The fixed colors are replaced within the render range,
//                 discarding the chooserange, where the actual chosen colors
//                 go. The inPal pointer is a pointer to a palette which was
//                 output by GetPalette in the quantize class, this means it
//                 is simply a list from 0->TotalColors, and is not in the
//                 correct range.
//---------------------------------------------------------------------------

void RemakeFixedColors(PALETTEENTRY *inPal, PALETTEENTRY *inFixedPal,
                       COUNTER ChooseFirst, COUNTER ChooseLast,
                       COUNTER RenderFirst, COUNTER RenderLast)
{
    COUNTER         index;
    PALETTEENTRY    tempPal[256];
    PALETTEENTRY    inputPal[256];
    PALETTEENTRY    fixedPal[256];
    COUNTER         palIndex;
        

    // It's possible we don't have a fixed palette, we may be building from
    // scratch, in which case we just need to stick colors in the choose
    // range and be done.

    if(inFixedPal == NULL)
    {
        printf("No fixed colors - rendering from bitmap palette\n");
        // zero the working palette
        for(index = 0;index < 256;index++)
        {
            tempPal[index].peRed   = 0;
            tempPal[index].peGreen = 0;
            tempPal[index].peBlue  = 0;
            tempPal[index].peFlags = 0;
        }   
        // copy the input palette into the working palette in the correct
        // range.
        for(index = ChooseFirst; index <= ChooseLast; index++)
        {
            tempPal[index].peRed   = inPal[index - ChooseFirst].peRed;
            tempPal[index].peGreen = inPal[index - ChooseFirst].peGreen;
            tempPal[index].peBlue  = inPal[index - ChooseFirst].peBlue;
            tempPal[index].peFlags = inPal[index - ChooseFirst].peFlags;
        }
        
        // Copy the tempPal back to the inPal, and set the 255 entry to white
        for(index = 0; index < 256; index++)
        {
            inPal[index].peRed   = tempPal[index].peRed;
            inPal[index].peGreen = tempPal[index].peGreen;
            inPal[index].peBlue  = tempPal[index].peBlue;
            inPal[index].peFlags = tempPal[index].peFlags;
        }

        qsort(inPal + ChooseFirst, ChooseLast - ChooseFirst + 1, sizeof(PALETTEENTRY), 
              CompareIntensity);
        inPal[0].peRed   = 0;
        inPal[0].peGreen = 0;
        inPal[0].peBlue  = 0;
        inPal[0].peFlags = 0;

        return;
    }

    // If we have a fixed palette zero the function's internal 
    // working palette :
    for(index = 0;index < 256;index++)
    {
        tempPal[index].peRed    = 0;
        tempPal[index].peGreen  = 0;
        tempPal[index].peBlue   = 0;
        tempPal[index].peFlags  = 0;

        inputPal[index].peRed   = inPal[index].peRed;
        inputPal[index].peGreen = inPal[index].peGreen;
        inputPal[index].peBlue  = inPal[index].peBlue;
        inputPal[index].peFlags = inPal[index].peFlags;

        // If the entry does not exist, I can use 0,0,0 here because the tempPal
        // is zeroed, and a zero entry in the fixed pal will not get put into the
        // tempPal - i.e. the entry in the tempPal will be blank BUT since tempPal
        // is zeroed, we're in good shape, because that blank entry will have
        // the value 0,0,0!
        if((inFixedPal[index].peRed == 0) && (inFixedPal[index].peGreen == 0)
                && (inFixedPal[index].peBlue == 0))
        {
            fixedPal[index].peRed   = 0;
            fixedPal[index].peGreen = 0;
            fixedPal[index].peBlue  = 0;
            fixedPal[index].peFlags = 0;
        }
        else
        // If the entry is non-zero, copy it into the internal fixedPal.
        {
            fixedPal[index].peRed   = inFixedPal[index].peRed;
            fixedPal[index].peGreen = inFixedPal[index].peGreen;
            fixedPal[index].peBlue  = inFixedPal[index].peBlue;
            fixedPal[index].peFlags = inFixedPal[index].peFlags;
        }
    }


    // All of the above is setup, now check to see if the render range starts
    // before the choose range, if it does, we need to go from RenderFirst->
    // ChooseFirst - 1, putting the original fixed colors back into the 
    // final palette. Each time a color is used, it is deleted from the inputPal
    // by setting the RGB triple at the index in question to 0,0,0.

    if(RenderFirst < ChooseFirst)
    {
        for(index = RenderFirst;index < ChooseFirst;index++)
        {
            palIndex = FindClosestMatch(&fixedPal[index], inputPal);            
            if(ColorDifference(&fixedPal[index], &inputPal[palIndex]) > 42)
            {
//              printf("Not finding index : %d from fixedPal in output pal\n",index);
            }
        
            // Peg to original values!
            tempPal[index].peRed   = fixedPal[index].peRed;
            tempPal[index].peGreen = fixedPal[index].peGreen;
            tempPal[index].peBlue  = fixedPal[index].peBlue;
            tempPal[index].peFlags = fixedPal[index].peFlags;
        
            inputPal[palIndex].peRed    = 0;
            inputPal[palIndex].peGreen  = 0;
            inputPal[palIndex].peBlue   = 0;
            inputPal[palIndex].peFlags  = 0;
        }
    }

    // If the renderlast value is after the choose range, we have to fix those colors,
    // too, and so we go from one after the last chosen index to the last render index,
    // putting back the fixedPal colors.

    if(RenderLast > ChooseLast)
    {
        for(index = ChooseLast + 1;index <= RenderLast;index++)
        {
            palIndex = FindClosestMatch(&fixedPal[index], inputPal);            
            if(ColorDifference(&fixedPal[index], &inputPal[palIndex]) > 42)
            {
//              printf("Not finding index : %d from fixedPal in output pal\n",index);
            }
            // Copy the color into the appropriate range 

            tempPal[index].peRed   = fixedPal[index].peRed;
            tempPal[index].peGreen = fixedPal[index].peGreen;
            tempPal[index].peBlue  = fixedPal[index].peBlue;
            tempPal[index].peFlags = fixedPal[index].peFlags;
            
            inputPal[palIndex].peRed    = 0;
            inputPal[palIndex].peGreen  = 0;
            inputPal[palIndex].peBlue   = 0;
            inputPal[palIndex].peFlags  = 0;
        }   
    }

    // Now we have fixed all the colors that need to remain fixed, and we're ready
    // to put into the palette the new, chosen colors. They will be the last non-
    // zero entries in the inputPal. We'll whip through the inputPal, and pull 
    // out all non-zero entries, putting them into the palette at the first
    // chosen index.

    palIndex = ChooseFirst;
    for(index = 0;index < 256;index++)
    {
        // This ne zero is because we've been getting rid of the palette
        // as we went, only the nonzero entries are left over. 
        if((inputPal[index].peRed != 0)   || 
           (inputPal[index].peGreen != 0) || 
           (inputPal[index].peBlue != 0)  || 
           (inputPal[index].peFlags != 0))
        {
            tempPal[palIndex].peRed   = inputPal[index].peRed;
            tempPal[palIndex].peGreen = inputPal[index].peGreen;
            tempPal[palIndex].peBlue  = inputPal[index].peBlue;
            tempPal[palIndex].peFlags = inputPal[index].peFlags;

            palIndex++;
            inputPal[index].peRed   = 0;
            inputPal[index].peGreen = 0;
            inputPal[index].peBlue  = 0;
            inputPal[index].peFlags = 0;
        }
        if(palIndex == (ChooseLast + 1)) 
            break;

    }

    // The final palIndex should be equal to the the last chosen index +1.

    if(palIndex != (ChooseLast + 1))
    {
        printf("Warning - not enough colors, padding to choose length\n");
        AssertMessage(0, avar("Warning - not enough colors, padding to choose length\n"));
    }

    // Copy the working palette back into the output palette.
    for(index = 0;index < 256;index++)
    {
        inPal[index].peRed   = tempPal[index].peRed;
        inPal[index].peGreen = tempPal[index].peGreen;
        inPal[index].peBlue  = tempPal[index].peBlue;
        inPal[index].peFlags = tempPal[index].peFlags;
    }
    
    qsort(inPal + ChooseFirst, ChooseLast - ChooseFirst + 1, sizeof(PALETTEENTRY), 
          CompareIntensity);

//    for(index = 0; index < 10; index++)
//    {
//        inPal[index].peRed   = inFixedPal[index].peRed;
//        inPal[index].peGreen = inFixedPal[index].peGreen;
//        inPal[index].peBlue  = inFixedPal[index].peBlue;
//    }
//
//    for(index = 246;index < 256;index++)
//    {
//        inPal[index].peRed   = inFixedPal[index].peRed;
//        inPal[index].peGreen = inFixedPal[index].peGreen;
//        inPal[index].peBlue  = inFixedPal[index].peBlue;
//    }
   for (index = 0; index < 256; index++) {
      if ((index < ChooseFirst || index > ChooseLast) &&
          (index < RenderFirst || index > RenderLast)) {
         inPal[index].peRed   = fixedPal[index].peRed;
         inPal[index].peGreen = fixedPal[index].peGreen;
         inPal[index].peBlue  = fixedPal[index].peBlue;
         inPal[index].peFlags = fixedPal[index].peFlags;
      }
   }
}


//---------------------------------------------------------------------------
//
//  Function Name: ColorDifference
//
//  Arguments    : colorPtr1 and 2 are colors of which to find the diff.
//                 
//                 
//  Comments     : just like the render:: function of the same name, w/out
//                 the LUV stuff. I'm not sure where to put this set of 
//                 functions, so I am just leaving them here for now.
//                 
//
//
//---------------------------------------------------------------------------

ULONG ColorDifference(PALETTEENTRY *colorPtr1, PALETTEENTRY *colorPtr2)
{
    SLONG db, dg, dr, da;
    ULONG diff;
    SLONG r0,r1,g0,g1,b0,b1,a0,a1;
    r0 = colorPtr1->peRed    & 0xfe;
    g0 = colorPtr1->peGreen  & 0xfe;
    b0 = colorPtr1->peBlue   & 0xfe;
    a0 = colorPtr1->peFlags  & 0xfe;

    r1 = colorPtr2->peRed   & 0xfe;
    g1 = colorPtr2->peGreen & 0xfe;
    b1 = colorPtr2->peBlue  & 0xfe;
    // peFlags will be zero if not an alpha palette.
    a1 = colorPtr2->peFlags  & 0xfe;

    dr = r1 - r0;
    dr *= dr;
    dg = g1 - g0;
    dg *= dg;
    db = b1 - b0;
    db *= db;
    da = a1 - a0;
    da *= da;

    diff = da + dg + dr + db;
    return diff;
}   


//---------------------------------------------------------------------------
//
//  Function Name: FindClosestMatch
//
//  Arguments    : targetColor is the color to match, checkPal is the 
//                 palette in which to match it.
//                 
//  Comments     : just like the render:: function of the same name. Returns
//                 an index into the checkPal.
//                 
//                 
//
//
//---------------------------------------------------------------------------

UBYTE FindClosestMatch(PALETTEENTRY *targetColor, PALETTEENTRY *checkPal)
{
    COUNTER index;
    ULONG closestValue = 10000000;
    UBYTE closestIndex = 10;
    ULONG RGBDistance;

    for(index = 0;index < 256;index++)
    {
        RGBDistance = ColorDifference(targetColor, &checkPal[index]);
        if(RGBDistance < closestValue)
        {
            closestValue = RGBDistance;
            closestIndex = (UBYTE) index;
        }
    }
    return closestIndex;
}           


//---------------------------------------------------------------------------
//
//  Function Name: CompareIntensity
//                 
//  Arguments    : Color1 and Color2 are PALETTEENTRY pointers to be compared.
//                 Finds the greyscale intensity and returns 0 if equal, 
//                 positive if Color2 is more intense than Color1 or negative
//                 if Color1 is more intense than Color2.
//                 
//  Comments     : This is the function used by qsort to sort the palette.
//                 
//
//
//---------------------------------------------------------------------------

#ifndef MSVC
signed int CompareIntensity(const void *vColor1, const void *vColor2)
#else
signed int __cdecl CompareIntensity(const void *vColor1, const void *vColor2)
#endif
{
    float           intensity1;
    float           intensity2;
    signed int      retVal;
    PALETTEENTRY    *Color1;
    PALETTEENTRY    *Color2;
     
    Color1 = (PALETTEENTRY *) vColor1;
    Color2 = (PALETTEENTRY *) vColor2;

    intensity1 = .299f * ((float)(Color1->peRed)) + .587f * ((float)(Color1->peGreen)) 
                 + .114f * ((float)(Color1->peBlue));

    intensity2 = .299f * ((float)(Color2->peRed)) + .587f * ((float)(Color2->peGreen)) 
                 + .114f * ((float)(Color2->peBlue));

    retVal = (signed int)(intensity1 - intensity2);
    return retVal;
}
