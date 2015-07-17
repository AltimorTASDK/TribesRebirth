    //---------------------------------------------------------------------------
//
//
//  Filename   : pcaquan.h
//
//
//  Description: quantizer class definition - basically the quantizer's job
//               is to scan the bitmaps (and palettes) and create the optimal
//               palette - the underlying quantizer itself is Dave Moore's PCA
//               library, the quantizer class wraps around this and makes inputs
//               for it.
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

#ifndef _PCAQUAN_H
#define _PCAQUAN_H

const float CONSTWEIGHT = -1;

class pca_quantize
{
    private:
        ULONG *ColorData;
        ULONG DataSize;
        ULONG ColorCount;
        float TotalWeight;
        
        quantVector *ColorVectors;
        quantVector *OutVectors;
        double *vecInfo;        
        double *vecInfoOut;
        PALETTEENTRY *ignoreList;
        COUNTER ignoreListSize;
        PALETTEENTRY *ZeroColor;
        FLOATCOLOR FixedPalette[256];

        TYPE Type;
        COUNTER FixedCounter;
        void QuantizeRGB(int);
        void QuantizeRGBA(int);
        FLAG IsInIgnoreList(PALETTEENTRY *);
        void RegisterColor(PALETTEENTRY *, float);
        void RegisterAlphaColor(PALETTEENTRY *, float);
        void HazeShadeScanColor(FLOATCOLOR *, PALETTEENTRY *, float,
                  PALETTEENTRY *, float, COUNTER, COUNTER, ULONG);
        void ShadeScanColor(FLOATCOLOR *, PALETTEENTRY *, float, COUNTER, ULONG);
        
               
    public:
        pca_quantize(TYPE);
        ~pca_quantize(void);
        void SetFlags(quantizeFlags *);
        void ScanImage(PALETTEENTRY *, ULONG, ULONG);
        void GetPalette(PALETTEENTRY *,int,int);
        void AddPaletteColors(PALETTEENTRY*, COUNTER, COUNTER, COUNTER, COUNTER);
        void HazeShadeScan(PALETTEENTRY *, float,
                                PALETTEENTRY *, float, COUNTER, COUNTER);
        void BlendTransluscent(PALETTEENTRY *, PALETTEENTRY *, COUNTER, COUNTER);

};                

#endif // _PCAQUAN_H