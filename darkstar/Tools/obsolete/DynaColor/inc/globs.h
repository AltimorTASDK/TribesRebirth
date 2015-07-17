#ifndef __GLOBS_H
#define __GLOBS_H
#include "dy_types.h"
#include "blend.h"


typedef struct
 {
  u8 b,g,r,pad;
 }
u8colorVec; 


//globals
extern u32 SortPalette;
extern u32 ApplyShade;
extern u32 ApplyHaze;
extern u32 IgnoreColorOn;
extern u8colorVec IgnoreColor;
extern u32 zeroColorOn;
extern u8colorVec zeroColor;
extern blend_color Shade; 
extern blend_color Haze;  
extern f32 GammaVal; /* normal range is 2.2-2.8 */
extern char *PalOutName;
extern u8colorVec Pal[256];	
extern u8 PalSetFlags[256];
extern u8 Color0,Color1;
extern u32 NewRange;
extern u32 MostRecentDist;
extern s32 Gamma[256];
extern u8 *Pointer;
extern u8 *Data;
extern u8 *Mask;
extern u32 ColorRange[17];
extern u16 Memory[256*256*32];
extern f32 Memory2[64*64*64];
extern u32 Rendering;
extern u32 Height;
extern u32 Width;
extern u32 PaddedWidth; // RST
extern u32 prevRed,prevBlue,prevGreen;
extern s32 prevErrorRed,prevErrorGreen,prevErrorBlue;
extern u32 ChooseTotalColors;
extern u32 RenderTotalColors;
extern u32 TotalColors;
extern u32 PercentageForRegions;
extern u32 PercentageForPopularity;
extern u32 GammaOn;
extern u32 BlackThreshold;
extern u32 MinDither;
extern u32 MaxDither;
extern u32 Method;
extern u32 ChooseBaseOffset;
extern u32 RenderBaseOffset;
extern u32 BaseOffset;
extern u32 Seed;
extern u32 ExactMatchPassThrough;
extern u32 LoadAPalette;
extern u32 ChooseAPalette;
extern u32 RenderABitmap;
extern u32 MinAviFrame;
extern u32 MaxAviFrame;
extern s32 RotateFrames;
extern s32 TrimFrames;
extern u32 CutFlag;
extern u32 CutThreshold;
extern u32 CompressFlag;

extern u8* RenderData;
extern u8 RenderPal [1024];


#endif //__GLOBS_H