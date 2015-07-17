//---------------------------------------------------------------------------
//
//
//  Filename   : render.h
//
//
//  Description: Class definition for the render class.
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

#ifndef _RENDER_H
#define _RENDER_H

const TYPE REMAP = 0;
const TYPE DITHER = 1;

const signed short NULINDEX = -1;

class render
{
    private:
        UBYTE *pData8;
        PALETTEENTRY *pData24;
        COUNTER Height;
        COUNTER Width;
        COUNTER ImageSize;
        TYPE Type;
        COUNTER RangeFirst;
        COUNTER RangeLast;        
        PALETTEENTRY RenderPal[256];
        COLOR LUVRenderPal[256];
        signed short *LookupTable;
        FLAG DitherFlag;
        PALETTEENTRY *ZeroColor;
        float tolerance;

        void MakeLookup(void);
        UBYTE FindClosestMatch(PALETTEENTRY *); 
        double PalColorDifference(PALETTEENTRY *, COUNTER);
        double ColorDifference(PALETTEENTRY *, PALETTEENTRY *);
        void Dither(void);
        FLAG IsZeroColor(PALETTEENTRY *);
        void ErrorDistrib(FLOATCOLOR *, COUNTER, COUNTER);
        void UpdatePixel(FLOATCOLOR *, COUNTER, COUNTER, float);
        FLAG SolidColor(PALETTEENTRY *, COUNTER, COUNTER);
        

    public:
        render(PALETTEENTRY *, TYPE);
        ~render(void);
        RemapImage(void);
        void RenderImage(void);
        void SetData(UBYTE *, PALETTEENTRY *, renderFlags *);
        
};

















#endif // _RENDER_H