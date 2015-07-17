//---------------------------------------------------------------------------
//
//
//  Filename   : pcaquan.cpp
//
//
//  Description: The methods cons/destructor for the pca_quantize class.
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
#include <base.h>
#include "pcatypes.h"
#include "svector.h"
#include "pcaquan.h"
#include "luvconv.h"


//---------------------------------------------------------------------------
//
//  Function Name: pca_quantize
//
//  Arguments    : TYPE of quantizer (RGB, LUV, ALPHA)
//
//
//  Comments     : creates the correct colorData buffer (7-7-7 -> 2^21 for
//                 RGB/LUV and 5-5-5-5 -> 2^20 for ALPHA) and zeroes it.
//                 The counter and weight counter are both zeroed as well.
//                 
//
//
//---------------------------------------------------------------------------

pca_quantize::pca_quantize(TYPE inType)
{
    COUNTER index;
    
    Type = inType;
    if(Type == ALPHA)
    {
        DataSize = ALPHASPACESIZE;
    }
    else
    {
        DataSize = COLORSPACESIZE;
    }

    // ColorData is the popularity buffer.
    ColorData = new ULONG[DataSize];
    for(index = 0;index < DataSize;index++)
    {
        ColorData[index] = 0;
    }


    // This -1 "zeroing" is better than 0 "zeroing", because 0,0,0
    // is a possible palette entry. 
    for(index = 0;index < 256;index++)
    {
        FixedPalette[index].R = -1;
        FixedPalette[index].G = -1;
        FixedPalette[index].B = -1;
    }

    // NULL the internal member pointers and zero the internal member counters.
    ColorVectors    = NULL;
    OutVectors      = NULL;
    vecInfo         = NULL;
    vecInfoOut      = NULL;
    ignoreList      = NULL;
    ZeroColor       = NULL;
    FixedCounter    = 0;
    TotalWeight     = 0;
    ColorCount      = 0;
}


//---------------------------------------------------------------------------
//
//  Function Name: ~pca_quantize
//                 
//  Arguments    : none
//
//
//  Comments     : deletes the allocated pointer data.
//                 
//                 
//                 
//
//
//---------------------------------------------------------------------------

pca_quantize::~pca_quantize(void)
{
    if(ColorData)
        delete [] ColorData;
    if(vecInfo)
        delete [] vecInfo;
    if(vecInfoOut)
        delete [] vecInfoOut;
    if(ColorVectors)
        delete [] ColorVectors;
    if(OutVectors)
        delete [] OutVectors;
    if(ignoreList)
        delete [] ignoreList;
    if(ZeroColor)
        delete ZeroColor;
}


//---------------------------------------------------------------------------
//
//  Function Name: SetFlags
//                 
//  Arguments    : a pointer to a quantizeFlags structure.
//
//
//  Comments     : Sets the ignorelist, the size of the ignore list,
//                 and the zero color.
//                 
//                 
//
//
//---------------------------------------------------------------------------

void pca_quantize::SetFlags(quantizeFlags *inFlags)
{
    COUNTER index;
    
    ignoreListSize = inFlags->numIgnores;
    if(inFlags->inIgnoreList)
    {
        ignoreList = new PALETTEENTRY[ignoreListSize];
        for(index = 0;index < ignoreListSize;index++)
        {
            ignoreList[index].peRed    = inFlags->inIgnoreList[index].peRed;
            ignoreList[index].peGreen  = inFlags->inIgnoreList[index].peGreen;
            ignoreList[index].peBlue   = inFlags->inIgnoreList[index].peBlue;
            ignoreList[index].peFlags  = inFlags->inIgnoreList[index].peFlags;
        }
    }
    if(inFlags->inZeroColor)
    {
        ZeroColor = new PALETTEENTRY;
        ZeroColor->peRed   = inFlags->inZeroColor->peRed;
        ZeroColor->peGreen = inFlags->inZeroColor->peGreen;
        ZeroColor->peBlue  = inFlags->inZeroColor->peBlue;
        ZeroColor->peFlags = inFlags->inZeroColor->peFlags;
    }
}


//---------------------------------------------------------------------------
//
//  Function Name: ScanImage
//                 
//  Arguments    : *imageData is a pointer to the image buffer, 
//                 imageSize is the Height*Width of the image, imageWeight
//                 is its weight (default is 1).
//                 
//                 
//  Comments     : Scans through the image, updating the weights of the            
//                 colors in ColorData, and keeping count of the new 
//                 colors, only those colors not in the ignore list get
//                 counted. The ignorelist must be set with setFlags b4 this
//                 is called.
//---------------------------------------------------------------------------

void pca_quantize::ScanImage(PALETTEENTRY *imageData, ULONG imageSize, ULONG imageWeight)
{
    ULONG           memIndex;
    COUNTER         index;
    PALETTEENTRY    color;

    if(imageData == NULL)
    {
        printf("ERROR - load bitmap before scanning it!\n");
        AssertFatal(imageData,avar("Load Bitmap before Scanning!\n"));
        exit(0);
    }

    for(index = 0;index < imageSize;index++)
    {
        color.peBlue = imageData[index].peBlue;
        color.peGreen = imageData[index].peGreen;
        color.peRed = imageData[index].peRed;
        color.peFlags = imageData[index].peFlags;
        if(Type == RGB)
        {
            // imageWeight == 0 indicates that we have weighting turned off.
            // The RegisterColor function is also used for the shading/hazing,
            // where an imageWeight of 0 can crop up due to the linear falloff
            // used in hazing and shading, so CONSTWEIGHT is a negative number
            // that will be checked for in RegisterColor, and acted upon appropriately.
            if(imageWeight == 0)
                RegisterColor(&color, CONSTWEIGHT);
            else
                RegisterColor(&color, imageWeight);
        }
        else
        if(Type == ALPHA)
        {
        
            if(imageWeight == 0)
                RegisterAlphaColor(&color, CONSTWEIGHT);
            else
                RegisterAlphaColor(&color, imageWeight);
//            memIndex = (((ULONG)(color.peFlags)>>3)<<15) + (((ULONG)(color.peGreen)>>3)<<10)
//                       + (((ULONG)(color.peRed)>>3)<<5) + ((ULONG)(color.peBlue)>>3);
//            if(ColorData[memIndex] == 0)
//            {
//                ColorCount++;
//                if(imageWeight == 0)
//                {
//                    ColorData[memIndex] = 1;
//                    TotalWeight++;
//                }
//                else
//                {
//                    ColorData[memIndex]+=(ULONG) imageWeight;
//                    TotalWeight += (ULONG) imageWeight;
//                }
//            }
        }
    }
    // end of function.
}


//---------------------------------------------------------------------------
//
//  Function Name: IsInIgnoreList
//                 
//  Arguments    : checkColor is a pointer to the color to check, 
//                 ignoreList is the list of colors to ignore, and
//                 ignoreListSize is its number of members.
//                 
//                 
//  Comments     : Just whips through. True on ignore entry or ZeroColor.
//                 
//
//
//---------------------------------------------------------------------------

FLAG pca_quantize::IsInIgnoreList(PALETTEENTRY *checkColor)
{
    COUNTER index;
    // If specifically ignored
    if(ignoreList != NULL) 
    {
        for(index = 0;index < ignoreListSize;index++)
        {
            if((checkColor->peRed    == ignoreList[index].peRed) &&
                (checkColor->peGreen == ignoreList[index].peGreen) &&
                (checkColor->peBlue  == ignoreList[index].peBlue) &&
                (checkColor->peFlags == ignoreList[index].peFlags))
            {
                return True;
            }
        }
    }
    // Or it's the zero color!
    if(ZeroColor != NULL)
    {
        if((checkColor->peRed    == ZeroColor->peRed) &&
            (checkColor->peGreen == ZeroColor->peGreen) &&
            (checkColor->peBlue  == ZeroColor->peBlue))
        {
            return True;
        }
    }

    return False;
}


//---------------------------------------------------------------------------
//
//  Function Name: GetPalette
//                 
//  Arguments    : outPal is the palette to be filled, and it will have
//                 colors input from 0-> totalColors, in no order!
//                 
//                 
//                 
//  Comments     : makes the input vectors and quantizes. Then writes the
//                 vectors into the specified palette (which must already
//                 be allocated).
//
//---------------------------------------------------------------------------

void pca_quantize::GetPalette(PALETTEENTRY *outPal, int first, int last)
{
    COUNTER index;
    ULONG   totalColors;

    // The base number, if no fixed colors, just select these
    totalColors = last - first + 1;
    // If there are fixed colors, however, we need to choose those, as
    // well!
    if(FixedCounter > 0)
    {
          totalColors += FixedCounter;
    }

    if(Type == RGB)
    {
        QuantizeRGB(totalColors);
    }
    if(Type == ALPHA)
    {
        QuantizeRGBA(totalColors);
    }
    
    // We've got them, now fill in outPal.
    for(index = 0;index < ColorCount;index++)
    {
        outPal[index].peRed   = OutVectors[index].pElem[0];
        outPal[index].peGreen = OutVectors[index].pElem[1];
        outPal[index].peBlue  = OutVectors[index].pElem[2];
        if(Type == ALPHA)
        {
            outPal[index].peFlags = OutVectors[index].pElem[3];
        }
        else
        {
            outPal[index].peFlags = 0;
        }
    }
    delete [] ColorData;
    ColorData = NULL;
    delete [] ColorVectors;
    ColorVectors = NULL;
    delete [] OutVectors;
    OutVectors = NULL;
}


//---------------------------------------------------------------------------
//
//  Function Name: QuantizeRGB
//                 
//  Arguments    : numOuts tells the quantizer how many vectors to output.
//                 
//                 
//                 
//                 
//  Comments     : makes the input vectors for RGB and quantizes.
//                 
//
//
//---------------------------------------------------------------------------

void pca_quantize::QuantizeRGB(int numOuts)
{
    COUNTER index;
    COUNTER paletteIndex;
    ULONG   r,g,b;
    Int32   obtained;
    ULONG   memIndex;
    float fixedWeight;

    // Make the TotalWeight large enough for the quantize function
    // to split properly. *10 is just a guess, maybe more is needed,
    // this can be changed.
    if(TotalWeight <= 1000000)
        fixedWeight = TotalWeight*10;
    else
        fixedWeight = TotalWeight;


    ColorVectors = new quantVector[ColorCount];
   //   vecInfo = new double[numColors*3];
    for(index = 0; index < ColorCount; index++)
    {
        ColorVectors[index].numDim = 3;
//      ColorVectors[index].pElem = &vecInfo[index*3];
        ColorVectors[index].pElem = new double[3];
    }

    OutVectors = new quantVector[ColorCount];
  //    vecInfoOut = new double[numOuts*3];
    for(index = 0; index < ColorCount; index++)
    {
        OutVectors[index].numDim = 3;
//      OutVectors[index].pElem = &vecInfoOut[index*3];
        OutVectors[index].weight = 0;
        OutVectors[index].pElem = new double[3];

    }


    // Enter the fixed palette colors.
    index = 0;
    for(paletteIndex = 0;paletteIndex < 256;paletteIndex++)
    {
        if((FixedPalette[paletteIndex].R != -1) &&
           (FixedPalette[paletteIndex].G != -1) &&
           (FixedPalette[paletteIndex].B != -1))

        {
            ColorVectors[index].pElem[0] = FixedPalette[paletteIndex].R;
            ColorVectors[index].pElem[1] = FixedPalette[paletteIndex].G;
            ColorVectors[index].pElem[2] = FixedPalette[paletteIndex].B;
            ColorVectors[index].weight   = fixedWeight;

            index++;
            
        }
    }


    // Now go through the popularity scan.
    for( r = 0;r < 128; r++)
    {
        for(g = 0;g < 128;g++)
        {
            for(b = 0;b < 128;b++)
            {
                memIndex=(((ULONG)(g))<<14)+(((ULONG)(r))<<7)+
                    ((ULONG)(b));

                if(ColorData[memIndex]!= 0)
                {
                    ColorVectors[index].pElem[0] = r*2;
                    ColorVectors[index].pElem[1] = g*2;
                    ColorVectors[index].pElem[2] = b*2;
                    ColorVectors[index].weight = ColorData[memIndex];
                    index++;
                }
            }
        }
    }
    obtained = numOuts;
    quantizeVectors(ColorVectors, ColorCount, OutVectors, obtained);
    if(obtained != numOuts)
    {
        printf("WARNING - more colors requested than found in data set\n");
        AssertMessage(0,avar("WARNING - more colors requested than found in data set\n"));
    }
    ColorCount = obtained;
}


//---------------------------------------------------------------------------
//
//  Function Name: QuantizeRGBA
//
//  Arguments    : numOuts tells the quantizer how many vectors to output.
//
//
//
//
//  Comments     : makes the input vectors for RGBA and quantizes. CANNOT do
//                 incremental palette building on RGBA's - no fixed palette.
//
//
//---------------------------------------------------------------------------

void pca_quantize::QuantizeRGBA(int numOuts)
{
    COUNTER index;
    ULONG   r,g,b,a;
    Int32   obtained;
    ULONG   memIndex;

    // Make the TotalWeight large enough for the quantize function
    // to split properly. *10 is just a guess, maybe more is needed,
    // this can be changed.
    float fixedWeight;
    if(TotalWeight <= 1000000)
        fixedWeight = TotalWeight*10;
    else
        fixedWeight = TotalWeight;

    ColorVectors = new quantVector[ColorCount];

//  vecInfo = new double[numColors*3];
    for(index = 0; index < ColorCount; index++)
    {
        ColorVectors[index].numDim = 4;
//      ColorVectors[index].pElem = &vecInfo[index*3];
        ColorVectors[index].pElem = new double[4];
    }

    OutVectors = new quantVector[ColorCount];
  //    vecInfoOut = new double[numOuts*3];
    for(index = 0; index < ColorCount; index++)
    {
        OutVectors[index].numDim = 4;
//      OutVectors[index].pElem = &vecInfoOut[index*3];
        OutVectors[index].weight = 0;
        OutVectors[index].pElem = new double[4];

    }

    // Enter the fixed palette colors.
    index = 0;
    for(int paletteIndex = 0;paletteIndex < 256;paletteIndex++)
    {
        if((FixedPalette[paletteIndex].R != -1) &&
           (FixedPalette[paletteIndex].G != -1) &&
           (FixedPalette[paletteIndex].B != -1))

        {
            ColorVectors[index].pElem[0] = FixedPalette[paletteIndex].R;
            ColorVectors[index].pElem[1] = FixedPalette[paletteIndex].G;
            ColorVectors[index].pElem[2] = FixedPalette[paletteIndex].B;
            ColorVectors[index].weight   = fixedWeight;

            index++;
            
        }
    }

    for( r = 0;r < 32; r++)
    {
        for(g = 0;g < 32;g++)
        {
            for(b = 0;b < 32;b++)
            {
                for(a = 0;a < 32;a++)
                {
                    memIndex = (((ULONG)(a))<<15) + (((ULONG)(g))<<10)
                                +(((ULONG)(r))<<5)+((ULONG)(b));
                    if(ColorData[memIndex]!= 0)
                    {
                        ColorVectors[index].pElem[0] = r*8;
                        ColorVectors[index].pElem[1] = g*8;
                        ColorVectors[index].pElem[2] = b*8;
                        ColorVectors[index].pElem[3] = a*8;
                        ColorVectors[index].weight = ColorData[memIndex];
                        index++;
                    }
                }
            }
        }
    }
    obtained = numOuts;
    quantizeVectors(ColorVectors, ColorCount, OutVectors, obtained);
    if(obtained != numOuts)
    {
        printf("WARNING - more colors requested than found in data set\n");
        AssertMessage(0,avar("WARNING - more colors requested than found in data set\n"));
    }
    ColorCount = obtained;
}


//---------------------------------------------------------------------------
//
//  Function Name: AddPaletteColors
//
//  Arguments    : inPal is the palette from which to pick the fixed
//                 colors. ChooseFirst/Last and RenderFirst/Last are the
//                 palette range/render range respectively.
//
//
//  Comments     : Takes all entries from inPal that are IN render range NOT IN
//                 choose range, and puts them in the FixedPalette member.
//                 
//                 
//
//---------------------------------------------------------------------------

void pca_quantize::AddPaletteColors(PALETTEENTRY *inPal, COUNTER ChooseFirst, COUNTER ChooseLast
                                    , COUNTER RenderFirst, COUNTER RenderLast)
{
    COUNTER index;
    ULONG   memIndex;
    
    // The fixed color range goes from the first color to render to
    // the first choose color, and then from the last choose color to
    // the last render color. Render ranges will ALWAYS be specified
    // regardless of whether we are rendering or not.

    if(RenderFirst < ChooseFirst)
    {
        for(index = RenderFirst;index < ChooseFirst;index++)
        {
            if(IsInIgnoreList(&inPal[index]) == False)
            {
                FixedPalette[index].R = inPal[index].peRed;
                FixedPalette[index].G = inPal[index].peGreen;
                FixedPalette[index].B = inPal[index].peBlue;

                ColorCount++;
                FixedCounter++;
            }
        }
    }
    
    // These if's are to make sure we have a fixed range, it is possible
    // that the render and choose ranges will be identical, in which
    // case we are choosing the entire range to render into, and we
    // don't need to fix any colors.
    if(RenderLast > ChooseLast)
    {
        for(index = ChooseLast + 1;index <= RenderLast;index++)
        {
            if(IsInIgnoreList(&inPal[index]) == False)
            {
                FixedPalette[index].R = inPal[index].peRed;
                FixedPalette[index].G = inPal[index].peGreen;
                FixedPalette[index].B = inPal[index].peBlue;

                ColorCount++;
                FixedCounter++;
            }
        }
    }
}


//---------------------------------------------------------------------------
//
//  Function Name: HazeShadeScan
//                 
//  Arguments    : shadeColor is a ptr to the, uh, shade color. maxShade is a 0->1
//                 percentage that tells how far to shade, shadeLevels tells how
//                 many steps to consider. Same for hazes.
//                 
//                 
//                 
//  Comments     : This reads in all colors from the pop buffer, and their weights,
//                 and then takes each one and sends it into the HazeShadeScanColor,
//                 which actually registeres the haze and shade colors. Actually,
//                 this function also determines whether you need shading or
//                 shading AND hazing, returns immediately if nothing.
//---------------------------------------------------------------------------

void pca_quantize::HazeShadeScan(PALETTEENTRY *shadeColor, float maxShade,
                                 PALETTEENTRY *hazeColor,  float maxHaze, 
                                 COUNTER shadeLevels, COUNTER hazeLevels)
{
    COUNTER         index;
    FLOATCOLOR      color;
    ULONG           r,g,b;
    ULONG           memIndex;
    COUNTER         shadedColors;
    ULONG           weight;
    COUNTER         paletteCount = 0;


    shadedColors = ColorCount - FixedCounter;
    PALETTEENTRY *ColorsToDo = new PALETTEENTRY[shadedColors];
    ULONG *ColorWeights = new ULONG[shadedColors];

    if((!shadeColor) && (!hazeColor))
    {
        return;
    }


    // Whip through once to get the colors to shade and haze.
    for( r = 0;r < 128; r++)
    {
        for(g = 0;g < 128;g++)
        {
            for(b = 0;b < 128;b++)
            {
                memIndex=(((ULONG)(g))<<14)+(((ULONG)(r))<<7)+
                    ((ULONG)(b));
                if(ColorData[memIndex] != 0)
                {
                    ColorsToDo[paletteCount].peRed = (UBYTE) r;
                    ColorsToDo[paletteCount].peGreen = (UBYTE) g;
                    ColorsToDo[paletteCount].peBlue = (UBYTE) b;
                    ColorWeights[paletteCount] = ColorData[memIndex];
                    paletteCount++;
                }
            }
        }
    }

    if((shadeColor) && (hazeColor))
        printf("Shade\\Hazing : ");
    else
    if(shadeColor)
        printf("Shading : ");
    else
    if(hazeColor)
        printf("Hazing : ");

    for(index = 0;index < shadedColors;index++)
    {
        if((index % 100) == 0) printf(".");
        color.R = 2*ColorsToDo[index].peRed;
        color.G = 2*ColorsToDo[index].peGreen;
        color.B = 2*ColorsToDo[index].peBlue;
        
        weight = ColorWeights[index];
        if((shadeColor) && (hazeColor))
        {
            HazeShadeScanColor(&color, shadeColor, maxShade, hazeColor, maxHaze, 
                                shadeLevels, hazeLevels,weight);
        }
        else
        if(shadeColor)
        {
            ShadeScanColor(&color, shadeColor, maxShade, shadeLevels, weight);
        }
        else
        if(hazeColor)
        {
            ShadeScanColor(&color, hazeColor, maxHaze, hazeLevels, weight);
        }
    }

    delete [] ColorsToDo;
    delete [] ColorWeights;
    printf("\n");
}



//---------------------------------------------------------------------------
//
//  Function Name: HazeShadeScanColor
//                 
//  Arguments    : takes a color ptr to shade and haze, the shade and 
//                 haze colors, the shade and haze levels and the percentage
//                 of each to which to shade and haze. This functions is used
//                 internally, called on each pixel of a HazeShadeScan.
//                 
//                 
//  Comments     : Just adds! The weight of the shade and haze fall off linearly.
//                 This is to keep things from bunching up around the shade
//                 and haze colors.
//                 
//
//---------------------------------------------------------------------------
void pca_quantize::HazeShadeScanColor(FLOATCOLOR *colortodo, PALETTEENTRY *shadeColor, float maxShade,
                  PALETTEENTRY *hazeColor, float maxHaze, COUNTER shadeLevels,
                  COUNTER hazeLevels, ULONG weight)
{

    FLOATCOLOR ShadeStep;
    FLOATCOLOR HazeStep;
    FLOATCOLOR HazeStepStep;

    FLOATCOLOR HazeAddition;
    FLOATCOLOR ShadeAddition;

    COUNTER    shadeIndex;
    COUNTER    hazeIndex;
    
    FLOATCOLOR shadedColor;
    FLOATCOLOR hazedColor;

    PALETTEENTRY byteColor;

    float       shadeWeight;
    float       hazeWeight;


    shadedColor.R = colortodo->R;
    shadedColor.G = colortodo->G;
    shadedColor.B = colortodo->B;

    ShadeStep.R = (float)(shadeColor->peRed) - (float)(shadedColor.R);
    ShadeStep.R *= maxShade;
    ShadeStep.R /= shadeLevels;

    ShadeStep.G = (float)(shadeColor->peGreen) - (float)(shadedColor.G);
    ShadeStep.G *= maxShade;
    ShadeStep.G /= shadeLevels;
    
    ShadeStep.B = (float)(shadeColor->peBlue) - (float)(shadedColor.B);
    ShadeStep.B *= maxShade;
    ShadeStep.B /= shadeLevels;

    HazeStep.R = (float)(hazeColor->peRed) - (float)(shadedColor.R);
    HazeStep.R *= maxHaze;
    HazeStep.R /= hazeLevels;

    HazeStep.G = (float)(hazeColor->peGreen) - (float)(shadedColor.G);
    HazeStep.G *= maxHaze;
    HazeStep.G /= hazeLevels;
    
    HazeStep.B = (float)(hazeColor->peBlue) - (float)(shadedColor.B);
    HazeStep.B *= maxHaze;
    HazeStep.B /= hazeLevels;

    HazeStepStep.R = -1*(float)(shadeColor->peRed) - (float)(shadedColor.R);
    HazeStepStep.R *= maxShade*maxHaze;
    HazeStepStep.R /= shadeLevels*hazeLevels;

    HazeStepStep.G = -1*(float)(shadeColor->peGreen) - (float)(shadedColor.G);
    HazeStepStep.G *= maxShade*maxHaze;
    HazeStepStep.G /= shadeLevels*hazeLevels;
    
    HazeStepStep.B = -1*(float)(shadeColor->peBlue) - (float)(shadedColor.B);
    HazeStepStep.B *= maxShade*maxHaze;
    HazeStepStep.B /= shadeLevels*hazeLevels;
    

    for(shadeIndex = 1;shadeIndex <= shadeLevels;shadeIndex++)
    {
        shadedColor.R += ShadeStep.R;
        shadedColor.G += ShadeStep.G;
        shadedColor.B += ShadeStep.B;
        
        byteColor.peRed  = (UBYTE)(shadedColor.R);
        byteColor.peGreen= (UBYTE)(shadedColor.G);
        byteColor.peBlue = (UBYTE)(shadedColor.B);

        shadeWeight = (1.25 - (float)(shadeIndex)/(float)(shadeLevels));
        shadeWeight *= weight;

        RegisterColor(&byteColor, shadeWeight);
        hazedColor.R = shadedColor.R;
        hazedColor.G = shadedColor.G;
        hazedColor.B = shadedColor.B;

        HazeStep.R -= HazeStepStep.R;
        HazeStep.G -= HazeStepStep.G;
        HazeStep.B -= HazeStepStep.B;
        

        for(hazeIndex = 1;hazeIndex <= hazeLevels;hazeIndex++)
        {
            hazedColor.R += HazeStep.R; 
            hazedColor.G += HazeStep.G; 
            hazedColor.B += HazeStep.B; 
            
            byteColor.peRed  = (UBYTE)(hazedColor.R);
            byteColor.peGreen= (UBYTE)(hazedColor.G);
            byteColor.peBlue = (UBYTE)(hazedColor.B);
            hazeWeight = (1.25 - (float)(hazeIndex)/(float)(hazeLevels));
            hazeWeight *= shadeWeight;

            RegisterColor(&byteColor, hazeWeight);
        }
    }
    // End of function.
}



//---------------------------------------------------------------------------
//
//  Function Name: ShadeScanColor
//                 
//  Arguments    : takes a color ptr to shade, the shade color, the shade haze level
//                 and the percentage of that to which to shade.
//                 
//  Comments     : Just adds!
//                 
//                 
//                 
//
//---------------------------------------------------------------------------

void pca_quantize::ShadeScanColor(FLOATCOLOR *colortodo, PALETTEENTRY *shadeColor, float maxShade,
                  COUNTER shadeLevels, ULONG weight)
{

    FLOATCOLOR   ShadeStep;
    COUNTER      shadeIndex;
    FLOATCOLOR   color;
    PALETTEENTRY byteColor;

    color.R = colortodo->R;
    color.G = colortodo->G;
    color.B = colortodo->B;

    ShadeStep.R = (float)(shadeColor->peRed) - (float)(color.R);
    ShadeStep.R *= maxShade;
    ShadeStep.R /= shadeLevels;

    ShadeStep.G = (float)(shadeColor->peGreen) - (float)(color.G);
    ShadeStep.G *= maxShade;
    ShadeStep.G /= shadeLevels;
    
    ShadeStep.B = (float)(shadeColor->peBlue) - (float)(color.B);
    ShadeStep.B *= maxShade;
    ShadeStep.B /= shadeLevels;

    for(shadeIndex = 1;shadeIndex <= shadeLevels;shadeIndex++)
    {
        color.R += ShadeStep.R;
        color.G += ShadeStep.G;
        color.B += ShadeStep.B;
        
        byteColor.peRed  = (UBYTE)(color.R);
        byteColor.peGreen= (UBYTE)(color.G);
        byteColor.peBlue = (UBYTE)(color.B);

        RegisterColor(&byteColor, (float)(weight));
    }
}


//---------------------------------------------------------------------------
//
//  Function Name: RegisterColor
//                 
//  Arguments    : *color is the color to put into the ColorData buffer. 
//                 
//                 
//                 
//                 
//                 
//  Comments     : this function works ONLY for RGB, it updates the ColorCount, checks
//                 for constant weighting, and updates the buffer.
//                 
//                 
//---------------------------------------------------------------------------


void pca_quantize::RegisterColor(PALETTEENTRY *color, float imageWeight)
{
    ULONG           memIndex;


    // It's conceivable via the haze/shade interpolation that this might be 0,
    // in which case we want to exit immediately!
    if((ULONG)(imageWeight) == 0)
        return;

    if(IsInIgnoreList(color) == False)
    {
        memIndex= (((ULONG)(color->peGreen)>>1)<<14)+
                  (((ULONG)(color->peRed)>>1)<<7)+((ULONG)(color->peBlue)>>1);
        if(ColorData[memIndex] == 0)
        {
            ColorCount++;
        }

        if(imageWeight == CONSTWEIGHT)
        {
                ColorData[memIndex] = 1;
                TotalWeight++;
        }
        else
        {
            ColorData[memIndex]+= (ULONG) imageWeight;
            TotalWeight += (ULONG) imageWeight;
        }
    }
    // end function
}


//---------------------------------------------------------------------------
//
//  Function Name: RegisterColor
//                 
//  Arguments    : *color is the color to put into the ColorData buffer. 
//                 
//                 
//                 
//                 
//                 
//  Comments     : this function works ONLY for RGB, it updates the ColorCount, checks
//                 for constant weighting, and updates the buffer.
//                 
//                 
//---------------------------------------------------------------------------


void pca_quantize::RegisterAlphaColor(PALETTEENTRY *color, float imageWeight)
{
    ULONG           memIndex;

    // It's conceivable via the haze/shade interpolation that this might be 0,
    // in which case we want to exit immediately!
    if((ULONG)(imageWeight) == 0)
        return;

    if(IsInIgnoreList(color) == False)
    {
        memIndex = (((ULONG)(color->peFlags)>>3)<<15) + (((ULONG)(color->peGreen)>>3)<<10)
                   + (((ULONG)(color->peRed)>>3)<<5) + ((ULONG)(color->peBlue)>>3);
        if(ColorData[memIndex] == 0)
        {
            ColorCount++;
        }
        
        if(imageWeight == 0)
        {
            if(ColorData[memIndex] == 0)
               TotalWeight++;

            ColorData[memIndex] = 1;
        }
        else
        {
            ColorData[memIndex] += (ULONG)imageWeight;
            TotalWeight         += (ULONG)imageWeight;
        }
    }
    // end function
}


//---------------------------------------------------------------------------
//
//  Function Name: BlendTransluscent
//                 
//  Arguments    : RGBPal is a pointer to a 256 entry RGB palette,
//                 RGBAPal is a pointer to a 256 entry RGBA palette.
//                 BlendRangeFirst is the first entry in the RGB pal to blend 
//                 with each of the transluscent colors - and 
//                 we'll blend to BlendRangeLast.
//                 
//                 
//  Comments     : Registers each color with the popularity scanner, as if 
//                 it were a bitmap.
//                 
//                 
//---------------------------------------------------------------------------


void pca_quantize::BlendTransluscent(PALETTEENTRY *RGBPal, PALETTEENTRY *RGBAPal, 
                                    COUNTER blendRangeFirst, COUNTER blendRangeLast)
{
    float correction;
    float alpha;
    PALETTEENTRY color;
    COUNTER RGBindex;
    COUNTER RGBAindex;
    ULONG memIndex;

    // 257 is the default blend value indicating that blending is OFF.

    if((blendRangeFirst == BLENDOFF) && (blendRangeLast == BLENDOFF))
    {
        AssertFatal(0, avar("Error - a blend range is needed for alpha blending\n"));
    }

    // If we have a valid range, go ahead and blend the colors and register them in the pop
    // scan.

    for(RGBAindex = 0;RGBAindex < 256;RGBAindex++)
    {
        for(RGBindex = blendRangeFirst;RGBindex <= blendRangeLast;RGBindex++)
        {

            // I am assuming here that black is not a color we want to blend with
            
            if((RGBAPal[RGBAindex].peRed != 0) && (RGBAPal[RGBAindex].peGreen != 0) &&
               (RGBAPal[RGBAindex].peBlue != 0)&& (RGBAPal[RGBAindex].peFlags != 0))
            {
                    correction = (float)((256.0f - (float)RGBAPal[RGBAindex].peFlags)/256.0f);
                    alpha = (float)(RGBAPal[RGBAindex].peFlags)/256.0f;
                
                    color.peRed   = RGBPal[RGBindex].peRed*correction  + RGBAPal[RGBAindex].peRed*alpha;
                    color.peGreen = RGBPal[RGBindex].peGreen*correction+ RGBAPal[RGBAindex].peGreen*alpha;
                    color.peBlue  = RGBPal[RGBindex].peBlue*correction + RGBAPal[RGBAindex].peBlue*alpha;
                    memIndex=(((ULONG)(color.peGreen)>>1)<<14)+
                             (((ULONG)(color.peRed)>>1)<<7)+((ULONG)(color.peBlue)>>1);
                    ColorData[memIndex] += 1;
                    ColorCount++;
                    TotalWeight++;
            }   
        }
    }
}