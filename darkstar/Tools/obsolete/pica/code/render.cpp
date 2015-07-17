//---------------------------------------------------------------------------
//
//
//  Filename   : render.cpp
//
//
//  Description: The render class definition. Render class takes 24 bit
//               data, and a Palette, and makes 8-bit data, doing either
//               a straight remap or dithering. The palette range and 
//               dither flag are set in the imageFlags struct, so in 
//               addition to image data, the render class needs to know what
//               the particulars of each file are. The functions in render can
//               do RGB/LUV and RGBA distance matching, in addition to the
//               RGBW weighting. 
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
#include "luvconv.h"
#include "pcatypes.h"
#include "render.h"


//---------------------------------------------------------------------------
//
//  Function Name: render
//
//  Arguments    : inPal is the palette from which to render, inType
//                 tells which color distance calculation to use:
//                 it can be RGB/LUV or ALPHA.
//                 
//
//  Comments     : Initializes members - RGB/LUV can use lookup table.
//                 The Lookup table is zeroed. ALPHA is too large to 
//                 use lookups.
//
//
//---------------------------------------------------------------------------

render::render(PALETTEENTRY *inPal, TYPE inType)
{
    COUNTER index;
    ULONG testSize = COLORSPACESIZE;
    
    // Set the internal RenderPal to the input palette.
    for(index = 0;index < 256;index++)
    {
        RenderPal[index].peRed   = inPal[index].peRed;
        RenderPal[index].peGreen = inPal[index].peGreen;
        RenderPal[index].peBlue  = inPal[index].peBlue;
        RenderPal[index].peFlags = inPal[index].peFlags;
    }
    
    // Initalize and zero the lookup table.
    Type        = inType;
    LookupTable = NULL;
    ZeroColor   = NULL;
    if((Type == RGB) || (Type == LUV) || (Type == RGBW))
    {
        LookupTable = new signed short[testSize];
        for(index = 0;index < testSize;index++)
        {
            LookupTable[index] = NULINDEX;
        }
    }

    // If we're going to be using LUV color distances, we need a 
    // version of the palette in LUV space.
    if(Type == LUV)
    {
        for(index = 0;index < 256;index++)
        {
            LUVRenderPal[index].R = inPal[index].peRed;
            LUVRenderPal[index].G = inPal[index].peGreen;
            LUVRenderPal[index].B = inPal[index].peBlue;
        }
        convertRGBtoLUV(LUVRenderPal,256);
    }
}


//---------------------------------------------------------------------------
//
//  Function Name: ~render
//
//  Arguments    : none
//                 
//                 
//                 
//
//  Comments     : deletes the lookup table.
//                 
//                 
//
//
//---------------------------------------------------------------------------

render::~render(void)
{
    if(LookupTable)
        delete [] LookupTable;
    if(ZeroColor)
        delete ZeroColor;
}


//---------------------------------------------------------------------------
//
//  Function Name: MakeLookup
//
//  Arguments    : none
//
//
//  Comments     : Inputs entries into a lookup table, this should be done
//                 every time new 24-bit data is entered. Only LUV/RGB
//                 do this.
//                 
//
//
//---------------------------------------------------------------------------

void render::MakeLookup(void)
{
    COUNTER         index;
    PALETTEENTRY    color;
    ULONG           memIndex;

    for(index = 0;index < Height*Width;index++)
    {
        color.peRed   = pData24[index].peRed;
        color.peGreen = pData24[index].peGreen;
        color.peBlue  = pData24[index].peBlue;
        color.peFlags = 0;

        memIndex=(((ULONG)(color.peGreen)>>1)<<14)+(((ULONG)(color.peRed)>>1)<<7)+
                ((ULONG)(color.peBlue)>>1);

        if(LookupTable[memIndex] == NULINDEX)
        {
            LookupTable[memIndex] = FindClosestMatch(&color);
        }
    }
}


//---------------------------------------------------------------------------
//
//  Function Name: SetData
//
//  Arguments    : inpData8 - pointer to 8-bit data
//                 inpData24 - pointer to 24-bit data
//                 inFlags - the image flags, with info on ranges and dithering
//
//                 
//  Comments     : We need to be able to render different images w/ the
//                 same lookup data, so resetting the render data is important.
//                 
//                 
//
//---------------------------------------------------------------------------

void render::SetData(UBYTE * inpData8, PALETTEENTRY *inpData24, renderFlags *inFlags)                                     

{
    Height    = inFlags->inHeight;
    Width     = inFlags->inWidth;
    ImageSize = Height*Width;

    pData8  = inpData8;
    pData24 = inpData24;

    RangeFirst = inFlags->inRenderRangeFirst;
    RangeLast  = inFlags->inRenderRangeLast;
    DitherFlag = inFlags->inDitherFlag;

    // Make the lookup table, if RGB/LUV. This  saves us time later.
    if((Type == LUV) || (Type == RGB) || (Type == RGBW))
    {
        MakeLookup();
    }

    if(inFlags->inZeroColor != NULL)
    {
        ZeroColor         = new PALETTEENTRY;
        ZeroColor->peRed  = inFlags->inZeroColor->peRed;
        ZeroColor->peGreen= inFlags->inZeroColor->peGreen;
        ZeroColor->peBlue = inFlags->inZeroColor->peBlue;
        ZeroColor->peFlags= inFlags->inZeroColor->peFlags;
    }
    // This is the "DITHERTOLERANCE", the value to be checked in distance
    // calculations to near pixels in the dithering process.
    tolerance = inFlags->inTolerance;

}

//---------------------------------------------------------------------------
//
//  Function Name: FindClosestMatch
//
//  Arguments    : colorPtr is a PALETTEENTRY color to match in the RenderPal
//
//
//  Comments     : Looks in the RenderPal in the render range (RangeFirst->
//                 RangeLast) and returns the palette index that is the
//                 best. Uses a weighted RGB Euclidean distance or a non-
//                 weighted LUV Euclidean distance or a 4-vector Euclidean
//                 distance (weighted) if ALPHA.
//
//---------------------------------------------------------------------------

UBYTE render::FindClosestMatch(PALETTEENTRY *colorPtr)
{
    COUNTER         closestIndex;
    double          closestValue;
    COUNTER         index;

    // Set the initial
    closestIndex = RangeFirst;
    closestValue=PalColorDifference(colorPtr,RangeFirst);

    // Go through the range, looking for palette 
    // entries that are closer than our initial seed.
    for(index = RangeFirst; index <= RangeLast;index++)
    {
        if(PalColorDifference(colorPtr,index) < closestValue)
        {
            closestIndex=index;
            closestValue=PalColorDifference(colorPtr,index);
        }
    }
    return (UBYTE)(closestIndex);
}


//---------------------------------------------------------------------------
//
//  Function Name: PalColorDifference
//
//  Arguments    : colorPtr is the color to match, palEntry is the palette
//                 entry number to compare with
//
//  Comments     : This is a little odd, one expects colorDifference to take
//                 two colors and return the diff, but because we have two
//                 different methods for doing this (LUV/RGB) I need that
//                 second entry to be a palette entry so I can use either the
//                 RenderPal color at location palEntry or the LUVRenderPal
//                 color at location palEntry.
//---------------------------------------------------------------------------

double render::PalColorDifference(PALETTEENTRY *colorPtr, COUNTER palIndex)
{
    SLONG   db, dg, dr, da;
    double  diff;
    SLONG   r0,r1,g0,g1,b0,b1,a0,a1;

    // The weighted Euclidean distance if we're dealing
    // with RGB or ALPHA
    if((Type == RGB) || (Type == ALPHA) || (Type == RGBW))
    {
        r0 = colorPtr->peRed   ;
        g0 = colorPtr->peGreen ;
        b0 = colorPtr->peBlue  ;
        a0 = colorPtr->peFlags ;
        
        r1 = RenderPal[palIndex].peRed   ;
        g1 = RenderPal[palIndex].peGreen ;
        b1 = RenderPal[palIndex].peBlue  ;
        // peFlags will be zero if not an alpha palette.
        a1 = RenderPal[palIndex].peFlags ;
        
        dr = r1 - r0;
        dr *= dr;
        dg = g1 - g0;
        dg *= dg;
        db = b1 - b0;
        db *= db;
        da = a1 - a0;
        da *= da;
        
        if(Type == RGBW) 
         diff = da + 6*dg + 3*dr + db;
        else
        if((Type == RGB) || (Type == ALPHA))
         diff = da + dg + dr + db;

        return diff;
    }

    // Otherwise, we want the LUV non-weighted Euclidean distance.
    COLOR inColor;
    inColor.R = (double)(colorPtr->peRed);
    inColor.G = (double)(colorPtr->peGreen);
    inColor.B = (double)(colorPtr->peBlue);
    convertRGBtoLUV(&inColor, 1);
    double deltaL;
    double deltaU;
    double deltaV;
    deltaL = abs(inColor.L - LUVRenderPal[palIndex].L);
    deltaU = abs(inColor.U - LUVRenderPal[palIndex].U);
    deltaV = abs(inColor.V - LUVRenderPal[palIndex].V);

    deltaL *= deltaL;
    deltaU *= deltaU;
    deltaV *= deltaV;

    diff = deltaL + deltaU + deltaV;
    return diff;
}   


//---------------------------------------------------------------------------
//
//  Function Name: ColorDifference
//
//  Arguments    : Two PALETTEENTRY pointers to compare
//                 
//
//  Comments     : Returns based on the current type - RGB/LUV the Euclidean
//                 distance or a weighted Euclidean distance (for RGB) between
//                 the two colors.
//                 
//                 
//                 
//---------------------------------------------------------------------------

double render::ColorDifference(PALETTEENTRY *colorPtr1, PALETTEENTRY *colorPtr2)
{
    SLONG   db, dg, dr, da;
    double  diff;
    SLONG   r0,r1,g0,g1,b0,b1,a0,a1;
    if((Type == RGB) || (Type == ALPHA) || (Type == RGBW))
    {
        r0 = colorPtr1->peRed    ;
        g0 = colorPtr1->peGreen  ;
        b0 = colorPtr1->peBlue   ;
        a0 = colorPtr1->peFlags  ;

        r1 = colorPtr2->peRed   ;
        g1 = colorPtr2->peGreen ;
        b1 = colorPtr2->peBlue  ;
        // peFlags will be zero if not an alpha palette.
        a1 = colorPtr2->peFlags  ;

        dr = r1 - r0;
        dr *= dr;
        dg = g1 - g0;
        dg *= dg;
        db = b1 - b0;
        db *= db;
        da = a1 - a0;
        da *= da;

        if(Type == RGBW) 
         diff = da + 6*dg + 3*dr + db;
        else
        if( (Type == RGB) || (Type == ALPHA))
         diff = da + dg + dr + db;
        return diff;
    }
    COLOR inColor1;
    COLOR inColor2;
    inColor1.R = (double)(colorPtr1->peRed);
    inColor1.G = (double)(colorPtr1->peGreen);
    inColor1.B = (double)(colorPtr1->peBlue);
    convertRGBtoLUV(&inColor1, 1);
    inColor2.R = (double)(colorPtr2->peRed);
    inColor2.G = (double)(colorPtr2->peGreen);
    inColor2.B = (double)(colorPtr2->peBlue);
    convertRGBtoLUV(&inColor2, 1);

    double deltaL;
    double deltaU;
    double deltaV;
    deltaL = abs(inColor2.L - inColor1.L);
    deltaU = abs(inColor2.U - inColor1.U);
    deltaV = abs(inColor2.V - inColor1.V);

    deltaL *= deltaL;
    deltaU *= deltaU;
    deltaV *= deltaV;

    diff = deltaL + deltaU + deltaV;
    return diff;
}   


//---------------------------------------------------------------------------
//
//  Function Name: RenderImage(void)
//
//  Arguments    : none
//
//
//  Comments     : Renders the data, if dithering is on, dithers, if not,
//                 it doesn't - it's a simple remap.
//
//
//
//
//---------------------------------------------------------------------------

void render::RenderImage(void)
{
    ULONG index;
    UBYTE palIndex;
    ULONG memIndex;

    if((Type == RGB) || (Type == RGBW))
    {
        if(DitherFlag == True)
        {
            Dither();
        }

        // Note that we are assured that all colors in the image are
        // in the LookupTable since for each image, we add its colors to
        // the lookup table before we get here.
        else
        {
            for(index = 0;index < ImageSize;index++)
            {
                memIndex=(((ULONG)(pData24[index].peGreen)>>1)<<14)+(((ULONG)(pData24[index].peRed)>>1)<<7)+
                          ((ULONG)(pData24[index].peBlue)>>1);

                if(IsZeroColor(&pData24[index]) == True)
                {
                    pData8[index] = 0;
                }
                else
                {
                    palIndex      = (UBYTE) LookupTable[memIndex];
                    pData8[index] = palIndex;
                }
            }
        }
    }
    else

    if(Type == LUV)
    {
        if(DitherFlag == True)
        {
            Dither();
        }
        else
        {

            for(index = 0;index < ImageSize; index++)
            {
                memIndex=(((ULONG)(pData24[index].peGreen)>>1)<<14)+(((ULONG)(pData24[index].peRed)>>1)<<7)+
                          ((ULONG)(pData24[index].peBlue)>>1);
                if(IsZeroColor(&pData24[index]) == True)
                {
                    pData8[index] = 0;
                }
                else
                {
                    palIndex      = (UBYTE) LookupTable[memIndex];
                    pData8[index] = palIndex;
                }
            }
        }
    }
    else
    // no lookup - no dither for ALPHA type.
    if(Type == ALPHA)
    {
        for(index = 0;index < ImageSize;index++)
        {
            if(IsZeroColor(&pData24[index]) == True)
            {
                pData8[index] = 0;
            }
            else
            {
                palIndex=FindClosestMatch(&pData24[index]);
                pData8[index] = palIndex;
            }
        }
    }
}


//---------------------------------------------------------------------------
//
//  Function Name: Dither
//
//  Arguments    : none - uses member functions that have been set with a call
//                 to SetData
//
//  Comments     : Floyd-Steinberg ditherer.
//                 
//                 
//                 
//
//
//---------------------------------------------------------------------------

void render::Dither(void)
{
    COUNTER index;
    COUNTER indexy;
    COUNTER indexx;
    FLOAT red, green, blue;
    FLOAT redErr, greenErr, blueErr;
    FLAG *noMap;
    FLOATCOLOR errorVals;
    PALETTEENTRY color;
    ULONG memIndex;
    UBYTE palIndex;

    noMap = new FLAG[Height*Width];

    for(index = 0;index < Height*Width;index++)
    {
        if(IsZeroColor(&pData24[index]) == True) 
            noMap[index] = True;
        else
            noMap[index] = False;
    }

    PALETTEENTRY *nodithData24 = new PALETTEENTRY[ImageSize];

    // Keep a copy of the non-dithered data for tolerance comparisons.
    for(index = 0;index < ImageSize;index++)
    {
        nodithData24[index].peRed   = pData24[index].peRed;
        nodithData24[index].peGreen = pData24[index].peGreen;
        nodithData24[index].peBlue  = pData24[index].peBlue;
        nodithData24[index].peFlags = 0;
        pData24[index].peFlags = 0;
    }   
    for(indexy = 0;indexy < (Height);indexy++)
    {
        for(indexx = 0;indexx < (Width);indexx++)    
        {
            
            if(noMap[indexy*Width + indexx] == True)
            {
                pData8[indexy*Width + indexx] = 0;
                continue;
            }
                                                
            color.peRed   = pData24[indexy*Width + indexx].peRed;
            color.peGreen = pData24[indexy*Width + indexx].peGreen;
            color.peBlue  = pData24[indexy*Width + indexx].peBlue;
            color.peFlags = 0;

            memIndex=(((ULONG)(color.peGreen)>>1)<<14)+(((ULONG)(color.peRed)>>1)<<7)+
                      ((ULONG)(color.peBlue)>>1);
            
                        
            if(LookupTable[memIndex] != NULINDEX)
            {
                palIndex = (UBYTE) LookupTable[memIndex];
            }
            else
            {
                palIndex = FindClosestMatch(&color);
                LookupTable[memIndex] = palIndex;
            }
            
            if((ColorDifference(&pData24[indexy*Width + indexx], &RenderPal[palIndex]) > tolerance) 
               || (SolidColor(nodithData24,indexx, indexy) == True))
            {
                color.peRed   = nodithData24[indexy*Width + indexx].peRed;
                color.peGreen = nodithData24[indexy*Width + indexx].peGreen;
                color.peBlue  = nodithData24[indexy*Width + indexx].peBlue;
                color.peFlags = 0;
                memIndex=(((ULONG)(color.peGreen)>>1)<<14)+(((ULONG)(color.peRed)>>1)<<7)+
                         ((ULONG)(color.peBlue)>>1);
            
               if(LookupTable[memIndex] != NULINDEX)
               {
                   palIndex = (UBYTE) LookupTable[memIndex];
               }
               else
               {
                  palIndex = FindClosestMatch(&color);
                  LookupTable[memIndex] = palIndex;
               }
            }

            pData8[indexy*Width + indexx] = palIndex;
            errorVals.R = color.peRed   - RenderPal[palIndex].peRed;
            errorVals.G = color.peGreen - RenderPal[palIndex].peGreen;
            errorVals.B = color.peBlue  - RenderPal[palIndex].peBlue;
            ErrorDistrib(&errorVals, indexx, indexy);
        }
    }

    delete [] noMap;
    delete [] nodithData24;
}




//---------------------------------------------------------------------------
//
//  Function Name: ErrorDistrib
//
//  Arguments    : *inError a pointer to FLOATCOLOR, with the error to distribute.
//                 xPos, yPos - the x and y positions, relative which to distribute.
//
//  Comments     : Floyd-Steinberg distribution.
//                 
//                 
//                 
//
//
//---------------------------------------------------------------------------

void render::ErrorDistrib(FLOATCOLOR *inError, COUNTER xPos, COUNTER yPos)
{
    float red, green, blue;

    // 5/16 to pixel below.
    if (yPos < Height - 1)
    {
        UpdatePixel(inError, xPos, (yPos + 1), float(5.0f/16.0f));  
    }
    
    // 7/16 to pixel on right.
    if (xPos < Width - 1)
    {
        UpdatePixel(inError, (xPos+1), yPos, float(7.0f/16.0f));    
    }
   
    // 1/16 to pixel on right and down
    if (xPos < Width - 1 && yPos < Height - 1)
    {
        UpdatePixel(inError, (xPos+1), (yPos+1), float(1.0f/16.0f));    
    }      

    // 3/16 to pixel on left and down
    if (xPos > 0 && yPos < Height - 1)
    {
        UpdatePixel(inError, (xPos-1), (yPos+1), float(3.0f/16.0f));    
    }      

}


//---------------------------------------------------------------------------
//
//  Function Name: UpdatePixel
//
//  Arguments    : inError is a pointer to the error vector to distribute to 
//                 the pixel at location xPos, yPos with multiplicative constant weight.
//
//  Comments     : Used by the ErrorDistrib function to assign the error values
//                 to neighbouring pixels with appropriate Floyd-Steinberg
//                 weights.
//                 
//
//
//---------------------------------------------------------------------------

void render::UpdatePixel(FLOATCOLOR *inError, COUNTER xPos, COUNTER yPos, float weight)
{
    float red, green, blue;
    ULONG index;

    float redErr, greenErr, blueErr;
    
    redErr   = inError->R;
    greenErr = inError->G;
    blueErr  = inError->B;    

    index = yPos*Width + xPos;

    red   = (pData24[index].peRed)   + (redErr*weight);
    green = (pData24[index].peGreen) + (greenErr*weight);
    blue  = (pData24[index].peBlue)  + (blueErr*weight);

    // Clamp values.
    if(red > 255.0f) red = 255.0f;
    if(red < 0.0f) red = 0.0f;

    if(blue > 255.0f) blue = 255.0f;
    if(blue < 0.0f) blue = 0.0f;

    if(green > 255.0f) green = 255.0f;
    if(green < 0.0f) green = 0.0f;

    // Reassign.
    pData24[index].peRed   = (UBYTE) red;
    pData24[index].peGreen = (UBYTE) green;
    pData24[index].peBlue  = (UBYTE) blue;
}


//---------------------------------------------------------------------------
//
//  Function Name: IsZeroColor
//
//  Arguments    : colorPtr is a pointer to the PALETTEENTRY to check.
//
//
//  Comments     : Checks to see if we should zero a color. True if the
//                 colorPtr is == ZeroColor, else False.
//                 
//                 
//
//
//---------------------------------------------------------------------------

FLAG render::IsZeroColor(PALETTEENTRY *colorPtr)
{
    if(ZeroColor == NULL) return False;
    if((colorPtr->peRed == ZeroColor->peRed) && (colorPtr->peGreen == ZeroColor->peGreen)
        && (colorPtr->peBlue == ZeroColor->peBlue))
        return True;
    else
        return False;
}


//---------------------------------------------------------------------------
//
//  Function Name: SolidColor
//
//  Arguments    : noDithData is a pointer to the raw, non-dithered 24-bit data,
//                 xPos, and yPos are the current buffer positions.
//
//  Comments     : This function is here to get rid of dither patterns on large,
//                 solid colored regions. It returns True if the 8 pixels surrounding
//                 the one at location xPos, yPos are IDENTICAL in color value, 
//                 else False.
//
//
//---------------------------------------------------------------------------

FLAG render::SolidColor(PALETTEENTRY *noDithData, COUNTER xPos, COUNTER yPos)
{
    ULONG bufIndex;
    ULONG refIndex;
    FLAG returnValue = True;
    refIndex = yPos*Width + xPos;

    // Check pixel to left
    if(xPos > 0)
    {
        bufIndex = yPos*Width + (xPos-1);
        if( (noDithData[refIndex].peRed   != noDithData[bufIndex].peRed)
          ||(noDithData[refIndex].peGreen != noDithData[bufIndex].peGreen)
          ||(noDithData[refIndex].peBlue  != noDithData[bufIndex].peBlue))
        {
            returnValue = False;
        }
    }

    // Check pixel to upper left
    if((xPos > 0) && (yPos > 0))
    {
        bufIndex = (yPos - 1)*Width + (xPos-1);
        if( (noDithData[refIndex].peRed   != noDithData[bufIndex].peRed)
          ||(noDithData[refIndex].peGreen != noDithData[bufIndex].peGreen)
          ||(noDithData[refIndex].peBlue  != noDithData[bufIndex].peBlue))
        {
            returnValue = False;
        }
    }

    // Check pixel above
    if(yPos > 0)
    {
        bufIndex = (yPos - 1)*Width + (xPos);
        if( (noDithData[refIndex].peRed   != noDithData[bufIndex].peRed)
          ||(noDithData[refIndex].peGreen != noDithData[bufIndex].peGreen)
          ||(noDithData[refIndex].peBlue  != noDithData[bufIndex].peBlue))
        {
            returnValue = False;
        }
    }

    // Check pixel to upper right
    if((xPos < Width - 1) && (yPos > 0))
    {
        bufIndex = (yPos - 1)*Width + (xPos+1);
        if( (noDithData[refIndex].peRed   != noDithData[bufIndex].peRed)
          ||(noDithData[refIndex].peGreen != noDithData[bufIndex].peGreen)
          ||(noDithData[refIndex].peBlue  != noDithData[bufIndex].peBlue))
        {
            returnValue = False;
        }
    }
        
    // Check pixel on right
    if(xPos < Width - 1)
    {
        bufIndex = (yPos)*Width + (xPos+1);
        if( (noDithData[refIndex].peRed   != noDithData[bufIndex].peRed)
          ||(noDithData[refIndex].peGreen != noDithData[bufIndex].peGreen)
          ||(noDithData[refIndex].peBlue  != noDithData[bufIndex].peBlue))
        {
            returnValue = False;
        }
    }

    // Check pixel on right and down
    if((xPos < Width - 1) && (yPos < Height - 1))
    {
        bufIndex = (yPos+1)*Width + (xPos+1);
        if( (noDithData[refIndex].peRed   != noDithData[bufIndex].peRed)
          ||(noDithData[refIndex].peGreen != noDithData[bufIndex].peGreen)
          ||(noDithData[refIndex].peBlue  != noDithData[bufIndex].peBlue))
        {
            returnValue = False;
        }
    }

    // Check pixel below
    if(yPos < Height - 1)
    {
        bufIndex = (yPos+1)*Width + (xPos);
        if( (noDithData[refIndex].peRed   != noDithData[bufIndex].peRed)
          ||(noDithData[refIndex].peGreen != noDithData[bufIndex].peGreen)
          ||(noDithData[refIndex].peBlue  != noDithData[bufIndex].peBlue))
        {
            returnValue = False;
        }
    }

    // Check pixel below and left
    if((xPos > 0) && (yPos < Height - 1))
    {
        bufIndex = (yPos+1)*Width + (xPos-1);
        if( (noDithData[refIndex].peRed   != noDithData[bufIndex].peRed)
          ||(noDithData[refIndex].peGreen != noDithData[bufIndex].peGreen)
          ||(noDithData[refIndex].peBlue  != noDithData[bufIndex].peBlue))
        {
            returnValue = False;
        }
    }

    return returnValue;
}