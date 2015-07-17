#include "fn_all.h"

// ========================================================
// Function tables

FunctionTable software_eclip_table =
{
   GFXClearScreen,         // Clear

   GFXDrawPoint2d_ec,      // Draw Point 2d
   GFXDrawLine2d_ec,       // Draw Line 2d
   GFXDrawCircle2d_ec,     // Draw Circle Outline 2d
   GFXDrawCircle2d_f_ec,   // Draw Circle Filled 2d
   GFXDrawRect2d_ec,       // Draw Rectangle 2d
   GFXDrawRect2d_f,     // Draw Rectangle 2d, Filled

   GFXDrawBitmap2d_f_ec,   // Draw Bitmap 2d
   GFXDrawBitmap2d_rf,     // Draw Bitmap 2d, Region
   GFXDrawBitmap2d_sf_ec,  // Draw Bitmap 2d, Stretch
//   GFXDrawBitmap2d_af,     // Draw Bitmap 2d, Stretch, Rotate
   NULL,                   // Draw Bitmap 2d, Stretch, Rotate

   GFXDrawPoly,       // Draw Poly

   GFXDrawText_p,          // Draw Text String at Given Point
   GFXDrawText_r,          // Draw Text String at Given Rectangle

   NULL,                   // Draw user defined 1
   NULL,                   // Draw user defined 2
   NULL,                   // Draw user defined 3
   NULL,                   // Draw user defined 4
   NULL,                   // Draw user defined 5
   NULL,                   // Draw user defined 6
   GFXDraw3DBegin,         // Draw 3D Begin
   GFXDraw3DEnd            // Draw 3D End
};

// --------------------------------------------------------

FunctionTable software_rclip_table =
{
   GFXClearScreen,         // Clear

   GFXDrawPoint2d,         // Draw Point 2d
   GFXDrawLine2d,          // Draw Line 2d
   GFXDrawCircle2d,        // Draw Circle Outline 2d
   GFXDrawCircle2d_f,      // Draw Circle Filled 2d

   GFXDrawRect2d,          // Draw Rectangle 2d
   GFXDrawRect2d_f,        // Draw Rectangle 2d, Filled

   GFXDrawBitmap2d_f,      // Draw Bitmap 2d
   GFXDrawBitmap2d_rf,     // Draw Bitmap 2d, Region
   GFXDrawBitmap2d_sf,     // Draw Bitmap 2d, Stretch
   GFXDrawBitmap2d_af,     // Draw Bitmap 2d, Stretch, Rotate

   GFXDrawPoly,          // Draw Poly

   GFXDrawText_p,          // Draw Text String at Given Point
   GFXDrawText_r,          // Draw Text String at Given Rectangle

   NULL,                   // Draw user defined 1
   NULL,                   // Draw user defined 2
   NULL,                   // Draw user defined 3
   NULL,                   // Draw user defined 4
   NULL,                   // Draw user defined 5
   NULL,                   // Draw user defined 6
   GFXDraw3DBegin,         // Draw 3D Begin
   GFXDraw3DEnd            // Draw 3D End
};


// --------------------------------------------------------

FunctionTable ddraw_eclip_table =
{
   GFXClearScreen,         // Clear

   GFXDrawPoint2d_ec,      // Draw Point 2d
   GFXDrawLine2d_ec,       // Draw Line 2d
   GFXDrawCircle2d_ec,     // Draw Circle Outline 2d
   GFXDrawCircle2d_f_ec,   // Draw Circle Filled 2d
   GFXDrawRect2d_ec,       // Draw Rectangle 2d
   GFXDrawRect2d_f,     // Draw Rectangle 2d, Filled

   GFXDrawBitmap2d_f_ec,   // Draw Bitmap 2d
   GFXDrawBitmap2d_rf,     // Draw Bitmap 2d, Region
   GFXDrawBitmap2d_sf_ec,  // Draw Bitmap 2d, Stretch
//   GFXDrawBitmap2d_af,     // Draw Bitmap 2d, Stretch, Rotate
   NULL,                   // Draw Bitmap 2d, Stretch, Rotate

   GFXDrawPoly,       // Draw Poly

   GFXDrawText_p,          // Draw Text String at Given Point
   GFXDrawText_r,          // Draw Text String at Given Rectangle

   NULL,                   // Draw user defined 1
   NULL,                   // Draw user defined 2
   NULL,                   // Draw user defined 3
   NULL,                   // Draw user defined 4
   NULL,                   // Draw user defined 5
   NULL,                   // Draw user defined 6
   GFXDraw3DBegin,         // Draw 3D Begin
   GFXDraw3DEnd            // Draw 3D End
};

// --------------------------------------------------------

FunctionTable ddraw_rclip_table =
{
   GFXClearScreen,         // Clear

   GFXDrawPoint2d,         // Draw Point 2d
   GFXDrawLine2d,          // Draw Line 2d
   GFXDrawCircle2d,        // Draw Circle Outline 2d
   GFXDrawCircle2d_f,      // Draw Circle Filled 2d

   GFXDrawRect2d,          // Draw Rectangle 2d
   GFXDrawRect2d_f,        // Draw Rectangle 2d, Filled

   GFXDrawBitmap2d_f,      // Draw Bitmap 2d
   GFXDrawBitmap2d_rf,     // Draw Bitmap 2d, Region
   GFXDrawBitmap2d_sf,     // Draw Bitmap 2d, Stretch
   GFXDrawBitmap2d_af,     // Draw Bitmap 2d, Stretch, Rotate

   GFXDrawPoly,          // Draw Poly

   GFXDrawText_p,          // Draw Text String at Given Point
   GFXDrawText_r,          // Draw Text String at Given Rectangle

   NULL,                   // Draw user defined 1
   NULL,                   // Draw user defined 2
   NULL,                   // Draw user defined 3
   NULL,                   // Draw user defined 4
   NULL,                   // Draw user defined 5
   NULL,                   // Draw user defined 6
   GFXDraw3DBegin,         // Draw 3D Begin
   GFXDraw3DEnd            // Draw 3D End
};
