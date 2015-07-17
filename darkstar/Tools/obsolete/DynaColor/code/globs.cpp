#include <stdio.h>
#include "dy_types.h"
#include "globs.h"
#include "blend.h"

//globals
u32 SortPalette;
u32 ApplyShade;
u32 ApplyHaze;
u32 IgnoreColorOn;
u8colorVec IgnoreColor;
u32 zeroColorOn;
u8colorVec zeroColor;
blend_color Shade;
blend_color Haze;
f32 GammaVal; /* normal range is 2.2-2.8 */
char *PalOutName;
u8colorVec Pal[256];	
u8 PalSetFlags[256];
u8 Color0,Color1;
u32 NewRange;
u32 MostRecentDist;
s32 Gamma[256];
u8 *Pointer;
u8 *Data;
u8 *Mask;
u32 ColorRange[17];
u16 Memory[256*256*32];
f32 Memory2[64*64*64];
u32 Rendering;
u32 Height;
u32 Width;
u32 PaddedWidth; // RST
u32 prevRed,prevBlue,prevGreen;
s32 prevErrorRed,prevErrorGreen,prevErrorBlue;

u32 ChooseTotalColors;
u32 RenderTotalColors;
u32 TotalColors;
u32 PercentageForRegions;
u32 PercentageForPopularity;
u32 GammaOn;
u32 BlackThreshold;
u32 MinDither;
u32 MaxDither;
u32 Method;
u32 ChooseBaseOffset;
u32 RenderBaseOffset;
u32 BaseOffset;
u32 Seed;
u32 ExactMatchPassThrough;
u32 LoadAPalette;
u32 ChooseAPalette;
u32 RenderABitmap;
u32 MinAviFrame;
u32 MaxAviFrame;
s32 RotateFrames;
s32 TrimFrames;
u32 CutFlag;
u32 CutThreshold;
u32 CompressFlag;



u8* RenderData;
u8 RenderPal [1024];
