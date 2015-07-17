//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "gSGLSfc.h"
#include "fn_table.h"

namespace PowerSGL {

Surface* 
Surface::create(HWND        /*io_clientWnd*/,
                const Int32 /*in_width*/,
                const Int32 /*in_height*/)
{
   // Must return a new PowerSGL::Surface* of resolution closest to in_w/h.
   //  Should also activate the device.
   //
   return NULL;
}


Surface::Surface()
{
   functionTable = &powersgl_table;
}


Surface::~Surface()
{

}


void 
Surface::flip()
{
   // Swap buffers
}

void 
Surface::_lock(const GFXLockMode in_lockMode)
{
   AssertFatal(in_lockMode == GFX_NORMALLOCK,
               "Double lock not supported in SGL Mode");

   // Do any prep for drawing.  It is not necessary to lock a surface pointer
   //  here.
   //
}

void 
Surface::_unlock()
{
   // Undo any prep for drawing.
   //
}

Bool 
Surface::_setPalette(GFXPalette* /*lpPalette*/,
                     Int32       /*in_start*/,
                     Int32       /*in_count*/,
                     Bool        /*in_rsvPal*/)
{
   return false;
}

void 
Surface::_setGamma()
{
   // Desired gamma value is stored in the gamma member of GFXSurface
   //
}

void 
Surface::drawSurface(GFXSurface*    /*lpSourceSurface*/,
                     const RectI*   /*in_subRegion*/,
                     const Point2I* /*in_at*/)
{
   // Would be nice to have this, but it's not particularily necessary...
   //
}

void 
Surface::processMessage(int         argc,
                        const char* argv[])
{
   if (argc == 0)
      return;
   
   const char* pMsg = argv[0];
   if (!strcmp(pMsg, "flushCache")) {
      flushTextureCache();
   } else if (!strcmp(pMsg, "bilinear")) {
      // Toggle bilinear filtering?  (Nice, but not necessary)
   } else {
      // Any useful debugging functions can go here...
   }
}

}; // namespace OpenGl

