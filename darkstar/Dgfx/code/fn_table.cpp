#include "fn_table.h"
#include "d_funcs.h"

// ========================================================
// Function tables

// --------------------------------------------------------

extern void GFXSetZMode(GFXSurface *, bool wBuffer);

FunctionTable rclip_table =
{
	GFXClearScreen,         // Clear

	GFXDrawPoint2d,         // Draw Point 2d
	GFXDrawLine2d,          // Draw Line 2d
	GFXDrawRect2d,          // Draw Rectangle 2d
	GFXDrawRect2d_f,        // Draw Rectangle 2d, Filled

	GFXDrawBitmap2d_f,      // Draw Bitmap 2d
	GFXDrawBitmap2d_rf,     // Draw Bitmap 2d, Region
	GFXDrawBitmap2d_sf,     // Draw Bitmap 2d, Stretch
	GFXDrawBitmap2d_af,     // Draw Bitmap 2d, Stretch, Rotate

	GFXDrawText_p,          // Draw Text String at Given Point
	GFXDrawText_r,          // Draw Text String at Given Rectangle

	GFXDraw3DBegin,         // Draw 3D Begin
	GFXDraw3DEnd,            // Draw 3D End

	GFXAddVertexV,
	GFXAddVertexVT,
	GFXAddVertexVC,
	GFXAddVertexVTC,
	GFXEmitPoly,
	GFXSetShadeSource,
	GFXSetHazeSource,
	GFXSetAlphaSource,
	GFXSetFillColorCF,
	GFXSetFillColorI,
	GFXSetHazeColorCF,
	GFXSetHazeColorI,
	GFXSetConstantShadeCF,
	GFXSetConstantShadeF,
	GFXSetConstantHaze,
	GFXSetConstantAlpha,
	GFXSetTransparency,
	GFXSetTextureMap,
	GFXSetFillMode,
	GFXSetTexturePerspective,

	GFXRegisterTexture,
	GFXRegisterTextureCB,
	GFXSetTextureHandle,
	GFXAllocateLightMap,
	GFXHandleGetLightMap,
	GFXHandleSetLightMap,
	GFXHandleSetTextureMap,
	GFXHandleSetMipLevel,
	GFXHandleSetTextureSize,
	GFXFlushTextureCache,
	GFXSetZTest,
	GFXClearZBuffer,
	GFXSetTextureWrap,
   
   GFXFlushTexture,
   GFXSetZMode,

   GFXSetClipPlanes,

   GFXGetHazeSource,
   GFXGetConstantHaze
};
